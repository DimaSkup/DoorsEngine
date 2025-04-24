// =================================================================================
// Filename:      Application.cpp
// 
// Created:       21.04.25  by DimaSkup
// =================================================================================
#include "pch.h"
#include "Application.h"


namespace Game
{

Application::Application() : wndContainer_(mainHWND_)
{
}

///////////////////////////////////////////////////////////

Application::~Application()
{
    delete pFacadeEngineToUI_;
}

///////////////////////////////////////////////////////////

bool Application::InitWindow()
{
     // get main params for the window initialization
    const bool isFullScreen     = settings_.GetBool("FULL_SCREEN");
    const std::string wndTitle  = settings_.GetString("WINDOW_TITLE");
    const std::string wndClass  = "MyWindowClass";
    const int wndWidth          = settings_.GetInt("WINDOW_WIDTH");
    const int wndHeight         = settings_.GetInt("WINDOW_HEIGHT");

    // init the main window
    bool result = wndContainer_.renderWindow_.Initialize(
        hInstance_,
        mainHWND_,
        isFullScreen,
        wndTitle,
        wndClass,
        wndWidth,
        wndHeight);

    if (!result)
        LogErr("can't initialize the window");

    return result;
}

///////////////////////////////////////////////////////////

bool Application::InitEngine()
{
    const std::string wndTitle = settings_.GetString("WINDOW_TITLE");

    // init the engine
    bool result = engine_.Initialize(
        hInstance_,
        mainHWND_,
        settings_,
        wndTitle,
        &entityMgr_,
        &userInterface_,
        &render_);

    if (!result)
        LogErr("can't initialize the engine");

    return result;
}

///////////////////////////////////////////////////////////

bool Application::InitScene(ID3D11Device* pDevice)
{
    CameraInitParams params;
    params.nearZ          = settings_.GetFloat("NEAR_Z");
    params.farZ           = settings_.GetFloat("FAR_Z");
    params.fovInRad       = settings_.GetFloat("FOV_IN_RAD");         // field of view in radians

    params.sensitivity    = settings_.GetFloat("CAMERA_SENSITIVITY"); // camera rotation speed
    params.walkSpeed      = settings_.GetFloat("CAMERA_WALK_SPEED");
    params.runSpeed       = settings_.GetFloat("CAMERA_RUN_SPEED");

    const Core::D3DClass& d3d = engine_.GetGraphicsClass().GetD3DClass();
    params.windowedSize   = d3d.GetWindowedWndSize();
    params.fullscreenSize = d3d.GetFullscreenWndSize();

    SceneInitializer sceneInitializer;

    bool result = sceneInitializer.Initialize(pDevice, entityMgr_, params);
    if (!result)
    {
        LogErr("can't initialize the scene's some stuff");
    }


    // initialize a base view matrix with the camera
    // for 2D user interface rendering (in GAME mode)
    const EntityID gameCamID = entityMgr_.nameSystem_.GetIdByName("game_camera");
    DirectX::XMMATRIX baseViewMatrix = entityMgr_.cameraSystem_.GetView(gameCamID);   // is used for 2D rendering
    engine_.GetGraphicsClass().SetBaseViewMatrix(baseViewMatrix);

    // setup the current camera
    const EntityID editorCamID = entityMgr_.nameSystem_.GetIdByName("editor_camera");
    engine_.GetGraphicsClass().SetCurrentCamera(editorCamID);

    return true;
}

///////////////////////////////////////////////////////////

void Application::Initialize()
{
    // ATTENTION: put the declation of logger before all the others; this instance is necessary to create a logger text file
    Core::InitLogger();

    eventHandler_.AddEventListener(&engine_);

    // set an event handler for the window container
    wndContainer_.SetEventHandler(&eventHandler_);

    InitWindow();
    InitEngine();


    // explicitly init Windows Runtime and COM
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    Core::Assert::NotFailed(hr, "ERROR: can't explicitly initialize Windows Runtime and COM");

    // init DirectX stuff
    Core::D3DClass& d3d = engine_.GetGraphicsClass().GetD3DClass();
    ID3D11Device* pDevice = d3d.GetDevice();
    ID3D11DeviceContext* pContext = d3d.GetDeviceContext();

    InitScene(pDevice);

    // create a facade btw the UI and the engine parts
    pFacadeEngineToUI_ = new UI::FacadeEngineToUI(pContext, &render_, &entityMgr_);

    // initialize the main UserInterface class
    InitGUI(pDevice, d3d.GetWindowWidth(), d3d.GetWindowHeight());

    // create a str with duration time of the engine initialization process
    sprintf(g_String, "Engine init time: %f s", engine_.GetTimer().GetGameTime());
    userInterface_.CreateConstStr(pDevice, g_String, { 10, 325 });
}

///////////////////////////////////////////////////////////

bool Application::InitGUI(
    ID3D11Device* pDevice,
    const int wndWidth,
    const int wndHeight)
{
    // this function initializes the GUI of the game/engine (interface elements, text, etc.);

    Core::LogMsgf(" ");
    Core::LogMsgf("%s----------------------------------------------------------", YELLOW);
    Core::LogMsgf("%s                   INITIALIZATION: GUI                    ", YELLOW);
    Core::LogMsgf("%s----------------------------------------------------------", YELLOW);

    try
    {
        std::string fontDataFullFilePath;
        fontDataFullFilePath += g_RelPathUIDataDir;
        fontDataFullFilePath += settings_.GetString("FONT_DATA_FILE_PATH");

        std::string fontTexFullFilePath;
        fontTexFullFilePath += g_RelPathUIDataDir;
        fontTexFullFilePath += settings_.GetString("FONT_TEXTURE_FILE_PATH");

        char videoCardName[128]{ '\0' };
        int videoCardMemory = 0;

        D3DClass& d3d = engine_.GetGraphicsClass().GetD3DClass();
        d3d.GetVideoCardInfo(videoCardName, 128, videoCardMemory);

        // initialize the user interface
        userInterface_.Initialize(
            pDevice,
            d3d.GetDeviceContext(),
            pFacadeEngineToUI_,
            fontDataFullFilePath,
            fontTexFullFilePath,
            wndWidth,
            wndHeight,
            videoCardMemory,
            videoCardName);

        return true;
    }
    catch (Core::EngineException& e)
    {
        Core::LogErr(e, true);
        return false;
    }
}

///////////////////////////////////////////////////////////

void Application::Run()
{
    // run the engine
    while (wndContainer_.renderWindow_.ProcessMessages(hInstance_, mainHWND_) == true)
    {
        if (!engine_.IsPaused())
        {
            engine_.Update();
            engine_.RenderFrame();
        }
        else
        {
            Sleep(100);
        }

        if (engine_.IsExit())
            break;
    }
}

///////////////////////////////////////////////////////////

void Application::Close()
{
    wndContainer_.renderWindow_.UnregisterWindowClass(hInstance_);
    Core::CloseLogger();
}

}; // namespace Game
