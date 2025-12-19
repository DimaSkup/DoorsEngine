// =================================================================================
// Filename:      App.cpp
// 
// Created:       21.04.25  by DimaSkup
// =================================================================================
#include "pch.h"
#include "Application.h"
#include <QuadTree/quad_tree.h>
#include <QuadTree/scene_object.h>
#include <geometry/rect_3d.h>



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
// Desc:   do all the init stuff for the engine:
//         init window, init engine's modules, load scene elements, etc.
//---------------------------------------------------------
void App::Init()
{
    // compute duration of importing process
    auto initStartTime = std::chrono::steady_clock::now();

    // explicitly init Windows Runtime and COM
    //HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't explicitly initialize Windows Runtime and COM");
        exit(-1);
    }

    eventHandler_.AddEventListener(&engine_);        // set engine class as one of the window events listeners
    wndContainer_.SetEventHandler(&eventHandler_);   // set an event handler for the window container

   

    engine_.BindRender(&render_);
    engine_.BindECS(&entityMgr_);
    engine_.BindUI(&userInterface_);

    InitWindow();
    InitRender(engineConfigs_);
    InitEngine();

    Core::CGraphics&     graphics = engine_.GetGraphicsClass();
    Render::D3DClass&    d3d      = render_.GetD3D();
    ID3D11Device*        pDevice  = d3d.GetDevice();
    ID3D11DeviceContext* pContext = d3d.GetDeviceContext();

    // after initialization of entine's modules we init the game related stuff
    // (model entities, light source, particle emitters, etc.)
    game_.Init(&engine_, &entityMgr_, &render_, engineConfigs_);

    Core::TerrainGeomip& terrain = Core::g_ModelMgr.GetTerrainGeomip();

    // create a facade btw the UI and the engine parts
    pFacadeEngineToUI_ = new UI::FacadeEngineToUI(
        pContext,
        &render_,
        &entityMgr_,
        &graphics,
        &terrain);

    // initialize the main UserInterface class
    InitGUI(pDevice, d3d.GetWindowWidth(), d3d.GetWindowHeight());
 
    auto initEndTime = std::chrono::steady_clock::now();
    std::chrono::duration<float, std::milli> initDuration = initEndTime - initStartTime;

    // create a str with duration time about the engine initialization process
    const POINT drawAt = { 10, 800 };
    char initTimeBuf[32]{'\0'};
    snprintf(initTimeBuf, 32, "Init time: %d ms", (int)initDuration.count());
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
void App::InitEngine()
{
    const std::string wndTitle = engineConfigs_.GetString("WINDOW_TITLE");

    engine_.Init(hInstance_, mainHWND_, engineConfigs_, wndTitle);
}

//---------------------------------------------------------
// Desc:   prepare params for initialization of the "Render" module and init it
//---------------------------------------------------------
bool App::InitRender(const Core::EngineConfigs& cfgs)
{
    // prepare WVO (world * base_view * ortho) matrix for 2D rendering
    const bool startInGameMode  = cfgs.GetBool("START_IN_GAME_MODE");
    const char* cameraEnttName  = (startInGameMode) ? "game_camera" : "editor_camera";
    const EntityID cameraId     = entityMgr_.nameSystem_.GetIdByName(cameraEnttName);
 
    const DirectX::XMMATRIX& baseView = entityMgr_.cameraSystem_.GetBaseView(cameraId);
    const DirectX::XMMATRIX& ortho    = entityMgr_.cameraSystem_.GetOrtho(cameraId);
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
    bool result = render_.Init(mainHWND_, renderParams);
    if (!result)
    {
        LogErr(LOG, "can't init the render module");
        return false;
    }

    // do some setup after initialization
    render_.SetSkyGradient(skyColorCenter, skyColorApex);
    render_.SetGrassDistFullSize(cfgs.GetFloat("GRASS_DIST_FULL_SIZE"));
    render_.SetGrassDistVisible (cfgs.GetFloat("GRASS_DIST_VISIBLE"));

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
        char dbgFontDataFilepath[64]{'\0'};
        char dbgFontTexFilepath[64]{'\0'};
        char gameFontDataFilepath[64]{'\0'};
        char gameFontTexFilepath[64]{'\0'};

        strcat(dbgFontDataFilepath, g_RelPathUIDataDir);
        strcat(dbgFontDataFilepath, engineConfigs_.GetString("DBG_FONT_DATA_PATH"));

        strcat(dbgFontTexFilepath, g_RelPathUIDataDir);
        strcat(dbgFontTexFilepath, engineConfigs_.GetString("DBG_FONT_TEX_PATH"));

        strcat(gameFontDataFilepath, g_RelPathUIDataDir);
        strcat(gameFontDataFilepath, engineConfigs_.GetString("GAME_FONT_DATA_PATH"));

        strcat(gameFontTexFilepath, g_RelPathUIDataDir);
        strcat(gameFontTexFilepath, engineConfigs_.GetString("GAME_FONT_TEX_PATH"));


        char videoCardName[128]{ '\0' };
        int videoCardMemory = 0;
        render_.GetD3D().GetVideoCardInfo(videoCardName, 128, videoCardMemory);

        // initialize the user interface
        userInterface_.Initialize(
            pDevice,
            pFacadeEngineToUI_,
            dbgFontDataFilepath,
            dbgFontTexFilepath,
            gameFontDataFilepath,
            gameFontTexFilepath,
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
    while (wndContainer_.renderWindow_.ProcessMessages(hInstance_, mainHWND_) == true)
    {
        if (!engine_.IsPaused())
        {
            const float deltaTime = engine_.GetTimer().GetDeltaTime();
            const float gameTime  = engine_.GetTimer().GetGameTime();

            game_.Update(deltaTime, gameTime);
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
}

}; // namespace Game
