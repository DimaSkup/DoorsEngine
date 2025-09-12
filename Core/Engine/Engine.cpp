// =================================================================================
// Filename:  Engine.cpp
// Created:   05.10.22
// =================================================================================
#include <CoreCommon/pch.h>
#include "Engine.h"

#include "ProjectSaver.h"
#include "DumpGenerator.h"
#include "ImGuizmo.h"

#include "../Texture/TextureMgr.h"
#include "../Model/ModelMgr.h"

#pragma warning (disable : 4996)


namespace Core
{

//---------------------------------------------------------
// Default constructor / destructor
//---------------------------------------------------------
Engine::Engine()
{
    LogDbg(LOG, "init");
    timer_.Reset();       // reset the engine/game timer
}

Engine::~Engine()
{
    SetConsoleColor(YELLOW);
    LogMsg("\n");
    LogMsg("-------------------------------------------------");
    LogMsg("            START OF THE DESTROYMENT:            ");
    LogMsg("-------------------------------------------------");
    LogMsg("\n");
    SetConsoleColor(RESET);

    // unregister the window class, destroys the window,
    // reset the responsible members;
    if (hwnd_ != NULL)
    {
        ChangeDisplaySettings(NULL, 0);  // before destroying the window we need to set it to the windowed mode
        DestroyWindow(hwnd_);            // Remove the window
        hwnd_ = NULL;
        hInstance_ = NULL;

        LogDbg(LOG, "engine desctuctor");
    }

    imGuiLayer_.Shutdown();

    LogMsg("the engine is shut down successfully");
}


// =================================================================================
//                            public methods
// =================================================================================
bool Engine::Initialize(
    HINSTANCE hInstance,
    HWND mainWnd,
    const EngineConfigs& settings,
    const std::string& windowTitle,
    ECS::EntityMgr* pEnttMgr,
    UI::UserInterface* pUserInterface,
    Render::CRender* pRender)
{
    // initializes all the main parts of the engine

    // stuff for debug dumps generation (in case of crash)
    if (!IsDebuggerPresent())
        SetUnhandledExceptionFilter(UnhandleExceptionFilter);

    try
    {
        // check support for SSE2 (Pentium4, AMD K8, and above)
        CAssert::True(DirectX::XMVerifyCPUSupport(), "XNA math not supported");
        CAssert::True(pEnttMgr != nullptr,       "input ptr to the Entity Manager == nullptr");
        CAssert::True(pUserInterface != nullptr, "input ptr to the User Interface == nullptr");
        CAssert::True(pRender != nullptr,        "input ptr to the Render == nullptr");

        // WINDOW: store a handle to the application instance
        hInstance_      = hInstance;  
        hwnd_           = mainWnd;
        windowTitle_    = windowTitle;

        // init pointers
        pEnttMgr_       = pEnttMgr;
        pUserInterface_ = pUserInterface;
        pRender_        = pRender;


        // GRAPHICS SYSTEM: initialize the graphics system
        bool result = graphics_.Initialize(
            hwnd_,
            systemState_, 
            settings,
            pEnttMgr,
            pRender);
        CAssert::True(result, "can't initialize the graphics system");

        Render::D3DClass& d3d         = graphics_.GetD3DClass();
        ID3D11Device* pDevice         = d3d.GetDevice();
        ID3D11DeviceContext* pContext = d3d.GetDeviceContext();

        systemState_.wndWidth_        = d3d.GetWindowWidth();
        systemState_.wndHeight_       = d3d.GetWindowHeight();
        
#if 0
        // SOUND SYSTEM: initialize the sound obj
        result = sound_.Initialize(hwnd_);
        CAssert::True(result, "can't initialize the sound system");
#endif

        // TIMERS: (game timer, CPU)
        timer_.Tick();                 
        cpu_.Initialize();
        imGuiLayer_.Initialize(hwnd_, pDevice, pContext);

        if (!g_ModelMgr.Init())
        {
            LogErr(LOG, "can't init a model manager");
            exit(0);
        }

        LogMsg("is initialized!");
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////

void Engine::Update()
{
    auto updateStartTime = std::chrono::steady_clock::now();
    timer_.Tick();
    
#if 0
    // to update the system stats each of timers classes we needs to call its 
    // own Update function for each frame of execution the application goes through
    cpu_.Update();

    // update the percentage of total cpu use that is occuring each second
    systemState_.cpu = cpu_->GetCpuPercentage();
#endif

    // get the time which passed since the previous frame
    deltaTime_ = timer_.GetDeltaTime();


    systemState_.deltaTime = deltaTime_;
    systemState_.frameTime = deltaTime_ * 1000.0f;

    // compute fps and frame time (ms)
    CalculateFrameStats();

    // handle keyboard imput
    if (keyboard_.IsAnyPressed() || keyboard_.HasReleasedEvents())
    {
        if (systemState_.isEditorMode)
            HandleEditorEventKeyboard(pUserInterface_, pEnttMgr_);
    }

    // update the entities and related data
    pEnttMgr_->Update(timer_.GetGameTime(), deltaTime_);

    pUserInterface_->Update(graphics_.GetD3DClass().GetDeviceContext(), systemState_);
    keyboard_.Update();
    graphics_.Update(systemState_, deltaTime_, timer_.GetGameTime(), pEnttMgr_, pRender_);

    // if we want to switch btw game/editor mode
    if (switchEngineMode_)
    {
        switchEngineMode_ = false;

        if (IsGameMode())
            TurnOnEditorMode();
        else
            TurnOnGameMode();
    }

    // compute the duration of the engine's update process
    auto updateEndTime = std::chrono::steady_clock::now();
    std::chrono::duration<float, std::milli> updateDuration = updateEndTime - updateStartTime;
    systemState_.updateTime = updateDuration.count();
}

///////////////////////////////////////////////////////////

void Engine::CalculateFrameStats()
{
    // measure the number of frames being rendered per second (FPS);
    // this method would be called every frame in order to count the frame

    // code computes the avetage frames per second, and also the average time it takes
    // to render one frame. These stats are appended to the window caption bar

    static int frameCount = 0;
    static int timeElapsed = 0;
    
    frameCount++;

    // compute averages over one second period
    if ((timer_.GetGameTime() - timeElapsed) >= 1.0f)
    {
        // store the fps value for later using (for example: render this value as text onto the screen)
        systemState_.fps = frameCount;
   
        // reset for next average
        frameCount = 0;
        ++timeElapsed;

        // print FPS/frame_time as the window caption
#if 0
        PROCESS_MEMORY_COUNTERS pmc;
        DWORD processID;
        GetWindowThreadProcessId(hwnd_, &processID);

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
        CAssert::True(hProcess != NULL, "can't get a process handle of the window");
        
        GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc));

        std::wostringstream outs;
        outs.precision(6);
        outs << windowTitle_ << L" "
             << L"FPS: " << systemState_.fps << L" "
             << L"Frame Time: " << systemState_.frameTime << L" (ms); "
             << L"RAM Usage: " << pmc.WorkingSetSize / 1024 / 1024 << L" (mb)";
        SetWindowText(hwnd_, outs.str().c_str());		
#endif
    }
}

///////////////////////////////////////////////////////////

void Engine::RenderFrame()
{
    // this function executes rendering of each frame;
    try
    {
        using namespace DirectX;

        auto renderStartTime = std::chrono::steady_clock::now();
        Render::D3DClass& d3d = graphics_.GetD3DClass();
        ID3D11DeviceContext* pContext = d3d.GetDeviceContext();

       
        if (systemState_.isEditorMode)
        {
            // Clear all the buffers before frame rendering and render our 3D scene
            d3d.ResetBackBufferRenderTarget();
            d3d.ResetViewport();

            d3d.BeginScene();
            graphics_.Render3D(pEnttMgr_, pRender_);
            
            // begin rendering of the editor elements
            imGuiLayer_.Begin();
            RenderUI(pUserInterface_, pRender_);

            ImGui::End();
            imGuiLayer_.End();
        }

        // we aren't in the editor mode
        else  
        {
            // Clear all the buffers before frame rendering and render our 3D scene
            d3d.BeginScene();
            graphics_.Render3D(pEnttMgr_, pRender_);

            // render game UI
            RenderUI(pUserInterface_, pRender_);
        }

        // Show the rendered stuff on the screen
        d3d.EndScene();

        // before next frame
        graphics_.ClearRenderingDataBeforeFrame(pEnttMgr_, pRender_);

        // compute the duration of the engine's rendering process
        auto renderEndTime = std::chrono::steady_clock::now();
        std::chrono::duration<float, std::milli> renderDuration = renderEndTime - renderStartTime;
        systemState_.renderTime = renderDuration.count();
    }
    catch (EngineException & e)
    {
        LogErr(e, true);
        LogErr("can't render a frame");
        isExit_ = true;                   // exit after it (shutdown the engine)
    }
}

///////////////////////////////////////////////////////////

void Engine::RenderUI(UI::UserInterface* pUI, Render::CRender* pRender)
{
    Render::D3DClass& d3d = graphics_.GetD3DClass();

    // preparation before 2D rendering
    d3d.TurnZBufferOff();
    d3d.TurnOnBlending(Render::R_ALPHA_ENABLE);
    d3d.TurnOnRSfor2Drendering();

    if (systemState_.isEditorMode)
    {
        // all render the scene view space and gizmos (if any entt is selected)
        pUI->RenderSceneWnd(systemState_);

        // HACK: we set background color for ImGui elements (except of scene windows)
        //       each fucking time because if we doesn't it we will have 
        //       scene objects which are using blending to be mixed with 
        //       ImGui bg color; so we want to have proper scene colors;
        ImVec4* colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_WindowBg] = imGuiLayer_.GetBackgroundColor();

        
        pUI->RenderEditor(systemState_);

        // reset: ImGui window bg color to fully invisible since we
        //        want to see the scene through the window
        colors[ImGuiCol_WindowBg] = { 0,0,0,0 };

    }
    // we're in the game mode
    else
    {
        pUI->RenderGameUI(
            d3d.GetDeviceContext(),
            *pRender,
            systemState_);
    }

    // reset after 2D rendering
    d3d.TurnOffBlending();     
    d3d.TurnZBufferOn(); 
    d3d.TurnOffRSfor2Drendering();
}


// =================================================================================
// Window events handlers 
// =================================================================================
void Engine::EventActivate(const APP_STATE state)
{
    // define that the app is curretly running or paused

    if (state == APP_STATE::ACTIVATED)
    {
        isPaused_ = false;
        timer_.Start();
    }
    else if (state == APP_STATE::DEACTIVATED)
    {
        isPaused_ = true;
        timer_.Stop();
    }
}

///////////////////////////////////////////////////////////

void Engine::EventWindowMove(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //assert("TODO: implement it!" && 0);
}

///////////////////////////////////////////////////////////

void Engine::EventWindowResize(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SIZE newSize{ LOWORD(lParam), HIWORD(lParam) };

    // try to resize the window
    if (!graphics_.GetD3DClass().ResizeSwapChain(hwnd, newSize))
    {
        PostQuitMessage(0);
    }

    // update all the cameras according to new dimensions
    if (pEnttMgr_)
    {
        const float aspectRatio = (float)newSize.cx / (float)newSize.cy;
        cvector<EntityID> ids;
        pEnttMgr_->cameraSystem_.GetAllCamerasIds(ids);

        // update each camera
        for (const EntityID id : ids)
            pEnttMgr_->cameraSystem_.SetAspectRatio(id, aspectRatio);
    }
}

///////////////////////////////////////////////////////////

void Engine::EventWindowSizing(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    RECT wndRect;
    GetWindowRect(hwnd, &wndRect);   // get the window dimensions

    switch (wParam)
    {
        case WMSZ_LEFT:
        case WMSZ_RIGHT:
        case WMSZ_TOP:
        case WMSZ_BOTTOM:
        {
            // resize by X or by Y
            const int posX   = wndRect.left;
            const int posY   = wndRect.top;
            const int width  = wndRect.right - posX;
            const int height = wndRect.bottom - posY;

            // Set new dimensions
            SetWindowPos(hwnd, NULL, posX, posY,
                width, height,
                SWP_NOMOVE | SWP_NOZORDER);

            // try to resize the window
            if (!graphics_.GetD3DClass().ResizeSwapChain(hwnd, { width, height }))
                PostQuitMessage(0);

            systemState_.wndWidth_  = width;
            systemState_.wndHeight_ = height;

            break;
        }
    }
}

//---------------------------------------------------------
// Desc:   handle camera movement when we're in the editor mode
//---------------------------------------------------------
void HandleEditorCameraMovement(
    const eKeyCodes code,
    const float deltaTime,
    ECS::EntityMgr* pEnttMgr,
    const Keyboard& keyboard)
{
    ECS::CameraSystem& camSys = pEnttMgr->cameraSystem_;
    const EntityID     camID = pEnttMgr->nameSystem_.GetIdByName("editor_camera");
    constexpr int      editorCameraSpeed = 3;
    
    // if camera look isn't fixed at some particular point
    if (!camSys.IsFixedLook(camID))
    {
        // double the speed if we press shift
        const bool  isRunning = keyboard.IsPressed(KEY_SHIFT);
        const int   speedMul  = 1 + 10 * (isRunning);
        const float currSpeed = deltaTime * (speedMul * editorCameraSpeed);

        switch (code)
        {
            case KEY_SPACE:
            {
                camSys.MoveUp(camID, currSpeed);
                break;
            }
            case KEY_A:
            {
                camSys.Strafe(camID, -currSpeed);
                break;
            }
            case KEY_D:
            {
                camSys.Strafe(camID, +currSpeed);
                break;
            }
            case KEY_S:
            {
                camSys.Walk(camID, -currSpeed);
                break;
            }
            case KEY_W:
            {
                camSys.Walk(camID, +currSpeed);
                break;
            }
            case KEY_Z:
            {
                camSys.MoveUp(camID, -currSpeed);
                break;
            }
        } // switch
    }
    // handle moving around some fixed look_at point (if we set any fixed one)
    else
    {
        switch (code)
        {
            case KEY_A:
            {
                camSys.RotateYAroundFixedLook(camID, deltaTime);
                break;
            }
            case KEY_D:
            {
                camSys.RotateYAroundFixedLook(camID, -deltaTime);
                break;
            }
            case KEY_S:
            {
                camSys.Walk(camID, -deltaTime);
                break;
            }
            case KEY_W:
            {
                camSys.Walk(camID, deltaTime);
                break;
            }
        } // switch
    }
}

//---------------------------------------------------------
// Desc:   a handler for keyboard events when we're in the editor mode
//---------------------------------------------------------
void Engine::HandleEditorEventKeyboard(UI::UserInterface* pUI, ECS::EntityMgr* pEnttMgr)
{
    // go through each currently pressed key and handle related events
    for (const eKeyCodes code : keyboard_.GetPressedKeysList())
    {
        switch (code)
        {
            case KEY_SHIFT:
            {
                //player.SetIsRunning(true);
                pUI->UseSnapping(true);
                break;
            }
            // case (keys 0-3): switch the number of directional lights
            case KEY_0:
            case KEY_1:
            case KEY_2:
            case KEY_3:
            {
                ID3D11DeviceContext* pContext = graphics_.GetD3DClass().GetDeviceContext();
                int numDirLights = code - (int)(KEY_0);
                pRender_->SetDirLightsCount(pContext, numDirLights);

                break;
            }
            case KEY_4:   // turn off showing of bounding boxes around entts
            case KEY_5:   // show bounding box of the whole model
            case KEY_6:   // show bounding box of each mesh of the model
            {
                int mode = code - (int)(KEY_4);
                graphics_.SetAABBShowMode(CGraphics::AABBShowMode(mode));
                break;
            }
            case KEY_RETURN:   // ENTER
            {
                if (!keyboard_.WasPressedBefore(KEY_RETURN))
                    UI::g_EventsHistory.FlushTempHistory();
                break;
            }
            case KEY_S:
            {
                if (keyboard_.IsPressed(KEY_CONTROL) && pUI->GetSelectedEntt())
                {
                    // handle Ctrl+S: turn on scaling with gizmo if any entt is selected
                    pUI->SetGizmoOperation(ImGuizmo::OPERATION::SCALE);
                }
                else
                {
                    // move backward
                    HandleEditorCameraMovement(code, deltaTime_, pEnttMgr, keyboard_);
                }
                break;
            }
            case KEY_A:
            case KEY_D:
            case KEY_W:
            case KEY_SPACE:
            {
                HandleEditorCameraMovement(code, deltaTime_, pEnttMgr, keyboard_);
            }
            case KEY_Z:
            {
                // if we want to undo the last operation
                if (keyboard_.IsPressed(KEY_CONTROL))
                {
                    if (!keyboard_.WasPressedBefore(KEY_Z))
                        pUI->UndoEditorLastEvent();
                }
                else
                {
                    // move down
                    HandleEditorCameraMovement(code, deltaTime_, pEnttMgr, keyboard_);
                }
                break;
            }
            case KEY_Q:
            {
                // guizmo: turn OFF any operation
                if (keyboard_.IsPressed(KEY_CONTROL))
                    pUI->SetGizmoOperation(ImGuizmo::OPERATION(-1));
                break;
            }
            case KEY_T:
            {
                if (keyboard_.IsPressed(KEY_CONTROL))
                    pUI->SetGizmoOperation(ImGuizmo::OPERATION::TRANSLATE);
                break;
            }
            case KEY_R:
            {
                if (keyboard_.IsPressed(KEY_CONTROL))
                    pUI->SetGizmoOperation(ImGuizmo::OPERATION::ROTATE);
                break;
            }
            case KEY_F1:
            {
                // switch to the game mode
                if (!keyboard_.WasPressedBefore(KEY_F1))
                    SwitchEngineMode();
                break;
            }
            case KEY_F2:
            {
                // switch btw cameras modes (free / game)
                if (!keyboard_.WasPressedBefore(KEY_F2))
                {
                    ECS::PlayerSystem& player = pEnttMgr->playerSystem_;
                    player.SetFreeFlyMode(!player.IsFreeFlyMode());
                }
                break;
            }
            case KEY_F3:
            {
                // shaders hot reload
                if (!keyboard_.WasPressedBefore(KEY_F3))
                    pRender_->ShadersHotReload(graphics_.GetD3DClass().GetDevice());
                break;
            }
            case KEY_F4:
            {
                if (keyboard_.WasPressedBefore(KEY_F4))
                    break;

                // recompute light map for the terrain
                TerrainGeomip& terrain = g_ModelMgr.GetTerrainGeomip();
                TerrainConfig terrainCfg;

                const char* configPath = "data/terrain/terrain.cfg";
                terrain.LoadSetupFile(configPath, terrainCfg);

                // setup the terrain's lighting system
                terrain.SetLightingType(terrainCfg.lightingType);

                terrain.CustomizeSlopeLighting(
                    terrainCfg.lightDirX,
                    terrainCfg.lightDirZ,
                    terrainCfg.lightMinBrightness,
                    terrainCfg.lightMaxBrightness,
                    terrainCfg.shadowSoftness);

                terrain.UnloadLightMap();

                //terrain.UnloadLightMap();
                terrain.CalculateLighting();

                const int bitsPerPixel = 8;
                const LightmapData& lightmap = terrain.lightmap_;
                Texture& tex = g_TextureMgr.GetTexByID(lightmap.id);
                

                g_TextureMgr.RecreateTextureFromRawData(
                    "terrain_light_map",
                    lightmap.pData,
                    lightmap.size,
                    lightmap.size,
                    bitsPerPixel,
                    false,
                    tex);

                // unload lightmap raw data since we're already created a texture resource with it
                terrain.UnloadLightMap();

                break;
            }
            case KEY_F5:
            {
                //if (!keyboard_.WasPressedBefore(KEY_F5))
                //    g_ModelMgr.GetTerrainGeomip().wantDebug_ = true;

                break;
            }
            case VK_ESCAPE:
            {
                // if we pressed the ESC button we exit from the application
                LogDbg(LOG, "Esc is pressed");
                isExit_ = true;
                break;
            }
        }
    }

    // handle released keys
    while (int key = keyboard_.ReadReleasedKey())
    {
        switch (key)
        {
            case KEY_SHIFT:
            {
                //player.SetIsRunning(false);
                pUI->UseSnapping(false);
                break;
            }
        }
    }
}

//---------------------------------------------------------
// Desc:   a handler for all the keyboard events
//---------------------------------------------------------
void Engine::EventKeyboard(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    inputMgr_.HandleKeyboardMessage(keyboard_, uMsg, wParam, lParam);
}

//---------------------------------------------------------
// Desc:   handle mouse events when we're in the editor mode
//---------------------------------------------------------
void Engine::HandleEditorEventMouse(UI::UserInterface* pUI, ECS::EntityMgr* pEnttMgr)
{
    using enum MouseEvent::EventType;
    static bool isMouseMiddlePressed = false;

    while (!mouse_.EventBufferIsEmpty())
    {
        mouseEvent_ = mouse_.ReadEvent();
        MouseEvent::EventType eventType = mouseEvent_.GetEventType();

        switch (eventType)
        {
            case Move:
            {
                // update mouse position data because we need to print mouse position on the screen
                systemState_.mouseX = mouseEvent_.GetPosX();
                systemState_.mouseY = mouseEvent_.GetPosY();
                break;
            }
            case RAW_MOVE:
            {
                // handle moving of the camera around fixed look_at point (if we set any fixed one)
           
                // if we want to rotate a camera
                if (isMouseMiddlePressed)
                {
                    const EntityID camID      = pEnttMgr->nameSystem_.GetIdByName("editor_camera");
                    ECS::CameraSystem& camSys = pEnttMgr->cameraSystem_;

                    // rotate around some particular point
                    if (camSys.IsFixedLook(camID))
                    {
                        assert(0 && "FIXME");
                        //cam.RotateYAroundFixedLook(mouseEvent_.GetPosX() * 0.01f);
                        //cam.PitchAroundFixedLook  (mouseEvent_.GetPosY() * 0.01f);
                    }
                    // rotate around itself
                    else
                    {
                        camSys.Pitch  (camID, mouseEvent_.GetPosY() * deltaTime_);
                        camSys.RotateY(camID, mouseEvent_.GetPosX() * deltaTime_);
                    }
                }
                break;
            }
            case LPress:
            {

                // if we currenly hovering the scene window with our mouse
                // and we don't hover any gizmo we execute entity picking (selection) test
                if (pUI->IsSceneWndHovered() && !pUI->IsGizmoHovered())
                {
                    EntityID selectedEnttID = 0;
                    selectedEnttID = graphics_.TestEnttSelection(mouseEvent_.GetPosX(), mouseEvent_.GetPosY(), pEnttMgr_);

                    // update the UI about selection of the entity
                    if (selectedEnttID)
                    {
                        pUI->SetSelectedEntt(selectedEnttID);
                        //userInterface_.SetGizmoOperation(ImGuizmo::OPERATION::TRANSLATE);
                        pUI->SetGizmoOperation(ImGuizmo::OPERATION(-1));  // turn off the gizmo
                    }
                    else
                    {
                        pUI->SetSelectedEntt(0);
                        pUI->SetGizmoOperation(ImGuizmo::OPERATION(-1));  // turn off the gizmo
                    }

                    // detach camera from any fixed look_at point
                    //graphics_.SetFixedLookState(false);
                }
                break;
            }
            case LRelease:
            {
                if (UI::g_EventsHistory.HasTempHistory())
                    UI::g_EventsHistory.FlushTempHistory();
                break;
            }
            case WheelUp:
            case WheelDown:
            {
                if (pUI->IsSceneWndHovered())
                {
                    // using mouse wheel we move forward or backward along 
                    // the camera direction vector (when hold shift - we move faster)
                    constexpr int editorCameraSpeed = 10;
                    const int sign     = (eventType == WheelUp) ? 1 : -1;
                    const int speedMul = (keyboard_.IsPressed(KEY_SHIFT)) ? 5 : 1;
                    const float step   = deltaTime_ * (editorCameraSpeed * speedMul * sign);

                    const EntityID camID = pEnttMgr->nameSystem_.GetIdByName("editor_camera");
                    pEnttMgr->cameraSystem_.Walk(camID, step);
                }
                break;
            }
            case MPress:             isMouseMiddlePressed = true;  break;
            case MRelease:           isMouseMiddlePressed = false; break;
            case LeftDoubleClick:
            {
                return;
                assert(0 && "FIXME: double click");
                break;
            } // case LeftDoubleClick:
        } // switch
    } // while
}

//---------------------------------------------------------
// Desc:   handle mouse events when we're in the game mode
//---------------------------------------------------------
void Engine::HandleGameEventMouse(UI::UserInterface* pUI, ECS::EntityMgr* pEnttMgr)
{
    mouseEvent_ = mouse_.ReadEvent();

    switch (mouseEvent_.GetEventType())
    {
        case MouseEvent::EventType::Move:
        {
            // update mouse position data because we need to print mouse position on the screen
            systemState_.mouseX = mouseEvent_.GetPosX();
            systemState_.mouseY = mouseEvent_.GetPosY();
            SetCursorPos(800, 450);                           // to prevent the cursor to get out of the window
            break;
        }
        case MouseEvent::EventType::RAW_MOVE:
        {
            // update the rotation data of the camera
            // with the current state of the input devices. The movement function will update
            // the position of the camera to the location for this frame

            ECS::PlayerSystem& player = pEnttMgr->playerSystem_;

            const float rotY  = mouseEvent_.GetPosX() * deltaTime_;
            const float pitch = mouseEvent_.GetPosY() * deltaTime_;
           
            player.RotateY(rotY);
            player.Pitch  (pitch);
    
            break;
        }
    }
}

//---------------------------------------------------------
// Desc:   handler for all the mouse events
//---------------------------------------------------------
void Engine::EventMouse(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    inputMgr_.HandleMouseMessage(mouse_, uMsg, wParam, lParam);

    if (systemState_.isEditorMode)
        HandleEditorEventMouse(pUserInterface_, pEnttMgr_);
}

//---------------------------------------------------------
// Desc:   switch from the game to the editor mode
//---------------------------------------------------------
void Engine::TurnOnEditorMode()
{
    Render::D3DClass& d3d         = graphics_.GetD3DClass();
    const EntityID    editorCamID = pEnttMgr_->nameSystem_.GetIdByName("editor_camera");

    d3d.ToggleFullscreen(hwnd_, false);
    graphics_.SetCurrentCamera(editorCamID);
    systemState_.isGameMode = false;
    systemState_.isEditorMode = true;

    // escape from the "clip cursor" mode so that users can choose to interact
    // with other windows if desired; Note: During the game we may also use such call
    // when go to a 'pause' to get out of 'mouse-look' behavior like this.
    ClipCursor(nullptr);

    
    ShowCursor(TRUE);

    const DirectX::XMMATRIX& baseView   = pEnttMgr_->cameraSystem_.GetBaseView(editorCamID);
    const DirectX::XMMATRIX& ortho      = pEnttMgr_->cameraSystem_.GetOrtho(editorCamID);
    const DirectX::XMMATRIX  WVO        = baseView * ortho;

    ID3D11DeviceContext* pContext = GetGraphicsClass().GetD3DClass().GetDeviceContext();
    pRender_->SetWorldViewOrtho(pContext, WVO);
}

//---------------------------------------------------------
// Desc:   limit the cursor to the be always in the window rectangle
//         (when we're in the game mode)
//---------------------------------------------------------
void TurnOnClipCursorMode(HWND hwnd)
{
    RECT rect;
    GetClientRect(hwnd, &rect);

    POINT ul{ rect.left,  rect.top };        // upper left corner
    POINT lr{ rect.right, rect.bottom};      // lower right corner

    MapWindowPoints(hwnd, nullptr, &ul, 1);
    MapWindowPoints(hwnd, nullptr, &lr, 1);

    // update: left/top/right/bottom
    rect = { ul.x, ul.y, lr.x, lr.y };

    ClipCursor(&rect);
}

//---------------------------------------------------------
// Desc:   switch from the editor to the game mode
//---------------------------------------------------------
void Engine::TurnOnGameMode()
{
    const EntityID gameCamID = pEnttMgr_->nameSystem_.GetIdByName("game_camera");

    graphics_.GetD3DClass().ToggleFullscreen(hwnd_, false);
    graphics_.SetCurrentCamera(gameCamID);
    systemState_.isGameMode = true;
    systemState_.isEditorMode = false;

    ShowCursor(FALSE);
    TurnOnClipCursorMode(hwnd_);

    //const DirectX::XMMATRIX  world      = DirectX::XMMatrixIdentity();
    const DirectX::XMMATRIX& baseView = pEnttMgr_->cameraSystem_.GetBaseView(gameCamID);
    const DirectX::XMMATRIX& ortho    = pEnttMgr_->cameraSystem_.GetOrtho(gameCamID);

    ID3D11DeviceContext* pContext = GetGraphicsClass().GetD3DClass().GetDeviceContext();
    pRender_->SetWorldViewOrtho(pContext, baseView * ortho);
}

} // namespace Core


