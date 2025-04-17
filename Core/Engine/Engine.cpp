// =================================================================================
// Filename:  Engine.cpp
// Created:   05.10.22
// =================================================================================
#include "Engine.h"

#include <CoreCommon/FileSystemPaths.h>
#include "ProjectSaver.h"

#include "DumpGenerator.h"
#include "ImGuizmo.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <winuser.h>

#pragma warning (disable : 4996)

//#include <Psapi.h>    // This header is used by Process Status API (PSAPI)


namespace Core
{

Engine::Engine()
{
    LogDbg("init");
    timer_.Reset();       // reset the engine/game timer

    //ECS_Tester tester;
    //tester.Run();
}

///////////////////////////////////////////////////////////

Engine::~Engine()
{
    LogMsgf(" ");
    LogMsgf("%s%s", YELLOW, "-------------------------------------------------");
    LogMsgf("%s%s", YELLOW, "            START OF THE DESTROYMENT:            ");
    LogMsgf("%s%s", YELLOW, "-------------------------------------------------");
    

    //ProjectSaver projSaver;

    //projSaver.StoreModels(graphics_.GetD3DClass().GetDevice());

    // unregister the window class, destroys the window,
    // reset the responsible members;

    if (hwnd_ != NULL)
    {
        ChangeDisplaySettings(NULL, 0);  // before destroying the window we need to set it to the windowed mode
        DestroyWindow(hwnd_);            // Remove the window
        hwnd_ = NULL;
        hInstance_ = NULL;

        LogDbg("engine desctuctor");
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
    const Settings& settings,
    const std::string& windowTitle,
    ECS::EntityMgr* pEnttMgr,
    UI::UserInterface* pUserInterface,
    Render::CRender* pRender)
{
    //
    // initializes all the main parts of the engine
    //

    // stuff for debug dumps generation (in case of crash)
    if (!IsDebuggerPresent())
        SetUnhandledExceptionFilter(UnhandleExceptionFilter);

    try
    {
        // check support for SSE2 (Pentium4, AMD K8, and above)
        Assert::True(DirectX::XMVerifyCPUSupport(), "XNA math not supported");
        Assert::True(pEnttMgr != nullptr,       "input ptr to the Entity Manager == nullptr");
        Assert::True(pUserInterface != nullptr, "input ptr to the User Interface == nullptr");
        Assert::True(pRender != nullptr,        "input ptr to the Render == nullptr");

        // WINDOW: store a handle to the application instance
        hInstance_ = hInstance;  
        hwnd_ = mainWnd;
        windowTitle_ = windowTitle;

        // init pointers
        pEnttMgr_ = pEnttMgr;
        pUserInterface_ = pUserInterface;
        pRender_ = pRender;


        // GRAPHICS SYSTEM: initialize the graphics system
        bool result = graphics_.Initialize(
            hwnd_,
            systemState_, 
            settings,
            pEnttMgr,
            pRender);
        Assert::True(result, "can't initialize the graphics system");

        D3DClass& d3d                 = graphics_.GetD3DClass();
        ID3D11Device* pDevice         = d3d.GetDevice();
        ID3D11DeviceContext* pContext = d3d.GetDeviceContext();

        systemState_.wndWidth_        = d3d.GetWindowWidth();
        systemState_.wndHeight_       = d3d.GetWindowHeight();
        

#if 0
        // SOUND SYSTEM: initialize the sound obj
        result = sound_.Initialize(hwnd_);
        Assert::True(result, "can't initialize the sound system");
#endif

        // TIMERS: (game timer, CPU)
        timer_.Tick();                 
        cpu_.Initialize();

        imGuiLayer_.Initialize(hwnd_, pDevice, pContext);
        

        LogMsg("is initialized!");
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        return false;
    }

    return true;
}

/////////////////////////////////////////////////

void Engine::RenderModelIntoTexture(
    ID3D11DeviceContext* pContext,
    FrameBuffer& frameBuffer)
{
#if 0
    using namespace DirectX;

    frameBuffer.ClearBuffers(pContext, {0.5f,0.5f,0.5f,1.0f});
    frameBuffer.Bind(pContext);

    Camera& cam = graphics_.GetEditorCamera();
    cam.SetFixedLookAt({ 0,0,0 });
    cam.SetDistanceToFixedLookAt(2.0f);


    BasicModel& model = graphics_.GetModelsStorage().GetModelByName("Barrel1");

    const BoundingBox& aabb = model.GetModelAABB();
    const XMVECTOR extents  = XMLoadFloat3(&aabb.Extents);
    const XMVECTOR center   = XMLoadFloat3(&aabb.Center);

    XMVECTOR length = XMVector3Length(extents);
    float scaleFactor = 1.0f / XMVectorGetX(length);

    XMVECTOR scaledCenter = center * scaleFactor;
    XMFLOAT3 camCenter;
    XMStoreFloat3(&camCenter, scaledCenter);

    const float piDiv3 = DirectX::XM_PI / 3.0f;
    const float piDiv6 = DirectX::XM_PI / 6.0f;
    const float piDiv4 = DirectX::XM_PIDIV4;

    XMFLOAT3 camPos;

    
    XMMATRIX world = 
        DirectX::XMMatrixScaling(scaleFactor, scaleFactor, scaleFactor) *
        DirectX::XMMatrixTranslation(-aabb.Center.x * scaleFactor, -aabb.Center.y * scaleFactor, -aabb.Center.z * scaleFactor);


    graphics_.RenderModel(model, world);

    graphics_.GetD3DClass().ResetBackBufferRenderTarget();
    graphics_.GetD3DClass().ResetViewport();
#endif
}



///////////////////////////////////////////////////////////

void Engine::Update()
{
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
    deltaTime_ = (deltaTime_ > 16.6f) ? 16.6f : deltaTime_;

    // update the entities and related data
    pEnttMgr_->Update(timer_.GetGameTime(), deltaTime_);
    //entityMgr_.lightSystem_.UpdateSpotLights(cameraPos, cameraDir);

    // compute fps and frame time (ms)
    CalculateFrameStats();

    pUserInterface_->Update(graphics_.GetD3DClass().GetDeviceContext(), systemState_);

    if (keyboard_.IsAnyPressed() || keyboard_.HasReleasedEvents())
    {
        // according to the engine mode we call a respective keyboard handler
        if (systemState_.isEditorMode)
        {
            HandleEditorEventKeyboard(pUserInterface_, pEnttMgr_);
        }
        else
        {
            HandleGameEventKeyboard(pUserInterface_, pEnttMgr_);
        }
        keyboard_.Update();
    }

    graphics_.Update(systemState_, deltaTime_, timer_.GetGameTime(), pEnttMgr_, pRender_);
    
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
        systemState_.frameTime = 1000.0f / (float)frameCount;  // ms per frame

        // reset for next average
        frameCount = 0;
        ++timeElapsed;

        // print FPS/frame_time as the window caption
#if 0
        PROCESS_MEMORY_COUNTERS pmc;
        DWORD processID;
        GetWindowThreadProcessId(hwnd_, &processID);

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
        Assert::True(hProcess != NULL, "can't get a process handle of the window");
        
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

void Engine::RenderMaterialsIcons(
    ID3D11ShaderResourceView** outArrShaderResourceViews,
    const size numShaderResourceViews,
    const int iconWidth,
    const int iconHeight)
{
    // render material icon (just sphere model with particular material) into
    // frame buffer and store shader resource view into the input arr;
    //
    // outArrShaderResourceViews is supposed to be already allocated to size numShaderResourceViews

    Assert::True(outArrShaderResourceViews != nullptr, "input ptr to arr of shader resource views == nullptr");

    D3DClass&            d3d = graphics_.GetD3DClass();
    ID3D11Device*        pDevice = d3d.GetDevice();
    ID3D11DeviceContext* pContext = d3d.GetDeviceContext();

    // setup params for the frame buffer (we will use the same params for each)
    FrameBufferSpecification frameBufSpec;

    frameBufSpec.width          = (UINT)iconWidth;
    frameBufSpec.height         = (UINT)iconHeight;
    frameBufSpec.format         = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
    frameBufSpec.screenNear     = d3d.GetScreenNear();
    frameBufSpec.screenDepth    = d3d.GetScreenDepth();

    // alloc memory for frame buffers and init each frame buffer
    materialsFrameBuffers_.resize(numShaderResourceViews);

    for (int i = 0; FrameBuffer& buf : materialsFrameBuffers_)
    {
        bool result = buf.Initialize(pDevice, frameBufSpec);
        if (!result)
        {
            sprintf(g_String, "can't initialize a frame buffer (idx: %d)", i);
            LogErr(g_String);
        }
        ++i;
    }

    // render scene into each frame buffer
    for (int i = 0; FrameBuffer& buf : materialsFrameBuffers_)
    {
        buf.ClearBuffers(pContext, { 0.5f,0.5f,0.5f,1.0f });
        buf.Bind(pContext);
        graphics_.Render3D(pEnttMgr_, pRender_);
    }
}

///////////////////////////////////////////////////////////

void Engine::RenderFrame()
{
    // this function executes rendering of each frame;
    try
    {
        using namespace DirectX;

        D3DClass& d3d = graphics_.GetD3DClass();
        ID3D11DeviceContext* pContext = d3d.GetDeviceContext();

       
        if (systemState_.isEditorMode)
        {
            // Clear all the buffers before frame rendering and render our 3D scene
            d3d.ResetBackBufferRenderTarget();
            d3d.ResetViewport();

            d3d.BeginScene();
            graphics_.Render3D(pEnttMgr_, pRender_);
                                
            //RenderModelIntoTexture(pContext, frameBuffer_);

            // begin rendering of the editor elements
            imGuiLayer_.Begin();

            //pFacadeEngineToUI_->pFrameBufTexSRV_ = frameBuffer_.GetSRV();
#if 0
            if (ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoMove))
            {
                ImGui::Image((ImTextureID)frameBuffer_.GetSRV(), { 500 * 1.777f, 500 });
            }
            ImGui::End();
#endif

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
            RenderUI(pUserInterface_, pRender_);
        }

        // Show the rendered stuff on the screen
        d3d.EndScene();

        // before next frame
        graphics_.ClearRenderingDataBeforeFrame(pEnttMgr_, pRender_);
    }
    catch (EngineException & e)
    {
        LogErr(e, true);
        LogErr("can't render a frame");
        
        // exit after it
        isExit_ = true;
    }
}

///////////////////////////////////////////////////////////

void Engine::RenderUI(UI::UserInterface* pUI, Render::CRender* pRender)
{
    D3DClass& d3d = graphics_.GetD3DClass();

    // preparation before 2D rendering
    d3d.TurnZBufferOff();
    d3d.TurnOnBlending(ALPHA_ENABLE);
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
            pRender->GetShadersContainer().fontShader_,
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
        PostQuitMessage(0);
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



// =================================================================================
// Keyboard events handlers
// =================================================================================

void Engine::HandleEditorEventKeyboard(UI::UserInterface* pUI, ECS::EntityMgr* pEnttMgr)
{
    Camera& cam = graphics_.GetEditorCamera();

    // go through each currently pressed key and handle related events
    for (const eKeyCodes code : keyboard_.GetPressedKeysList())
    {
        switch (code)
        {
            case KEY_SHIFT:
            {
                graphics_.editorCamera_.SetIsRunning(true);
                pUI->UseSnapping(true);
                break;
            }
            // case (keys 0-3): switch the number of directional lights
            case KEY_0:
            case KEY_1:
            case KEY_2:
            case KEY_3:
            {
                assert(0 && "FIXME!");

#if 0
                ID3D11DeviceContext* pContext = graphics_.GetD3DClass().GetDeviceContext();
                int numDirLights = code - (int)(KEY_0);
                pRender->SetDirLightsCount(pContext, numDirLights);
#endif
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
            
            case KEY_RETURN:   // aka ENTER
            {
                if (!keyboard_.WasPressedBefore(KEY_RETURN))
                    UI::gEventsHistory.FlushTempHistory();
                break;
            }
            case KEY_A:
            case KEY_D:
            {
                if (cam.IsFixedLook())
                {
                    // handle moving of the camera around fixed look_at point (if we set any fixed one)
                    int sign = (code == KEY_A) ? +1 : -1;
                    cam.RotateYAroundFixedLook(deltaTime_ * sign);
                }
                else
                {
                    // handle moving left/right
                    int sign = (code == KEY_D) ? +1 : -1;
                    cam.Strafe(cam.GetSpeed() * deltaTime_ * sign);
                }

                graphics_.UpdateCameraEntity("editor_camera", cam.View(), cam.Proj());
                UpdateFlashLightPosition(cam.GetPosition(), pEnttMgr);

                break;
            }
            case KEY_W:
            {
                if (!cam.IsFixedLook())                       // handle moving forward
                {
                    cam.Walk(cam.GetSpeed() * deltaTime_);
                }
                else                                          // handle moving up of the camera around fixed look_at point (if we set any fixed one)
                {
                    cam.PitchAroundFixedLook(deltaTime_);
                }

                graphics_.UpdateCameraEntity("editor_camera", cam.View(), cam.Proj());
                UpdateFlashLightPosition(cam.GetPosition(), pEnttMgr);

                break;
            }
            case KEY_S:
            {
                if (cam.IsFixedLook())                        // handle moving down of the camera around fixed look_at point (if we set any fixed one)
                {
                    cam.PitchAroundFixedLook(-deltaTime_);
                }
                else if (!keyboard_.IsPressed(KEY_CONTROL))   // handle moving backward
                {
                    cam.Walk(-cam.GetSpeed() * deltaTime_);  
                }
                else if (keyboard_.IsPressed(KEY_CONTROL) && pUI->GetSelectedEntt())
                {
                    // handle Ctrl+S: turn on scaling with gizmo if any entt is selected
                    pUI->SetGizmoOperation(ImGuizmo::OPERATION::SCALE);
                }
                    
                graphics_.UpdateCameraEntity("editor_camera", cam.View(), cam.Proj());
                UpdateFlashLightPosition(cam.GetPosition(), pEnttMgr);

                break;
            }
            case KEY_L:
            {
                assert(0 && "FIXME!");

#if 0
                // switch the flashlight
                if (!keyboard_.WasPressedBefore(KEY_L))
                {
                    SwitchFlashLight(cam);
                }
#endif
                break;
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
                    graphics_.editorCamera_.MoveUp(-cam.GetSpeed() * deltaTime_);
                    UpdateFlashLightPosition(cam.GetPosition(), pEnttMgr);
                }
                
                break;
            }
            case KEY_SPACE:
            {
                graphics_.editorCamera_.MoveUp(cam.GetSpeed() * deltaTime_);
                UpdateFlashLightPosition(cam.GetPosition(), pEnttMgr);
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
                    TurnOnGameMode();
                break;
            }
            case VK_ESCAPE:
            {
                // if we pressed the ESC button we exit from the application
                LogDbg("Esc is pressed");
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
                graphics_.editorCamera_.SetIsRunning(false);
                pUI->UseSnapping(false);
                break;
            }
        }
    }
}

///////////////////////////////////////////////////////////

void Engine::HandleGameEventKeyboard(UI::UserInterface* pUI, ECS::EntityMgr* pEnttMgr)
{
    Camera& cam = graphics_.gameCamera_;

    // go through each currently pressed key and handle related events
    for (const eKeyCodes code : keyboard_.GetPressedKeysList())
    {
        switch (code)
        {
            case VK_ESCAPE:
            {
                // if we pressed the ESC button we exit from the application
                LogDbg("Esc is pressed");
                isExit_ = true;
                break;
            }
            case KEY_SHIFT:
            {
                cam.SetIsRunning(true);
                break;
            }
            // (keys A/D): handle moving left/right
            case KEY_A: 
            {
                cam.Strafe(-cam.GetSpeed() * deltaTime_);
                UpdateFlashLightPosition(cam.GetPosition(), pEnttMgr);
                break;
            }
            case KEY_D:
            {
                cam.Strafe(cam.GetSpeed() * deltaTime_);
                UpdateFlashLightPosition(cam.GetPosition(), pEnttMgr);
                break;
            }
            // (keys W/S): handle moving forward/backward
            case KEY_S:
            {
                cam.Walk(-cam.GetSpeed() * deltaTime_);
                UpdateFlashLightPosition(cam.GetPosition(), pEnttMgr);
                break;
            }
            case KEY_W:
            {
                cam.Walk(cam.GetSpeed() * deltaTime_);
                UpdateFlashLightPosition(cam.GetPosition(), pEnttMgr);
                break;
            }
            case KEY_Z:
            {
                cam.MoveUp(-cam.GetSpeed() * deltaTime_);
                UpdateFlashLightPosition(cam.GetPosition(), pEnttMgr);
                break;
            }
            case KEY_SPACE:
            {
                cam.MoveUp(cam.GetSpeed() * deltaTime_);
                UpdateFlashLightPosition(cam.GetPosition(), pEnttMgr);
                break;
            }
            case KEY_L:
            {
                assert(0 && "FIXME!");

#if 0
                // switch the flashlight
                if (!keyboard_.WasPressedBefore(KEY_L))
                {
                    SwitchFlashLight(cam);
                }
#endif
                break;
            }
            case KEY_F1:
            {
                // switch from game to the editor mode
                if (!keyboard_.WasPressedBefore(KEY_F1))
                    TurnOnEditorMode();
                break;
            }
            case KEY_F2:
            {
                // switch btw cameras modes (free / game)
                if (!keyboard_.WasPressedBefore(KEY_F2))
                {
                    cam.SetFreeCamera(!cam.IsFreeCamera());
                }
                break;
            }
            case KEY_F3:
            {
                // show/hide debug info in the game mode
                if (!keyboard_.WasPressedBefore(KEY_F3))
                    systemState_.isShowDbgInfo = !systemState_.isShowDbgInfo;
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
                graphics_.gameCamera_.SetIsRunning(false);
                break;
            }
        }
    }
}

///////////////////////////////////////////////////////////

void Engine::EventKeyboard(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // a handler for all the keyboard events
    inputMgr_.HandleKeyboardMessage(keyboard_, uMsg, wParam, lParam);
}


// =================================================================================
// Mouse events handlers
// =================================================================================

void Engine::SwitchFlashLight(
    const Camera& camera,
    ECS::EntityMgr& mgr,
    Render::CRender& render)
{
    const EntityID flashlightID   = mgr.nameSystem_.GetIdByName("flashlight");
    const bool isFlashlightActive = !mgr.lightSystem_.IsLightActive(flashlightID);   // invert the state
    ID3D11DeviceContext* pContext = graphics_.GetD3DClass().GetDeviceContext();

    mgr.lightSystem_.SetLightIsActive(flashlightID, isFlashlightActive);
    render.SwitchFlashLight(pContext, isFlashlightActive);

    // if we just turned on the flashlight we need to update its position and direction
    if (isFlashlightActive)
    {
        graphics_.UpdateCameraEntity("editor_camera", camera.View(), camera.Proj());

        const DirectX::XMFLOAT3 pos = camera.GetPosition();
        const DirectX::XMFLOAT3 dir = camera.GetLook();
        const DirectX::XMVECTOR dirQuat = DirectX::XMQuaternionRotationRollPitchYaw(dir.y, dir.x, dir.z);

        // update position
        mgr.transformSystem_.SetPositionByID(flashlightID, pos);
        mgr.lightSystem_.SetSpotLightProp(flashlightID, ECS::LightProp::POSITION, { pos.x, pos.y, pos.z, 1.0f });

        // update direction
        mgr.lightSystem_.SetSpotLightProp(flashlightID, ECS::LightProp::DIRECTION, { dir.x, dir.y, dir.z, 0.0f });
    }
}

///////////////////////////////////////////////////////////

void Engine::UpdateFlashLightPosition(const DirectX::XMFLOAT3& pos, ECS::EntityMgr* pEnttMgr)
{
    ECS::EntityMgr& mgr = *pEnttMgr;
    const EntityID flashlightID   = mgr.nameSystem_.GetIdByName("flashlight");
    const bool isFlashlightActive = mgr.lightSystem_.IsLightActive(flashlightID);

    if (isFlashlightActive)
    {
        mgr.transformSystem_.SetPositionByID(flashlightID, pos);
        mgr.lightSystem_.SetSpotLightProp(flashlightID, ECS::LightProp::POSITION, { pos.x, pos.y, pos.z, 1.0f });
    }
}

///////////////////////////////////////////////////////////

void Engine::UpdateFlashLightDirection(const DirectX::XMFLOAT3& dir, ECS::EntityMgr* pEnttMgr)
{
    ECS::EntityMgr& mgr = *pEnttMgr;
    const EntityID flashlightID   = mgr.nameSystem_.GetIdByName("flashlight");
    const bool isFlashlightActive = mgr.lightSystem_.IsLightActive(flashlightID);

    if (isFlashlightActive)
    {
        const DirectX::XMVECTOR dirQuat = DirectX::XMQuaternionRotationRollPitchYaw(dir.y, dir.x, dir.z);
        //mgr.transformSystem_.SetDirectionQuatByID(flashlightID, dirQuat);
        mgr.lightSystem_.SetSpotLightProp(flashlightID, ECS::LightProp::DIRECTION, { dir.x, dir.y, dir.z, 0.0f });
    }
}

///////////////////////////////////////////////////////////

void Engine::HandleEditorEventMouse(UI::UserInterface* pUI, ECS::EntityMgr* pEnttMgr)
{
    using enum MouseEvent::EventType;

    static bool isMouseMiddlePressed = false;
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
            Camera& cam = graphics_.editorCamera_;

            // if we want to rotate a camera
            if (isMouseMiddlePressed)
            {
                // rotate around some particular point
                if (cam.IsFixedLook())
                {
                    cam.RotateYAroundFixedLook(mouseEvent_.GetPosX() * 0.01f);
                    cam.PitchAroundFixedLook  (mouseEvent_.GetPosY() * 0.01f);
                }
                // rotate around itself
                else
                {
                    cam.Pitch  (mouseEvent_.GetPosY() * deltaTime_);
                    cam.RotateY(mouseEvent_.GetPosX() * deltaTime_);

                    // if flashlight is active we update its direction 
                    UpdateFlashLightDirection(cam.GetLook(), pEnttMgr);
                }

                graphics_.UpdateCameraEntity("editor_camera", cam.View(), cam.Proj());
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
                graphics_.editorCamera_.SetFixedLookState(false);
            }
            break;
        }
        case LRelease:
        {
            if (UI::gEventsHistory.HasTempHistory())
                UI::gEventsHistory.FlushTempHistory();
            break;
        }
        case WheelUp:
        case WheelDown:
        {
            if (pUI->IsSceneWndHovered())
            {
                Camera& cam = graphics_.GetEditorCamera();
            
                // using mouse wheel we move forward or backward along 
                // the camera direction vector
                int sign          = (eventType == WheelUp) ? 1 : -1;
                float cameraSpeed = cam.GetSpeed() * 3.0f;
                float speed       = (keyboard_.IsPressed(KEY_SHIFT)) ? cameraSpeed * 5.0f : cameraSpeed;
                float step        = speed * deltaTime_ * sign;

                cam.Walk(step);

                graphics_.UpdateCameraEntity("editor_camera", cam.View(), cam.Proj());
                UpdateFlashLightPosition(cam.GetPosition(), pEnttMgr);
            }
            break;
        }
        case MPress:
        {
            isMouseMiddlePressed = true;
            break;
        }
        case MRelease:
        {
            isMouseMiddlePressed = false;
            break;
        }
        case LeftDoubleClick:
        {
            // handle double click right on selected entity
            if (pUI->IsSceneWndHovered() && (pUI->GetSelectedEntt() != 0))
            {
                using namespace DirectX;

                Camera& cam = graphics_.editorCamera_;
                EntityID selectedEnttID = pUI->GetSelectedEntt();
                XMFLOAT3 enttPos = pEnttMgr->transformSystem_.GetPositionByID(selectedEnttID);

                // fix on the entt and move closer to it
                if (keyboard_.IsPressed(KEY_CONTROL))
                {
                    // compute new position for the camera
                    XMVECTOR lookAt = DirectX::XMLoadFloat3(&enttPos);
                    XMVECTOR camOldPos = cam.GetPositionVec();
                    XMVECTOR camDir = lookAt - camOldPos;

                    // p = p0 + v*t
                    XMVECTOR newPos = lookAt - (camDir * 0.1f);

                    // focus camera on entity
                    cam.LookAt(newPos, lookAt, { 0,1,0 });

                    // set fixed focus on the selected entity so we will move around it
                    // (to turn off fixed focus just click aside of the entity)
                    cam.SetFixedLookState(true);
                    cam.SetFixedLookAtPoint(lookAt);
                }
                else
                {
                    // focus (but not fix) camera on entity
                    cam.LookAt(cam.GetPosition(), enttPos, { 0,1,0 });
                }
                
                //userInterface_.SetGizmoOperation(ImGuizmo::OPERATION::TRANSLATE);
            }
            break;
        }
    } // switch
}

///////////////////////////////////////////////////////////

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

            break;
        }
        case MouseEvent::EventType::RAW_MOVE:
        {
            // update the rotation data of the camera
            // with the current state of the input devices. The movement function will update
            // the position of the camera to the location for this frame
            Camera& cam = graphics_.GetGameCamera();
            cam.Pitch  (mouseEvent_.GetPosY() * deltaTime_);
            cam.RotateY(mouseEvent_.GetPosX() * deltaTime_);
    
            // if flashlight is active we update its direction 
            UpdateFlashLightDirection(cam.GetLook(), pEnttMgr);

            break;
        }
    }
}

///////////////////////////////////////////////////////////

void Engine::EventMouse(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // handler for all the mouse events;

    inputMgr_.HandleMouseMessage(mouse_, uMsg, wParam, lParam);

    // according to the engine mode we call a respective keyboard handler
    if (systemState_.isEditorMode)
    {
        while (!mouse_.EventBufferIsEmpty())
            HandleEditorEventMouse(pUserInterface_, pEnttMgr_);
    }
    else
    {
        while (!mouse_.EventBufferIsEmpty())
            HandleGameEventMouse(pUserInterface_, pEnttMgr_);
    }
}


// =================================================================================
// Private helpers
// =================================================================================

void Engine::TurnOnEditorMode()
{
    // switch from the game to the editor mode

    D3DClass& d3d = graphics_.GetD3DClass();
    Camera& cam = graphics_.GetEditorCamera();

    d3d.ToggleFullscreen(hwnd_, false);
    graphics_.SwitchGameMode(false);

    // update the camera proj matrix according to new window size
    cam.SetProjection(
        cam.GetFovY(),
        d3d.GetAspectRatio(),
        d3d.GetScreenNear(),
        d3d.GetScreenDepth());

    UpdateFlashLightPosition(cam.GetPosition(), pEnttMgr_);
    UpdateFlashLightDirection(cam.GetLook(), pEnttMgr_);

    systemState_.isEditorMode = true;
    ShowCursor(TRUE);

}

///////////////////////////////////////////////////////////

void Engine::TurnOnGameMode()
{
    // switch from the editor to the game mode

    graphics_.GetD3DClass().ToggleFullscreen(hwnd_, true);
    graphics_.SwitchGameMode(true);

    Camera& cam = graphics_.GetGameCamera();
    UpdateFlashLightPosition(cam.GetPosition(), pEnttMgr_);
    UpdateFlashLightDirection(cam.GetLook(), pEnttMgr_);

    systemState_.isEditorMode = false;
    ShowCursor(FALSE);
}

} // namespace Core


