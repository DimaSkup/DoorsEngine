// =================================================================================
// Filename:      App.cpp
// 
// Created:       21.04.25  by DimaSkup
// =================================================================================
#include "../Common/pch.h"
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


//---------------------------------------------------------
// some typedefs and helpers to get timings
//---------------------------------------------------------
using TimeDurationMs = std::chrono::duration<float, std::milli>;

inline std::chrono::steady_clock::time_point GetTimePoint()
{
    return std::chrono::steady_clock::now();
}

//---------------------------------------------------------
// Desc:   do all the init stuff for the engine:
//         init window, init engine's modules, load scene elements, etc.
//---------------------------------------------------------
void App::Init()
{
    // compute duration of importing process
    auto initStartTime = GetTimePoint();

    // explicitly init Windows Runtime and COM
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't explicitly initialize Windows Runtime and COM");
        exit(-1);
    }

    eventHandler_.AddEventListener(&engine_);        // set engine class as one of the window events listeners
    wndContainer_.SetEventHandler(&eventHandler_);   // set an event handler for the window container

    Render::CRender* pRender = &Render::g_Render;

    engine_.BindRender(pRender);
    engine_.BindECS(&entityMgr_);
    engine_.BindUI(&userInterface_);

    InitWindow();
    InitRender(engineConfigs_);
    InitEngine();

    Core::CGraphics&     graphics = engine_.GetGraphicsClass();
    Render::D3DClass&    d3d      = pRender->GetD3D();
    ID3D11Device*        pDevice  = d3d.GetDevice();
    ID3D11DeviceContext* pContext = d3d.GetDeviceContext();

    // after initialization of entine's modules we init the game related stuff
    // (model entities, light source, particle emitters, etc.)
    game_.Init(&engine_, &entityMgr_, pRender, engineConfigs_);

    Core::TerrainGeomip& terrain = Core::g_ModelMgr.GetTerrainGeomip();

    // create a facade btw the UI and the engine parts
    pFacadeEngineToUI_ = new UI::FacadeEngineToUI(
        &engine_,
        pContext,
        pRender,
        &entityMgr_,
        &graphics,
        &terrain);

    // initialize the main UserInterface class
    InitGUI(pDevice, d3d.GetWindowWidth(), d3d.GetWindowHeight());
 

    // create a str with duration time about the engine initialization process
    const TimeDurationMs initDurationMs = GetTimePoint() - initStartTime;
    const POINT drawAt = { 10, 820 };
    char initTimeBuf[32]{'\0'};
    snprintf(initTimeBuf, 32, "Init time: %d ms", (int)initDurationMs.count());
    userInterface_.AddConstStr(pDevice, initTimeBuf, drawAt);


    // for some textures do its binding only once
    engine_.BindBindlessTextures("data/bindless_textures.cfg");

    // print into console and log file info about duration of the initialization 
    SetConsoleColor(GREEN);
    LogMsg("---------------------------------------------");
    LogMsg(g_String);
    LogMsg("---------------------------------------------");
    SetConsoleColor(RESET);
}

//---------------------------------------------------------
// Desc:  initialize the main window
//---------------------------------------------------------
bool App::InitWindow()
{
    // get main params for the window initialization
    const bool isFullScreen     = engineConfigs_.GetBool("FULL_SCREEN");
    const std::string wndTitle  = engineConfigs_.GetString("WINDOW_TITLE");
    const std::string wndClass  = "MyWindowClass";
    const int wndWidth          = engineConfigs_.GetInt("WINDOW_WIDTH");
    const int wndHeight         = engineConfigs_.GetInt("WINDOW_HEIGHT");

   
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
void App::InitEngine()
{
    const char* wndTitle = engineConfigs_.GetString("WINDOW_TITLE");

    engine_.Init(hInstance_, mainHWND_, engineConfigs_, wndTitle);
}

//---------------------------------------------------------
// Desc:   prepare params for initialization of the "Render" module and init it
//---------------------------------------------------------
bool App::InitRender(const Core::EngineConfigs& cfgs)
{
    Render::CRender* pRender = &Render::g_Render;

    // prepare WVO (world * base_view * ortho) matrix for 2D rendering
    const bool startInGameMode  = cfgs.GetBool("START_IN_GAME_MODE");
    const char* cameraEnttName  = (startInGameMode) ? "game_camera" : "editor_camera";
    const EntityID cameraId     = entityMgr_.nameSys_.GetIdByName(cameraEnttName);
 
    const DirectX::XMMATRIX& baseView = entityMgr_.cameraSys_.GetBaseView(cameraId);
    const DirectX::XMMATRIX& ortho    = entityMgr_.cameraSys_.GetOrtho(cameraId);
    const DirectX::XMMATRIX  WVO      = baseView * ortho;

    // setup initial params for the "Render" module
    Render::InitParams renderParams;

    renderParams.worldViewOrtho = DirectX::XMMatrixTranspose(WVO);

    // setup fog params
    renderParams.fogColor.x     = cfgs.GetFloat("FOG_RED");
    renderParams.fogColor.y     = cfgs.GetFloat("FOG_GREEN");
    renderParams.fogColor.z     = cfgs.GetFloat("FOG_BLUE");
    renderParams.fogStart       = cfgs.GetFloat("FOG_START");
    renderParams.fogRange       = cfgs.GetFloat("FOG_RANGE");

    // setup horizon and apex (top) color of the sky
    const Core::SkyModel&   sky            = Core::g_ModelMgr.GetSky();
    const DirectX::XMFLOAT3 skyColorCenter = sky.GetColorCenter();
    const DirectX::XMFLOAT3 skyColorApex   = sky.GetColorApex();

    renderParams.vsyncEnabled   = cfgs.GetBool("VSYNC_ENABLED");
    renderParams.fullscreen     = cfgs.GetBool("FULL_SCREEN");
    renderParams.enable4xMSAA   = cfgs.GetBool("ENABLE_4X_MSAA");
    renderParams.nearZ          = cfgs.GetFloat("GAME_CAM_NEAR_Z");
    renderParams.farZ           = cfgs.GetFloat("GAME_CAM_FAR_Z");


    // init the "Render" module
    bool result = pRender->Init(mainHWND_, renderParams);
    if (!result)
    {
        LogErr(LOG, "can't init the render module");
        return false;
    }

    // do some setup after initialization
    pRender->SetSkyGradient(skyColorCenter, skyColorApex);
    pRender->SetGrassDistFullSize(cfgs.GetFloat("GRASS_DIST_FULL_SIZE"));
    pRender->SetGrassDistVisible (cfgs.GetFloat("GRASS_DIST_VISIBLE"));

    return true;
}

//---------------------------------------------------------
// Desc: initialize the GUI of the game / engine (interface elements, text, etc.);
//---------------------------------------------------------
bool App::InitGUI(
    ID3D11Device* pDevice,
    const int wndWidth,
    const int wndHeight)
{
    SetConsoleColor(YELLOW);
    LogMsg("");
    LogMsg("----------------------------------------------------------");
    LogMsg("                    INITIALIZATION: GUI                   ");
    LogMsg("----------------------------------------------------------\n");

    try
    {
        char dbgFontDataFilepath[64]{'\0'};
        char dbgFontTexName[MAX_LEN_TEX_NAME]{'\0'};
        char gameFontDataFilepath[64]{'\0'};
        char gameFontTexName[MAX_LEN_TEX_NAME]{'\0'};

        strcat(dbgFontDataFilepath, g_RelPathUIDataDir);
        strcat(dbgFontDataFilepath, engineConfigs_.GetString("DBG_FONT_DATA_PATH"));

        strcat(gameFontDataFilepath, g_RelPathUIDataDir);
        strcat(gameFontDataFilepath, engineConfigs_.GetString("GAME_FONT_DATA_PATH"));

        strcat(dbgFontTexName,  engineConfigs_.GetString("DBG_FONT_TEX_NAME"));
        strcat(gameFontTexName, engineConfigs_.GetString("GAME_FONT_TEX_NAME"));


        char videoCardName[128]{ '\0' };
        int videoCardMemory = 0;

        Render::g_Render.GetD3D().GetVideoCardInfo(videoCardName, 128, videoCardMemory);

        // initialize the user interface
        userInterface_.Initialize(
            pDevice,
            pFacadeEngineToUI_,
            dbgFontDataFilepath,
            dbgFontTexName,
            gameFontDataFilepath,
            gameFontTexName,
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

//---------------------------------------------------------
// Desc:  run application's infinite/game loop
//---------------------------------------------------------
void App::Run()
{
    while (wndContainer_.renderWindow_.ProcessMessages(hInstance_, mainHWND_) == true)
    {
        if (!engine_.IsPaused())
        {
            engine_.GetTimer().Tick();

            const float deltaTime = engine_.GetTimer().GetDeltaTime();
            const float gameTime  = engine_.GetTimer().GetGameTime();

            // update game and engine
            Update(deltaTime, gameTime);

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


//---------------------------------------------------------
// Desc:  update the game and then update the engine;
//        aslo calculate duration of updating process
//---------------------------------------------------------
void App::Update(const float deltaTime, const float gameTime)
{
    // calc duration of updating process
    static int   numFramesHalfSec = 0;
    static float sumTime          = 0;
    static float sumUpdateTime    = 0;

    const auto startTimestamp = GetTimePoint();

    game_.Update(deltaTime, gameTime);
    const auto gameUpdatedTimestamp = GetTimePoint();

    engine_.Update(deltaTime, gameTime);
    auto endTimestamp = GetTimePoint();


    // calc the duration of the whole update process and its stages
    const TimeDurationMs durGameUpdate   = gameUpdatedTimestamp - startTimestamp;
    const TimeDurationMs durEngineUpdate = endTimestamp - gameUpdatedTimestamp;
    const TimeDurationMs updateDuration  = durGameUpdate + durEngineUpdate;

    Core::SystemState& sysState = engine_.GetSystemState();
    sysState.updateTime       = updateDuration.count();
    sysState.updateTimeGame   = durGameUpdate.count();
    sysState.updateTimeEngine = durEngineUpdate.count();

    sumUpdateTime += sysState.updateTime;
    numFramesHalfSec++;
    sumTime += deltaTime;

    // if time > 500 ms...
    if (sumTime > 0.5f)
    {
        // ... compute averaged updating time for last 0.5 seconds
        sysState.updateTimeAvg = sumUpdateTime / numFramesHalfSec;
        sumUpdateTime    = 0;
        numFramesHalfSec = 0;
        sumTime          = 0;
    }
}

///////////////////////////////////////////////////////////

void App::Close()
{
    eventHandler_.DetachAllEventListeners();
    wndContainer_.renderWindow_.UnregisterWindowClass(hInstance_);
}

}; // namespace Game
