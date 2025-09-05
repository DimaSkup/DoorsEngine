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
    //HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
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
    Render::D3DClass&    d3d      = graphics.GetD3DClass();
    ID3D11Device*        pDevice  = d3d.GetDevice();
    ID3D11DeviceContext* pContext = d3d.GetDeviceContext();

    InitRenderModule(pDevice, engineConfigs_, &render_);

    // create a facade btw the UI and the engine parts
    pFacadeEngineToUI_ = new UI::FacadeEngineToUI(
        pContext,
        &render_,
        &entityMgr_,
        &graphics,
        &g_ModelMgr.GetTerrainGeomip());

    // after all init all the game related stuff
    game_.Init(&engine_, &entityMgr_, &render_, engineConfigs_);

    // initialize the main UserInterface class
    InitGUI(pDevice, d3d.GetWindowWidth(), d3d.GetWindowHeight());

 
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
    const bool isFullScreen     = engineConfigs_.GetBool("FULL_SCREEN");
    const std::string wndTitle  = engineConfigs_.GetString("WINDOW_TITLE");
    const std::string wndClass  = "MyWindowClass";
    const int wndWidth          = engineConfigs_.GetInt("WINDOW_WIDTH");
    const int wndHeight         = engineConfigs_.GetInt("WINDOW_HEIGHT");

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

//---------------------------------------------------------
// init the engine (core)
//---------------------------------------------------------
bool App::InitEngine()
{
    const std::string wndTitle = engineConfigs_.GetString("WINDOW_TITLE");

    
    bool result = engine_.Initialize(
        hInstance_,
        mainHWND_,
        engineConfigs_,
        wndTitle,
        &entityMgr_,
        &userInterface_,
        &render_);

    if (!result)
        LogErr("can't initialize the engine");

    return result;
}

//---------------------------------------------------------
// Desc:   prepare params for initialization of the "Render" module and init it
//---------------------------------------------------------
bool App::InitRenderModule(
    ID3D11Device* pDevice,
    const EngineConfigs& settings,
    Render::CRender* pRender)
{

    // prepare WVO (world * base_view * ortho) matrix for 2D rendering
    const bool startInGameMode  = engineConfigs_.GetBool("START_IN_GAME_MODE");
    const char* cameraEnttName  = (startInGameMode) ? "game_camera" : "editor_camera";
    const EntityID cameraID     = entityMgr_.nameSystem_.GetIdByName(cameraEnttName);
 
    const DirectX::XMMATRIX& baseView = entityMgr_.cameraSystem_.GetBaseView(cameraID);
    const DirectX::XMMATRIX& ortho    = entityMgr_.cameraSystem_.GetOrtho(cameraID);
    const DirectX::XMMATRIX  WVO      = baseView * ortho;

    // setup initial params for the "Render" module
    Render::InitParams renderParams;

    renderParams.worldViewOrtho = DirectX::XMMatrixTranspose(WVO);

    // setup fog params
    renderParams.fogColor =
    {
        settings.GetFloat("FOG_RED"),
        settings.GetFloat("FOG_GREEN"),
        settings.GetFloat("FOG_BLUE"),
    };
    renderParams.fogStart = settings.GetFloat("FOG_START");
    renderParams.fogRange = settings.GetFloat("FOG_RANGE");

    // setup horizon and apex (top) color of the sky
    const DirectX::XMFLOAT3 skyColorCenter = g_ModelMgr.GetSky().GetColorCenter();
    const DirectX::XMFLOAT3 skyColorApex   = g_ModelMgr.GetSky().GetColorApex();

    // setup terrain's material
    const Material& terrainMat = g_MaterialMgr.GetMatByName("terrain_mat_geomip");
    renderParams.terrainMatColors = 
    {
        DirectX::XMFLOAT4(&terrainMat.ambient.x),
        DirectX::XMFLOAT4(&terrainMat.diffuse.x),
        DirectX::XMFLOAT4(&terrainMat.specular.x),
        DirectX::XMFLOAT4(&terrainMat.reflect.x)
    };


    ID3D11DeviceContext* pContext = nullptr;
    pDevice->GetImmediateContext(&pContext);

    // init the "Render" module
    bool result = pRender->Initialize(pDevice, pContext, renderParams);
    if (!result)
    {
        LogErr(LOG, "can't init the render module");
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
        fontDataFullFilePath += engineConfigs_.GetString("FONT_DATA_FILE_PATH");

        std::string fontTexFullFilePath;
        fontTexFullFilePath += g_RelPathUIDataDir;
        fontTexFullFilePath += engineConfigs_.GetString("FONT_TEXTURE_FILE_PATH");

        char videoCardName[128]{ '\0' };
        int videoCardMemory = 0;

        Render::D3DClass& d3d = engine_.GetGraphicsClass().GetD3DClass();
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
