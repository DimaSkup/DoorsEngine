// =================================================================================
// Filename:      App.cpp
// 
// Created:       21.04.25  by DimaSkup
// =================================================================================
#include "pch.h"
#include "Application.h"


namespace Game
{

App::App() : wndContainer_(mainHWND_)
{
}

App::~App()
{
    SafeDelete(pFacadeEngineToUI_);
}

///////////////////////////////////////////////////////////

void App::Initialize()
{
    // compute duration of importing process
    auto initStartTime = std::chrono::steady_clock::now();

    // explicitly init Windows Runtime and COM
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        LogErr("can't explicitly initialize Windows Runtime and COM");
        exit(-1);
    }

    eventHandler_.AddEventListener(&engine_);        // set engine class as one of the window events listeners
    wndContainer_.SetEventHandler(&eventHandler_);    // set an event handler for the window container

    InitWindow();
    InitEngine();



    Core::CGraphics&     graphics = engine_.GetGraphicsClass();
    Core::D3DClass&      d3d      = graphics.GetD3DClass();
    ID3D11Device*        pDevice  = d3d.GetDevice();
    ID3D11DeviceContext* pContext = d3d.GetDeviceContext();

    InitScene       (pDevice, settings_);
    InitRenderModule(pDevice, settings_, &render_);

    // create a facade btw the UI and the engine parts
    pFacadeEngineToUI_ = new UI::FacadeEngineToUI(
        pContext,
        &render_,
        &entityMgr_,
        &graphics);

    // initialize the main UserInterface class
    InitGUI(pDevice, d3d.GetWindowWidth(), d3d.GetWindowHeight());

    // after all init all the game related stuff
    game_.Init(&engine_, &entityMgr_, &render_);


    auto initEndTime = std::chrono::steady_clock::now();
    std::chrono::duration<float, std::milli> initDuration = initEndTime - initStartTime;

    // create a str with duration time about the engine initialization process
    const POINT drawAt = { 10, 350 };

    sprintf(g_String, "Init time: %d ms", (int)initDuration.count());
    userInterface_.CreateConstStr(pDevice, g_String, drawAt);

    // print into console and log file info about duration of the initialization 
    SetConsoleColor(GREEN);
    LogMsg("---------------------------------------------");
    LogMsg(g_String);
    LogMsg("---------------------------------------------");
    SetConsoleColor(RESET);
}

///////////////////////////////////////////////////////////

bool App::InitWindow()
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

bool App::InitEngine()
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

bool App::InitScene(ID3D11Device* pDevice, const EngineConfigs& settings)
{
    const Core::D3DClass& d3d = engine_.GetGraphicsClass().GetD3DClass();
    const SIZE windowedSize   = d3d.GetWindowedWndSize();
    const SIZE fullscreenSize = d3d.GetFullscreenWndSize();

    CameraInitParams editorCamParams;
    editorCamParams.nearZ       = settings_.GetFloat("NEAR_Z");
    editorCamParams.farZ        = settings_.GetFloat("FAR_Z");
    editorCamParams.fovInRad    = settings_.GetFloat("FOV_IN_RAD");         // field of view in radians

    editorCamParams.wndWidth    = (float)windowedSize.cx;
    editorCamParams.wndHeight   = (float)windowedSize.cy;
    editorCamParams.aspectRatio = editorCamParams.wndWidth / editorCamParams.wndHeight;

    CameraInitParams gameCamParams = editorCamParams;
    gameCamParams.wndWidth      = (float)fullscreenSize.cx;
    gameCamParams.wndHeight     = (float)fullscreenSize.cy;
    gameCamParams.aspectRatio   = editorCamParams.wndWidth / editorCamParams.wndHeight;


    SceneInitializer sceneInitializer;

    bool result = sceneInitializer.Initialize(
        pDevice,
        entityMgr_,
        editorCamParams,
        gameCamParams);
    if (!result)
    {
        LogErr("can't initialize the scene's some stuff");
    }


    const std::string cameraEnttName = (startInGameMode_) ? "game_camera" : "editor_camera";
    const EntityID    cameraID = entityMgr_.nameSystem_.GetIdByName(cameraEnttName);
    Core::CGraphics&  graphics = engine_.GetGraphicsClass();

    // setup the current camera
    const EntityID editorCamID = entityMgr_.nameSystem_.GetIdByName("editor_camera");
    graphics.SetCurrentCamera(editorCamID);

    // setup distance after which the objects are fully fogged
    //const float fogStart = settings.GetFloat("FOG_START");
    //const float fogRange = settings.GetFloat("FOG_RANGE");
    //const float fullFogDistance = fogStart + fogRange;

    return true;
}

///////////////////////////////////////////////////////////

bool App::InitRenderModule(
    ID3D11Device* pDevice,
    const EngineConfigs& settings,
    Render::CRender* pRender)
{
    // setup render initial params

    // prepare WVO (world * base_view * ortho) matrix for 2D rendering
    const std::string cameraEnttName  = (startInGameMode_) ? "game_camera" : "editor_camera";
    const EntityID cameraID           = entityMgr_.nameSystem_.GetIdByName(cameraEnttName);
 
    const DirectX::XMMATRIX& baseView = entityMgr_.cameraSystem_.GetBaseView(cameraID);
    const DirectX::XMMATRIX& ortho    = entityMgr_.cameraSystem_.GetOrtho(cameraID);
    const DirectX::XMMATRIX  WVO      = baseView * ortho;

    Render::InitParams renderParams;
    renderParams.worldViewOrtho       = DirectX::XMMatrixTranspose(WVO);

    // zaporizha sky box horizon (darker by 0.1f)
    renderParams.fogColor =
    {
        settings.GetFloat("FOG_RED"),
        settings.GetFloat("FOG_GREEN"),
        settings.GetFloat("FOG_BLUE"),
    };
    renderParams.fogStart = settings.GetFloat("FOG_START");
    renderParams.fogRange = settings.GetFloat("FOG_RANGE");

    const DirectX::XMFLOAT3 skyColorCenter = g_ModelMgr.GetSky().GetColorCenter();
    const DirectX::XMFLOAT3 skyColorApex   = g_ModelMgr.GetSky().GetColorApex();

    // setup the fog color according to the sky center color
    renderParams.fogColor.x;// *= skyColorCenter.x;
    renderParams.fogColor.y;// *= skyColorCenter.y;
    renderParams.fogColor.z;// *= skyColorCenter.z;

    
    ID3D11DeviceContext* pContext = nullptr;
    pDevice->GetImmediateContext(&pContext);

    // init the render module
    bool result = pRender->Initialize(pDevice, pContext, renderParams);
    if (!result)
    {
        LogErr("can't init the render module");
        return false;
    }

    pRender->SetSkyGradient(pContext, skyColorCenter, skyColorApex);

    return true;
}

///////////////////////////////////////////////////////////

bool App::InitGUI(
    ID3D11Device* pDevice,
    const int wndWidth,
    const int wndHeight)
{
    // this function initializes the GUI of the game/engine (interface elements, text, etc.);

    SetConsoleColor(YELLOW);
    LogMsg("");
    LogMsg("----------------------------------------------------------");
    LogMsg("                    INITIALIZATION: GUI                   ");
    LogMsg("----------------------------------------------------------\n");

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
    catch (EngineException& e)
    {
        LogErr(e, true);
        return false;
    }
}

///////////////////////////////////////////////////////////

void App::Run()
{
    // run the engine
    while (wndContainer_.renderWindow_.ProcessMessages(hInstance_, mainHWND_) == true)
    {
        if (!engine_.IsPaused())
        {
            const float deltaTime = engine_.GetTimer().GetDeltaTime();

            game_.Update(deltaTime);
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

void App::Close()
{
    eventHandler_.DetachAllEventListeners();
    wndContainer_.renderWindow_.UnregisterWindowClass(hInstance_);
    CloseLogger();
}

}; // namespace Game
