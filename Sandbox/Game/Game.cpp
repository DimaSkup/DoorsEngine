//==================================================================================
// Filename:   Game.cpp
// Created:    25.07.2025  by DimaSkup
//==================================================================================
#include "../Common/pch.h"
#include "Game.h"
#include <types.h>

#include <Engine/Engine.h>
#include <Model/model_mgr.h>
#include <Model/grass_mgr.h>
#include <Render/debug_draw_manager.h>
#include <Model/animation_mgr.h>
#include <Input/keyboard.h>
#include <Sound/sound_mgr.h>
#include <Sound/sound.h>
#include <Terrain/terrain.h>

#include <Model/model_importer.h>
#include <Model/ufbx.h>

#include "../Initializers/game_initializer.h"
#include "../Initializers/weapons_initializer.h"

#include "event_handlers.h"

using namespace Core;


namespace Game
{

Game::~Game()
{
}

//---------------------------------------------------------
// Desc:    init all the game related stuff
//---------------------------------------------------------
bool Game::Init(
    Core::Engine* pEngine,
    ECS::EntityMgr* pEnttMgr,
    Render::CRender* pRender,
    const Core::EngineConfigs& configs)
{
    if (!pEngine)
        LogFatal(LOG, "a ptr to engine == NULL");

    if (!pEnttMgr)
        LogFatal(LOG, "a ptr to entity manager == NULL");

    if (!pRender)
        LogFatal(LOG, "a ptr to render == NULL");


#if 0
    //ModelImporter importer;
    //Model kordon;
    //importer.LoadFromFile(&kordon, "data/models/ext/Kordon model/escape.fbx");


    const char* pathToModel = "data/models/ext/Kordon model/escape.fbx";

    ufbx_load_opts opts = {};
    opts.target_axes = ufbx_axes_left_handed_y_up;
    opts.target_unit_meters = 1.0f;

    ufbx_error error;


    TimePoint startImport = GameTimer::GetTimePoint();

    ufbx_scene* pScene = ufbx_load_file(pathToModel, &opts, &error);
    if (!pScene)
    {
        printf("Failed to load scene: %s\n", error.description.data);
        return false;
    }

    TimePoint endImport = GameTimer::GetTimePoint();
    TimeDurationMs dur = endImport - startImport;
    printf("\n");
    printf("dur of import: %.2f sec\n", dur.count() / 1000.0f);
    printf("\n");

    // print out name of each node
    for (int i = 0; ufbx_node* node : pScene->nodes)
    {
        printf("[%d]:   %s\n", i++, node->name.data);

        node->mesh->instances

        if (i == 10)
        {
            ufbx_free_scene(pScene);
            exit(0);
        }
    }

    ufbx_free_scene(pScene);
    exit(0);
#endif

    pEngine_  = pEngine;
    pEnttMgr_ = pEnttMgr;
    pRender_  = pRender;
    g_AnimationMgr.Init();


    LogMsg(LOG, "game scene initialization: start");

    const TimePoint initStartTime = GetTimePoint();

    GameInitializer gameInit;
    Core::CGraphics& graphics   = pEngine->GetGraphics();
    ECS::NameSystem& nameSys    = pEnttMgr->nameSys_;
    const Render::D3DClass& d3d = pRender->GetD3D();

    GameInitPaths initPaths;
    gameInit.ReadGameInitPaths(configs.GetString("LOAD_LEVEL"), initPaths);

    // initialize some data/resource managers
    if (!g_TextureMgr.Init(initPaths.texturesFilepath))
    {
        LogFatal(LOG, "can't init a texture manager");
    }

    if (!g_MaterialMgr.Init())
    {
        LogFatal(LOG, "can't init a material manager");
    }

    if (!g_ModelMgr.Init())
    {
        LogFatal(LOG, "can't init a model manager");
    }

    if (!g_SoundMgr.Init(initPaths.soundsFilepath, pEngine_->GetHWND()))
    {
        LogFatal(LOG, "can't init a sound manager");
    }

    // create and init scene elements
    if (!gameInit.InitEntities(*pEnttMgr, *pRender, configs, initPaths))
    {
        LogErr(LOG, "can't initialize entities");
    }

    //-----------------------------------------------------

    // init all the cameras
    const float editorCamNearZ  = configs.GetFloat("EDITOR_CAM_NEAR_Z");
    const float editorCamFarZ   = configs.GetFloat("EDITOR_CAM_FAR_Z");
    const float gameCamNearZ    = configs.GetFloat("GAME_CAM_NEAR_Z");
    const float gameCamFarZ     = configs.GetFloat("GAME_CAM_FAR_Z");
    const float fovInRad        = configs.GetFloat("FOV_IN_RAD");         // field of view in radians

    CameraInitParams editorCamParams;
    editorCamParams.wndWidth    = (float)d3d.GetWindowedWndSize().cx;
    editorCamParams.wndHeight   = (float)d3d.GetWindowedWndSize().cy;
    editorCamParams.nearZ       = editorCamNearZ;
    editorCamParams.farZ        = editorCamFarZ;
    editorCamParams.fovInRad    = fovInRad;

    editorCamParams.posX = 0;
    editorCamParams.posY = 0;
    editorCamParams.posZ = 0;

    CameraInitParams gameCamParams;
    gameCamParams.wndWidth      = (float)d3d.GetFullscreenWndSize().cx;
    gameCamParams.wndHeight     = (float)d3d.GetFullscreenWndSize().cy;
    gameCamParams.nearZ         = gameCamNearZ;
    gameCamParams.farZ          = gameCamFarZ;
    gameCamParams.fovInRad      = fovInRad;

    if (!gameInit.InitCamera(*pEnttMgr, "editor_camera", editorCamParams))
    {
        LogErr(LOG, "can't init an editor camera");
    }

    if (!gameInit.InitCamera(*pEnttMgr, "game_camera", gameCamParams))
    {
        LogErr(LOG, "can't init a game camera");
    }

    
    //-----------------------------------------------------

 
    gameInit.InitParticles(initPaths.particlesFilepath, *pEnttMgr);
    gameInit.InitGrass    (initPaths.grassFilepath, *pEnttMgr);
    gameInit.InitLights   (initPaths.lightsFilepath, *pEnttMgr);
    gameInit.InitPlayer   (initPaths.playerFilepath, *pEnttMgr);

    // setup horizon and apex (top) color of the sky
    //const Core::SkyModel&   sky            = Core::g_ModelMgr.GetSky();
    const DirectX::XMFLOAT3 skyColorCenter = { 1,0,0 };// sky.GetColorCenter();
    const DirectX::XMFLOAT3 skyColorApex = { 0,1,0 };// sky.GetColorApex();

    pRender->SetSkyGradient(skyColorCenter, skyColorApex);

    // push grass distances into GPU
    pRender->SetGrassDistFullSize(Core::g_GrassMgr.GetGrassDistFullSize());
    pRender->SetGrassDistVisible(Core::g_GrassMgr.GetGrassVisibilityRange());



    // get id of rain entity which is always over the player
    rainEnttId_ = nameSys.GetIdByName("rain_over_player");
    assert(rainEnttId_ != INVALID_ENTT_ID);

    // init all the weapons
    LogMsg(LOG, "initialize weapons:");
    WeaponsInitializer initializer;
    initializer.Init(initPaths.weaponsFilepath, pEnttMgr_);
    
    InitSoundsStuff();

    // prevent lagging when we move player for the first time
    PlayerPlayFootstepSound(pEnttMgr->playerSys_.GetData(), 100);



    // set the current camera
    if (configs.GetBool("START_IN_GAME_MODE"))
    {
        const EntityID camId = nameSys.GetIdByName("game_camera");
        graphics.SetActiveCamera(camId);
    }
    else
    {
        const EntityID camId = nameSys.GetIdByName("editor_camera");
        graphics.SetActiveCamera(camId);
    }

    PlayerPlayAnimWeaponIdle(pEngine, nullptr);

    //
    // bind event handlers
    //
    eventMgr_.Subscribe("toggle_flashlight",    PlayerToggleFlashLight);

    eventMgr_.Subscribe("player_single_shot",   PlayerShot);
    eventMgr_.Subscribe("player_single_shot",   PlayerPlayShotSound);
    eventMgr_.Subscribe("player_single_shot",   PlayerPlayAnimWeaponShot);

    eventMgr_.Subscribe("player_switch_weapon", PlayerSwitchWeapon);
    eventMgr_.Subscribe("player_reload_weapon", PlayerReloadWeapon);
    eventMgr_.Subscribe("player_move",          PlayerMove);

    eventMgr_.Subscribe("player_shot_single",  PlayerShot);

    eventMgr_.Subscribe("player_shot_multiple", PlayerMultipleShots);

    eventMgr_.Subscribe("radiation_zone",       HandleRadiationZone);

    eventMgr_.Subscribe("radioactive_house",    HandleHouseRadiation);
    eventMgr_.Subscribe("fire_anomaly",         HandleFireAnomaly);

    const TimeDurationMs initDuration = GetTimePoint() - initStartTime;

    LogMsg(LOG, "game scene is initialized");
    SetConsoleColor(MAGENTA);
    LogMsg("-------------------------------");
    LogMsg("game scene init took:  %.2f sec", initDuration.count() * 0.001f);
    LogMsg("-------------------------------\n\n");
    SetConsoleColor(RESET);

    return true;
}

//---------------------------------------------------------
// Desc:    update the game
// Args:    - dt:   the time passed since the previous frame
//---------------------------------------------------------
bool Game::Update(const float dt, const float gameTime)
{
    // update timings
    deltaTime_ = dt;
    gameTime_ = gameTime;


    if (!pEngine_->IsGameMode())
        return true;

    ECS::PlayerSystem& player = pEnttMgr_->playerSys_;
    DirectX::XMFLOAT3 playerPos = pEnttMgr_->playerSys_.GetPosition();
    EventData eventData;
    eventData.fx = playerPos.x;
    eventData.fy = playerPos.y;
    eventData.fz = playerPos.z;

    UpdateRainbowAnomaly();
    eventMgr_.TriggerEvent("radioactive_house", pEngine_, &eventData);
    eventMgr_.TriggerEvent("fire_anomaly",      pEngine_, &eventData);

    static float thunderTimer = 0;
    static int thunderSoundIdx = 0;

    thunderTimer += dt;

    // play thunder sound each "thunderInterval" seconds
    if (thunderTimer > thunderInterval_)
    {
        const SoundID id = thunderSoundIds_[thunderSoundIdx];
        g_SoundMgr.GetSound(id)->PlayTrack();

        thunderTimer = 0;
        thunderSoundIdx++;
        thunderSoundIdx &= 3;   // idx % 4
    }

    float currActTime = player.GetCurrActTime();
    currActTime += dt;

    // if "act" is finished (we reloaded weapon, shoot, ect.) we switch to "idle" animation
    if (currActTime >= player.GetEndActTime())
    {
        player.SetCurrActTime(0);

        if (player.IsReloading())
            player.SetIsReloading(false);

        if (player.IsShooting())
        {
            player.SetIsShooting(false);
        }

        if (player.IsDrawWeapon())
        {
            player.SetIsDrawWeapon(false);
        }

        // switch to idle
        PlayerPlayAnimWeaponIdle(pEngine_, nullptr);
    }
    else
    {
        player.SetCurrActTime(currActTime);
    }

    //gameEventsList_.Reset();

    HandleGameEventKeyboard();
    HandleGameEventMouse(dt);
    HandleGameEvents();

    UpdatePlayerFootstepsSound(dt);
    UpdateShootSound(dt);
    UpdateRainPos();

    if (!rainSoundIsPlaying_)
    {
        g_SoundMgr.GetSound(rainSoundId_)->PlayTrack(DSBPLAY_LOOPING);
        rainSoundIsPlaying_ = true;
    }

    return true;
}

//---------------------------------------------------------
// Desc:   handle events from keyboard when we are int the game mode
//---------------------------------------------------------
void Game::HandleGameEventKeyboard()
{
    Keyboard& keyboard = pEngine_->GetKeyboard();

    // go through each currently pressed key and handle related events
    for (const eKeyCodes code : keyboard.GetPressedKeysList())
    {
        switch (code)
        {
            case KEY_TAB:
            {
                if (!keyboard.WasPressedBefore(KEY_TAB))
                    pEngine_->SwitchQuadTree();
                break;
            }

            case VK_ESCAPE:
            {
                // if we pressed the ESC button we exit from the application
                LogDbg(LOG, "Esc is pressed");
                pEngine_->DoExit();
                break;
            }
            case KEY_F1:
            {
                // switch from game to the editor mode
                if (!keyboard.WasPressedBefore(KEY_F1))
                {
                    pEngine_->SwitchEngineMode();
                    rainSoundIsPlaying_ = false;
                    g_SoundMgr.GetSound(rainSoundId_)->StopTrack();
                }
                break;
            }
            case KEY_F2:
            {
                // switch btw cameras modes (free / game)
                if (!keyboard.WasPressedBefore(KEY_F2))
                {
                    ECS::PlayerSystem& player = pEnttMgr_->playerSys_;
                    player.SetFreeFlyMode(!player.IsFreeFlyMode());
                }
                break;
            }
            case KEY_F3:
            {
                // show/hide debug info in the game mode
                if (!keyboard.WasPressedBefore(KEY_F3))
                {
                    SystemState& sysState = pEngine_->GetSystemState();
                    sysState.isShowDbgInfo = !sysState.isShowDbgInfo;
                }
                break;
            }
            case KEY_F4:
            {
                // turn on/off gathering of the GPU metrics
                if (!keyboard.WasPressedBefore(KEY_F4))
                {
                    SystemState& state = pEngine_->GetSystemState();
                    state.collectGpuMetrics = !state.collectGpuMetrics;
                    pEngine_->SwitchGpuMetricsCollection(state.collectGpuMetrics);
                }
                break;
            }
            case KEY_SHIFT:
            case KEY_1:
            case KEY_2:
            case KEY_3:
            case KEY_4:
            case KEY_5:
            case KEY_6:
            case KEY_A:
            case KEY_D:
            case KEY_L:
            case KEY_S:
            case KEY_R:
            case KEY_W:
            case KEY_Z:
            case KEY_SPACE:
            {
                HandlePlayerActions(code);
                break;
            }
        }
    }

    //-------------------------------------------

    // handle released keys
    while (int key = keyboard.ReadReleasedKey())
    {
        switch (key)
        {
            case KEY_SHIFT:
            {
                pEnttMgr_->PushEvent(ECS::EventPlayerRun(false));
                break;
            }
        }
    }
}

//---------------------------------------------------------
// Desc:   handle player related input when we are int the game mode
// Args:   - code:  input key code from keyboard
//---------------------------------------------------------
void Game::HandlePlayerActions(const eKeyCodes code)
{
    using namespace ECS;

    ECS::PlayerSystem& player = pEnttMgr_->playerSys_;
    Keyboard& keyboard = pEngine_->GetKeyboard();
    EventData eventData;

    eventData.deltaTime = deltaTime_;
    
    switch (code)
    {
        // switch weapon
        case KEY_1:
        case KEY_2:
        case KEY_3:
        case KEY_4:
        case KEY_5:
        case KEY_6:
        case KEY_7:
        case KEY_8:
        case KEY_9:
        {
            if (keyboard.WasPressedBefore(code))
                break;

            // define a weapon index (slot)
            eventData.ix = code - KEY_1;

            // exec switch
            eventMgr_.TriggerEvent("player_switch_weapon", pEngine_, &eventData);
            break;
        }

        case KEY_SHIFT:
        {
            pEnttMgr_->PushEvent(EventPlayerRun(true));
            break;
        }
        case KEY_A:
        {
            eventData.ix = EVENT_PLAYER_MOVE_LEFT;
            eventMgr_.TriggerEvent("player_move", pEngine_, &eventData);
            break;
        }
        case KEY_D:
        {
            eventData.ix = EVENT_PLAYER_MOVE_RIGHT;
            eventMgr_.TriggerEvent("player_move", pEngine_, &eventData);
            break;
        }
        case KEY_L:
        {
            // switch the flashlight
            if (!keyboard.WasPressedBefore(KEY_L))
                eventMgr_.TriggerEvent("toggle_flashlight", pEngine_, nullptr);
            break;
        }
        case KEY_S:
        {
            eventData.ix = EVENT_PLAYER_MOVE_BACKWARD;
            eventMgr_.TriggerEvent("player_move", pEngine_, &eventData);
            break;
        }
        case KEY_R:
        {
            if (!keyboard.WasPressedBefore(KEY_R))
                eventMgr_.TriggerEvent("player_reload_weapon", pEngine_, nullptr);
            break;
        }
        case KEY_W:
        {
            eventData.ix = EVENT_PLAYER_MOVE_FORWARD;
            eventMgr_.TriggerEvent("player_move", pEngine_, &eventData);
            break;
        }
        case KEY_Z:
        {
            if (pEnttMgr_->playerSys_.IsFreeFlyMode())
            {
                eventData.ix = EVENT_PLAYER_MOVE_DOWN;
                eventMgr_.TriggerEvent("player_move", pEngine_, &eventData);
            }
            break;
        }
        case KEY_SPACE:
        {
            if (pEnttMgr_->playerSys_.IsFreeFlyMode())
                eventData.ix = EVENT_PLAYER_MOVE_UP;
            else
                eventData.ix = EVENT_PLAYER_JUMP;

            eventMgr_.TriggerEvent("player_move", pEngine_, &eventData);
            break;
        }
    } // switch
}

//---------------------------------------------------------
// Desc:   hangle mouse input events
// Args:   dt - delta time
//---------------------------------------------------------
void Game::HandleGameEventMouse(const float dt)
{
    Mouse& mouse = pEngine_->GetMouse();
  
    while (!mouse.EventBufferIsEmpty())
    {
        MouseEvent mouseEvent = mouse.ReadEvent();

        switch (mouseEvent.GetEventType())
        {
            // update mouse position data because we need to print mouse position on the screen
            case MouseEvent::EventType::Move:
            {
                SystemState& sysState = pEngine_->GetSystemState();
                sysState.mouseX = mouseEvent.GetPosX();
                sysState.mouseY = mouseEvent.GetPosY();
                SetCursorPos(800, 450);                           // to prevent the cursor to get out of the window
                break;
            }

            // update the rotation data of the camera
            // with the current state of the input devices. The movement function will update
            // the position of the camera to the location for this frame
            case MouseEvent::EventType::RAW_MOVE:
            {
                ECS::PlayerSystem& player = pEnttMgr_->playerSys_;
                const float rotY  = mouseEvent.GetPosX() * dt;
                const float pitch = mouseEvent.GetPosY() * dt;

                player.RotateY(rotY);
                player.Pitch(pitch);

                break;
            }
            case MouseEvent::EventType::LPress:
            {
                break;
            }
            case MouseEvent::EventType::LRelease:
            {
                break;
            }
        }
    }

    // if LMB is currently pressed
    static bool last = false;
    bool current = GetAsyncKeyState(VK_LBUTTON) & 0x8000;
    
    if (current && !last)
    {
        // execute a single shot
        eventMgr_.TriggerEvent("player_shot_single", pEngine_, nullptr);
    }
    else if (current)
    {
        // execute multiple shots (possible only for machine guns)
        eventMgr_.TriggerEvent("player_shot_multiple", pEngine_, nullptr);
    }

    last = current;
}

//---------------------------------------------------------
// Desc:  call it each frame
//        update player's footsteps to step right goes properly after step left
//---------------------------------------------------------
void Game::UpdatePlayerFootstepsSound(const float dt)
{
    ECS::PlayerData& player = pEngine_->GetECS()->playerSys_.GetData();

    // which stepL sound is playing...
    if (player.soundStepL_Playing && !player.soundStepR_Played)
    {
        player.stepTimer += dt;
        
        if (player.stepTimer <= player.currStepInterval)
            return;

        // check if stepL finished playing
        if (WaitForSingleObject(eventStepLDone, 0) != WAIT_OBJECT_0)
            return;

        // play another step sound
        g_SoundMgr.GetSound(player.soundStepR)->PlayTrack();
        player.soundStepR_Played  = true;
        player.soundStepL_Playing = false;
        player.stepTimer          = 0;
    }
}

//---------------------------------------------------------
// Desc:  we will be able to restart shooting sound only after
//        a particular interval since the start
//---------------------------------------------------------
void Game::UpdateShootSound(const float dt)
{
    ECS::PlayerSystem&   player = pEngine_->GetECS()->playerSys_;
    ECS::PlayerData& playerData = player.GetData();

    playerData.shotTimer += dt;

    if (playerData.soundShotPlaying)
    {
        if (playerData.shotTimer < player.GetActiveWeapon().shotInterval)
            return;

        playerData.soundShotPlaying = false;
        playerData.shotTimer = 0;
    }
}

//---------------------------------------------------------
// Desc:  update the rainbow anomaly (place point light near each rainbow particle)
//---------------------------------------------------------
void Game::UpdateRainbowAnomaly()
{
    ECS::EntityMgr& mgr = *pEnttMgr_;

    ECS::NameSystem&      nameSys      = mgr.nameSys_;
    ECS::ParticleSystem&  particleSys  = mgr.particleSys_;
    ECS::LightSystem&     lightSys     = mgr.lightSys_;

    const EntityID anomaly0 = nameSys.GetIdByName("anomaly_rainbow_0");
    const EntityID anomaly1 = nameSys.GetIdByName("anomaly_rainbow_1");

    const cvector<ECS::Particle>& particles0 = particleSys.GetParticlesOfEmitter(anomaly0);
    const cvector<ECS::Particle>& particles1 = particleSys.GetParticlesOfEmitter(anomaly1);
    const size                   numRainbows = particles0.size() + particles1.size();

    // we have no rainbow particles to update
    if (numRainbows == 0)
        return;

    constexpr size numPointL = 8;
    DirectX::XMFLOAT3 newPositions [numPointL];
    EntityID          pointLightIds[numPointL]{ INVALID_ENTT_ID };

    // we have 8 point lights for rainbows...
    const char* pointLightsNames[numPointL] =
    {
        "pointL_rainbow_1",
        "pointL_rainbow_2",
        "pointL_rainbow_3",
        "pointL_rainbow_4",
        "pointL_rainbow_5",
        "pointL_rainbow_6",
        "pointL_rainbow_7",
        "pointL_rainbow_8",
    };

    // ... find its IDs by names
    nameSys.GetIdsByNames((const char**)pointLightsNames, numPointL, pointLightIds);

    // activate point light for alive rainbows
    for (index i = 0; i < numRainbows; ++i)
        lightSys.SetLightIsActive(pointLightIds[i], true);

    // deactivate all the rest point lights
    for (index i = numRainbows; i < numPointL; ++i)
        lightSys.SetLightIsActive(pointLightIds[i], false);


    // gather positions of rainbows
    for (index i = 0; i < particles0.size(); ++i)
        XMStoreFloat3(&newPositions[i], particles0[i].pos);

    for (index posIdx = particles0.size(), i = 0; posIdx < numRainbows; ++posIdx, ++i)
        XMStoreFloat3(&newPositions[posIdx], particles1[i].pos);

    // place point light in exact position of related rainbow particle
    mgr.transformSys_.SetPositions(pointLightIds, numRainbows, newPositions);
}

//---------------------------------------------------------
// Desc:  update position of the rain so it is always over the player
//---------------------------------------------------------
void Game::UpdateRainPos()
{
    const DirectX::XMFLOAT3 p = pEnttMgr_->playerSys_.GetPosition();
    pEnttMgr_->PushEvent(ECS::EventTranslate(rainEnttId_, p.x, p.y, p.z));
}

//---------------------------------------------------------
// Desc:  init sounds related stuff for the scene
//---------------------------------------------------------
void Game::InitSoundsStuff()
{
    //
    // gather sounds ids (so we don't need to get it by names each time)
    //

    // get ids of thunder sounds
    thunderSoundIds_[0] = g_SoundMgr.GetSoundIdByName("thunder_0");
    thunderSoundIds_[1] = g_SoundMgr.GetSoundIdByName("thunder_1");
    thunderSoundIds_[2] = g_SoundMgr.GetSoundIdByName("thunder_2");
    thunderSoundIds_[3] = g_SoundMgr.GetSoundIdByName("thunder_3");

    // get id of the rain sound
    rainSoundId_ = g_SoundMgr.GetSoundIdByName("ambient_rain");

    // get ids of actor's steps
    ECS::PlayerData& player = pEngine_->GetECS()->playerSys_.GetData();
    player.soundStepL = g_SoundMgr.GetSoundIdByName("actor_stepL");
    player.soundStepR = g_SoundMgr.GetSoundIdByName("actor_stepR");

    //
    // create event handler for some sounds so we will be able to know when
    // sound is over, or it is currently playing
    //
    Core::Sound* pSound = g_SoundMgr.GetSound(player.soundStepL);
    IDirectSoundBuffer8* pSoundBufStepL = pSound->GetBuffer();

    // create notification event
    eventStepLDone = CreateEvent(NULL, FALSE, FALSE, NULL);

    // attach notification to the END of buffer
    IDirectSoundNotify8* pNotifyStep = nullptr;
    pSoundBufStepL->QueryInterface(IID_IDirectSoundNotify8, (void**)&pNotifyStep);

    DSBPOSITIONNOTIFY notify = {};
    notify.dwOffset     = DSBPN_OFFSETSTOP; // end of buffer
    notify.hEventNotify = eventStepLDone;

    pNotifyStep->SetNotificationPositions(1, &notify);
    pNotifyStep->Release();
}


//**********************************************************************************
//                               EVENTS HANDLERS
//**********************************************************************************

//---------------------------------------------------------
// Desc:  handle each event pushed into the game events list
//---------------------------------------------------------
void Game::HandleGameEvents()
{

}

} // namespace
