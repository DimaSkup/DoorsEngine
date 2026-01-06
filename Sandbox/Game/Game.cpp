//==================================================================================
// Filename:   Game.cpp
// Created:    25.07.2025  by DimaSkup
//==================================================================================
#include "../Common/pch.h"
#include "Game.h"
#include <types.h>

#include <Engine/Engine.h>
#include <Model/model_mgr.h>
#include <Render/debug_draw_manager.h>
#include <Model/animation_mgr.h>
#include <Input/keyboard.h>
#include <Sound/sound_mgr.h>
#include <Sound/sound.h>

#include "../Initializers/GameInitializer.h"
#include "../Initializers/weapons_initializer.h"

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
    {
        LogErr(LOG, "a ptr to engine == nullptr");
        exit(0);
    }
    if (!pEnttMgr)
    {
        LogErr(LOG, "a ptr to entity manager == nullptr");
        exit(0);
    }
    if (!pRender)
    {
        LogErr(LOG, "a ptr to render == nullptr");
        exit(0);
    }

    pEngine_  = pEngine;
    pEnttMgr_ = pEnttMgr;
    pRender_  = pRender;
    g_AnimationMgr.Init();


    LogMsg(LOG, "game scene initialization: start");

    const TimePoint initStartTime = pEngine->GetTimer().GetTimePoint();

    GameInitializer gameInit;
    Core::CGraphics& graphics   = pEngine->GetGraphicsClass();
    ECS::NameSystem& nameSys    = pEnttMgr->nameSystem_;
    const Render::D3DClass& d3d = pRender->GetD3D();
    bool result = false;


    // create and init scene elements
    if (!gameInit.InitModelEntities(*pEnttMgr, *pRender, &configs))
    {
        LogErr(LOG, "can't initialize models");
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


    // set the current camera
    if (configs.GetBool("START_IN_GAME_MODE"))
    {
        const EntityID cameraId = nameSys.GetIdByName("game_camera");
        graphics.SetCurrentCamera(cameraId);
    }
    else
    {
        const EntityID cameraId = nameSys.GetIdByName("editor_camera");
        graphics.SetCurrentCamera(cameraId);
    }
    

    //-----------------------------------------------------

    const char* particlesFilepath = "data/particles/particles.cfg";
    const char* lightsFilepath    = "data/light.dentt";

    gameInit.InitParticles(particlesFilepath, *pEnttMgr);
    gameInit.InitLightEntities(lightsFilepath, *pEnttMgr);
    gameInit.InitPlayer(d3d.GetDevice(), pEnttMgr, &configs);


    // get id of rain entity which is always over the player
    rainEnttId_ = nameSys.GetIdByName("rain_over_player");
    assert(rainEnttId_ != INVALID_ENTITY_ID);

    InitWeapons();
    InitSoundsStuff();

    StartFootstepSequence(100);   // prevent lagging when we move player for the first time


    const TimePoint      initEndTime  = pEngine->GetTimer().GetTimePoint();
    const TimeDurationMs initDuration = initEndTime - initStartTime;

    LogMsg(LOG, "game scene is initialized");
    SetConsoleColor(MAGENTA);
    LogMsg("-------------------------------");
    LogMsg("game scene init took:  %.2f sec", initDuration.count() * 0.001f);
    LogMsg("-------------------------------\n\n");
    SetConsoleColor(RESET);

    return true;
}


//---------------------------------------------------------

void Game::InitWeapons()
{
    printf("\n");
    LogMsg(LOG, "initialize weapons:");

    const char* weaponCfgFile = "data/weapon.cfg";

    WeaponsInitializer initializer;
    initializer.Init(weaponCfgFile, pEnttMgr_, weapons_);

    //
    // setup initial weapon
    //
    const Weapon& currWeapon = weapons_[0];
    currWeaponEnttId_ = currWeapon.enttId;
    assert(currWeaponEnttId_ != INVALID_ENTITY_ID);

    pSkeleton_  = currWeapon.pSkeleton;
    currAnimId_ = currWeapon.animIds[WPN_ANIM_TYPE_IDLE];

    pEnttMgr_->playerSystem_.SetActiveWeapon(currWeaponEnttId_);
}

//---------------------------------------------------------
// Desc:    update the game
// Args:    - dt:   the time passed since the previous frame
//---------------------------------------------------------
bool Game::Update(const float dt, const float gameTime)
{
    // generate particles
    pEnttMgr_->particleSystem_.CreateParticles(dt);

    deltaTime_ = dt;
    gameTime_ = gameTime;

    UpdateRainbowAnomaly();

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
        thunderSoundIdx %= 4;
    }

    // update timings
    if (pEngine_->IsGameMode())
    {
        currActTime_ += dt;

        if (currActTime_ >= endActTime_)
        {
            ECS::PlayerSystem& player = pEnttMgr_->playerSystem_;

            currActTime_ = 0;

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

            currAnimId_ = weapons_[currWeaponIdx_].animIds[WPN_ANIM_TYPE_IDLE];
            StartAnimWeaponIdle();
        }

        //printf("curr: %.3f    end: %.3f    dt: %.3f\n", currActTime_, endActTime_, dt);

        gameEventsList_.Reset();

        UpdateFootstepsSound(dt);
        UpdateShootSound(dt);

        HandleGameEventKeyboard();
        HandleGameEventMouse(dt);

        HandleGameEvents();

        UpdateRainPos();

        if (!rainSoundIsPlaying_)
        {
            g_SoundMgr.GetSound(rainSoundId_)->PlayTrack(DSBPLAY_LOOPING);
            rainSoundIsPlaying_ = true;
        }
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
            case VK_ESCAPE:
            {
                // if we pressed the ESC button we exit from the application
                LogDbg(LOG, "Esc is pressed");
                pEngine_->DoExit();
                break;
            }
            case KEY_N:
            {
                if (!keyboard.WasPressedBefore(KEY_N))
                {
                    pEngine_->GetGraphicsClass().IncreaseCurrAnimIdx();
                }
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
                    ECS::PlayerSystem& player = pEnttMgr_->playerSystem_;
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
                pEnttMgr_->AddEvent(ECS::EventPlayerRun(false));
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

    ECS::PlayerSystem& player = pEnttMgr_->playerSystem_;
    Keyboard& keyboard = pEngine_->GetKeyboard();
    
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
            if (pEngine_->GetKeyboard().WasPressedBefore(code))
                break;

            // weapon index
            const int weaponIdx = code - KEY_1;

            gameEventsList_.PushEvent(GameEvent(PLAYER_SWITCH_WEAPON, (float)weaponIdx));
            break;
        }

        case KEY_SHIFT:
        {
            pEnttMgr_->AddEvent(EventPlayerRun(true));
            break;
        }
        case KEY_A:
        {
            pEnttMgr_->AddEvent(EventPlayerMove(EVENT_PLAYER_MOVE_LEFT));
            StartFootstepSequence(0);
            UpdateRainPos();
            break;
        }
        case KEY_D:
        {
            pEnttMgr_->AddEvent(EventPlayerMove(EVENT_PLAYER_MOVE_RIGHT));
            StartFootstepSequence(0);
            UpdateRainPos();
            break;
        }
        case KEY_L:
        {
            // switch the flashlight
            if (!pEngine_->GetKeyboard().WasPressedBefore(KEY_L))
                gameEventsList_.PushEvent(GameEvent(PLAYER_SWITCH_FLASHLIGHT));
            break;
        }
        case KEY_S:
        {
            pEnttMgr_->AddEvent(EventPlayerMove(EVENT_PLAYER_MOVE_BACK));
            StartFootstepSequence(deltaTime_);
            UpdateRainPos();
            break;
        }
        case KEY_R:
        {
            gameEventsList_.PushEvent(GameEvent(PLAYER_RELOAD_WEAPON));
            break;
        }
        case KEY_W:
        {
            pEnttMgr_->AddEvent(EventPlayerMove(EVENT_PLAYER_MOVE_FORWARD));

            StartFootstepSequence(deltaTime_);
            UpdateRainPos();
            player.SetIsMoving(true);

            // don't switch animation to "walking/running" if we already shooting or reloading
            if (player.IsReloading() || player.IsShooting())
                break;

            if (player.IsRunning())
            {
                const Weapon&        wpn    = GetCurrentWeapon();
                const AnimationID    animId = wpn.animIds[WPN_ANIM_TYPE_RUN];
                const AnimationClip& anim   = pSkeleton_->GetAnimation(animId);

                endActTime_       = anim.GetEndTime();
                
            }
            else
            {
                player.SetIsWalking();
            }

            break;
        }
        case KEY_Z:
        {
            if (pEnttMgr_->playerSystem_.IsFreeFlyMode())
                pEnttMgr_->AddEvent(EventPlayerMove(EVENT_PLAYER_MOVE_DOWN));
            break;
        }
        case KEY_SPACE:
        {
            if (pEnttMgr_->playerSystem_.IsFreeFlyMode())
                pEnttMgr_->AddEvent(EventPlayerMove(EVENT_PLAYER_MOVE_UP));
            else
                pEnttMgr_->AddEvent(EventPlayerMove(EVENT_PLAYER_JUMP));

            break;
        }
    } // switch
   
    UpdateMovementRelatedStuff();
}

//---------------------------------------------------------
// Desc:   hangle mouse input events
//---------------------------------------------------------
void Game::HandleGameEventMouse(const float deltaTime)
{
    Mouse& mouse = pEngine_->GetMouse();
    ECS::PlayerSystem& player = pEnttMgr_->playerSystem_;
    Weapon& wpn = weapons_[currWeaponIdx_];

  

  
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
                const float rotY  = mouseEvent.GetPosX() * deltaTime;
                const float pitch = mouseEvent.GetPosY() * deltaTime;

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
        gameEventsList_.PushEvent(GameEvent(PLAYER_SINGLE_SHOT));
    }
    else if (current)
    {
        // execute multiple shots (possible only for machine guns)
        gameEventsList_.PushEvent(GameEvent(PLAYER_MULTIPLE_SHOTS));

        int mousePosX = mouse.GetPosX();
        int mousePosY = mouse.GetPosY();
        pEngine_->GetGraphicsClass().GetRayIntersectionPoint(mousePosX, mousePosY);
        //pEngine_->GetGraphicsClass().TestEnttSelection(mousePosX, mousePosY);
    }

    last = current;
}

//---------------------------------------------------------
// Desc:  call it each time when player's position is changed
//---------------------------------------------------------
void Game::UpdateMovementRelatedStuff()
{
    UpdateRainPos();
}

//---------------------------------------------------------
// Desc:  call it when player's position is changed:
//        start playing footsteps sound
//---------------------------------------------------------
void Game::StartFootstepSequence(const float dt)
{
    if (!soundStepL_Playing)
    {
        stepTimer_ += dt;

        if (stepTimer_ <= currStepInterval_)
            return;

        g_SoundMgr.GetSound(actorStepL_)->PlayTrack();

        soundStepL_Playing = true;
        soundStepR_Played  = false;
        stepTimer_         = 0;
    }
}

//---------------------------------------------------------
// Desc:  call it each frame
//        update player's footsteps to step right goes properly after step left
//---------------------------------------------------------
void Game::UpdateFootstepsSound(const float dt)
{
    // which stepL sound is playing...
    if (soundStepL_Playing && !soundStepR_Played)
    {
        stepTimer_ += dt;
        
        if (stepTimer_ <= currStepInterval_)
            return;

        // check if stepL finished playing
        if (WaitForSingleObject(eventStepLDone, 0) == WAIT_OBJECT_0)
        {
            // play second sound
            g_SoundMgr.GetSound(actorStepR_)->PlayTrack();
            soundStepR_Played  = true;
            soundStepL_Playing = false;
            stepTimer_         = 0;
        }
    }
}

//---------------------------------------------------------
// Desc:  start playing shooting sound after we pressed LMB
//---------------------------------------------------------
void Game::StartPlayShootSound()
{
    const Weapon&    wpn = GetCurrentWeapon();
    const bool ableShoot = wpn.type == WPN_TYPE_PISTOL;

    if (!soundShootIsPlaying_ || ableShoot)
    {
        g_SoundMgr.GetSound(wpn.soundIds[WPN_SOUND_TYPE_SHOOT])->PlayTrack();
        soundShootIsPlaying_ = true;
    }
}

//---------------------------------------------------------
// Desc:  we will be able to restart shooting sound only after
//        a particular interval since the start
//---------------------------------------------------------
void Game::UpdateShootSound(const float dt)
{
    shootTimer_ += dt;

    if (soundShootIsPlaying_)
    {
        if (shootTimer_ < shootInterval_)
            return;

        soundShootIsPlaying_ = false;
        shootTimer_ = 0;
    }
}

//---------------------------------------------------------
// Desc:  update the rainbow anomaly (place point light near each rainbow particle)
//---------------------------------------------------------
void Game::UpdateRainbowAnomaly()
{
    ECS::EntityMgr& mgr = *pEnttMgr_;

    ECS::NameSystem&      nameSys      = mgr.nameSystem_;
    ECS::ParticleSystem&  particleSys  = mgr.particleSystem_;
    ECS::LightSystem&     lightSys     = mgr.lightSystem_;

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
    EntityID          pointLightIds[numPointL]{ INVALID_ENTITY_ID };

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
    mgr.transformSystem_.SetPositions(pointLightIds, numRainbows, newPositions);
}

//---------------------------------------------------------
// Desc:  update position of the rain so it is always over the player
//---------------------------------------------------------
void Game::UpdateRainPos()
{
    const DirectX::XMFLOAT3 p = pEnttMgr_->playerSystem_.GetPosition();
    pEnttMgr_->AddEvent(ECS::EventTranslate(rainEnttId_, p.x, p.y, p.z));
}

//---------------------------------------------------------
// Desc:  init sounds related stuff
//---------------------------------------------------------
void Game::InitSoundsStuff()
{
    //
    // gather sounds ids (so we don't need to get it by names)
    //

    // get ids of thunder sounds
    thunderSoundIds_[0] = g_SoundMgr.GetSoundIdByName("thunder_0");
    thunderSoundIds_[1] = g_SoundMgr.GetSoundIdByName("thunder_1");
    thunderSoundIds_[2] = g_SoundMgr.GetSoundIdByName("thunder_2");
    thunderSoundIds_[3] = g_SoundMgr.GetSoundIdByName("thunder_3");

    // get id of the rain sound
    rainSoundId_ = g_SoundMgr.GetSoundIdByName("ambient_rain");

    // get ids of actor's steps
    actorStepL_ = g_SoundMgr.GetSoundIdByName("actor_stepL");
    actorStepR_ = g_SoundMgr.GetSoundIdByName("actor_stepR");

    //
    // create event handler for some sounds so we will be able to know when
    // sound is over, or it is currently playing
    //
    Core::Sound* pSound = g_SoundMgr.GetSound(actorStepL_);
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
    for (int i = 0; i < gameEventsList_.numEvents; ++i)
    {
        const GameEvent& e = gameEventsList_.events[i];

        switch (e.type)
        {
            case PLAYER_SWITCH_WEAPON:
            {
                const int weaponIdx = (int)e.x;

                if (weaponIdx >= weapons_.size())
                    break;

                if (currWeaponIdx_ == weaponIdx)
                    break;

                HandleEventWeaponSwitch(weaponIdx);
                break;
            }

            case PLAYER_RELOAD_WEAPON:
                HandleEventWeaponReload();
                break;

            case PLAYER_SINGLE_SHOT:
                HandleEventWeaponSingleShot();
                break;

            case PLAYER_MULTIPLE_SHOTS:
                HandleEventWeaponMultipleShots();
                break;

            case PLAYER_RUN:
                break;

            case PLAYER_SWITCH_FLASHLIGHT:
                SwitchFlashLight(*pEnttMgr_, *pRender_);
                break;
        }
    }
}

//---------------------------------------------------------
// Desc:  player decided to switch its weapon so handle this event:
//        1. bind another weapon as a current
//        2. play weapon's "switching/drawing" sound
//        3. play weapon's "switching/drawing" animation
//---------------------------------------------------------
void Game::HandleEventWeaponSwitch(const int newWeaponIdx)
{
     // stop all sounds for the current weapon
    for (int i = 0; i < (int)NUM_WPN_SOUND_TYPES; ++i)
    {
        const SoundID id = weapons_[currWeaponIdx_].soundIds[i];
        g_SoundMgr.GetSound(id)->StopTrack();
    }

    const Weapon& wpn             = weapons_[newWeaponIdx];

    currWeaponEnttId_             = wpn.enttId;
    pSkeleton_                    = wpn.pSkeleton;

    const AnimationID animIdDraw  = wpn.animIds[WPN_ANIM_TYPE_DRAW];
    const AnimationID animIdShoot = wpn.animIds[WPN_ANIM_TYPE_SHOOT];

    const float animDrawEndTime   = pSkeleton_->GetAnimation(animIdDraw).GetEndTime();
    const float animShootEndTime  = pSkeleton_->GetAnimation(animIdShoot).GetEndTime();

    shootInterval_                = animShootEndTime / 2.5f;
    currAnimId_                   = animIdDraw;
    currWeaponIdx_                = newWeaponIdx;
    currActTime_                  = 0;
    endActTime_                   = animDrawEndTime;

    pEnttMgr_->playerSystem_.SetActiveWeapon(currWeaponEnttId_);
    pEnttMgr_->playerSystem_.SetIsDrawWeapon(true);

    g_SoundMgr.GetSound(wpn.soundIds[WPN_SOUND_TYPE_DRAW])->PlayTrack();
    StartAnimWeaponDraw();
}

//---------------------------------------------------------
// Desc:  reload our current weapon
//---------------------------------------------------------
void Game::HandleEventWeaponReload()
{
    Weapon& wpn = GetCurrentWeapon();
    const AnimationID animIdReload = wpn.animIds[WPN_ANIM_TYPE_RELOAD];
    const AnimationID animIdDraw   = wpn.animIds[WPN_ANIM_TYPE_DRAW];

    if (currAnimId_ == animIdReload || currAnimId_ == animIdDraw)
        return;

    const AnimationClip& anim = pSkeleton_->GetAnimation(animIdReload);
    pEnttMgr_->playerSystem_.SetIsReloading(true);

    // start animation immediately
    currActTime_ = anim.GetEndTime();
    endActTime_ = anim.GetEndTime();

    g_SoundMgr.GetSound(wpn.soundIds[WPN_SOUND_TYPE_RELOAD])->PlayTrack();
    StartAnimWeaponReload();
}

//---------------------------------------------------------
// Desc:  execute a single shot (act of attack) by the player
//---------------------------------------------------------
void Game::HandleEventWeaponSingleShot()
{
    const Weapon& wpn = GetCurrentWeapon();
    const AnimationID animIdReload = wpn.animIds[WPN_ANIM_TYPE_RELOAD];
    const AnimationID animIdDraw   = wpn.animIds[WPN_ANIM_TYPE_DRAW];

    // aren't able to shoot while reloading or drawing (appearing) weapon
    if (currAnimId_ == animIdReload || currAnimId_ == animIdDraw)
        return;

    StartPlayShootSound();
    StartAnimWeaponShoot();
}

//---------------------------------------------------------
// Desc:  handle the case when LMB is down for several frames
//        so we do multiple shots (if our weapon is able to do so)
//---------------------------------------------------------
void Game::HandleEventWeaponMultipleShots()
{
    const Weapon& wpn = weapons_[currWeaponIdx_];
   
    // if current weapon can't do multiple shots...
    if (wpn.type == WPN_TYPE_PISTOL || wpn.type == WPN_TYPE_SHOTGUN)
        return;

    // we aren't able to shoot while reloading or drawing (appearing) weapon
    const AnimationID animIdReload = wpn.animIds[WPN_ANIM_TYPE_RELOAD];
    const AnimationID animIdDraw   = wpn.animIds[WPN_ANIM_TYPE_DRAW];

    if (currAnimId_ == animIdReload || currAnimId_ == animIdDraw)
        return;

    // if not each time spent since the previous shot...
    if (currActTime_ < shootInterval_)
        return;

    // restart shooting sound, animation, etc.
    StartPlayShootSound();
    StartAnimWeaponShoot();
}

//---------------------------------------------------------
// Desc:   switch on/off the player's flashlight
//---------------------------------------------------------
void Game::SwitchFlashLight(ECS::EntityMgr& mgr, Render::CRender& render)
{
    ECS::PlayerSystem& player = mgr.playerSystem_;
    const bool isActive = !player.IsFlashLightActive();;
    player.SwitchFlashLight(isActive);

    // update the state of the flashlight entity
    const EntityID flashlightId = mgr.nameSystem_.GetIdByName("player_flashlight");

    mgr.lightSystem_.SetLightIsActive(flashlightId, isActive);
    render.SwitchFlashLight(isActive);

    // if we just turned on the flashlight we update its position and direction
    if (isActive)
    {
        mgr.transformSystem_.SetPosition(flashlightId, player.GetPosition());
        mgr.transformSystem_.SetDirection(flashlightId, player.GetDirVec());
    }
}


//**********************************************************************************
//                        PLAYER ANIMATIONS SWITCHERS
//**********************************************************************************

//---------------------------------------------------------
// Desc:  play weapon "appearing" animation
//---------------------------------------------------------
void Game::StartAnimWeaponDraw()
{
    const Weapon&        wpn = weapons_[currWeaponIdx_];
    const AnimationID animId = wpn.animIds[WPN_ANIM_TYPE_DRAW];
    const float  animEndTime = pSkeleton_->GetAnimation(animId).GetEndTime();

    currActTime_            = 0;
    currAnimId_             = animId;
    endActTime_             = animEndTime;

    pEnttMgr_->animationSystem_.SetAnimation(
        currWeaponEnttId_,
        animId,
        animEndTime,
        ECS::ANIM_PLAY_ONCE);

    pEnttMgr_->animationSystem_.RestartAnimation(currWeaponEnttId_);
}

//---------------------------------------------------------
// switch to "reloading" animation
//---------------------------------------------------------
void Game::StartAnimWeaponReload()
{
    const Weapon&        wpn = weapons_[currWeaponIdx_];
    const AnimationID animId = wpn.animIds[WPN_ANIM_TYPE_RELOAD];
    const float  animEndTime = pSkeleton_->GetAnimation(animId).GetEndTime();

    currActTime_ = 0;
    currAnimId_ = animId;

    pEnttMgr_->animationSystem_.SetAnimation(
        currWeaponEnttId_,
        animId,
        animEndTime,
        ECS::ANIM_PLAY_ONCE);
}

//---------------------------------------------------------
// switch to "shooting" animation
//---------------------------------------------------------
void Game::StartAnimWeaponShoot()
{
    const Weapon&        wpn = weapons_[currWeaponIdx_];
    const AnimationID animId = wpn.animIds[WPN_ANIM_TYPE_SHOOT];
    const float  animEndTime = pSkeleton_->GetAnimation(animId).GetEndTime();

    currActTime_ = 0;
    endActTime_ = animEndTime;
    currAnimId_ = animId;

    pEnttMgr_->animationSystem_.SetAnimation(
        currWeaponEnttId_,
        animId,
        animEndTime,
        ECS::ANIM_PLAY_ONCE);

    pEnttMgr_->animationSystem_.RestartAnimation(currWeaponEnttId_);
}

//---------------------------------------------------------
// switch to "run/sprint" animation
//---------------------------------------------------------
void Game::StartAnimWeaponRun()
{
    const Weapon&        wpn = weapons_[currWeaponIdx_];
    const AnimationID animId = wpn.animIds[WPN_ANIM_TYPE_RUN];
    const float  animEndTime = pSkeleton_->GetAnimation(animId).GetEndTime();

    currActTime_ = 0;
    endActTime_ = animEndTime;
    currAnimId_ = animId;

    pEnttMgr_->animationSystem_.SetAnimation(
        currWeaponEnttId_,
        animId,
        animEndTime,
        ECS::ANIM_PLAY_ONCE);
}

//---------------------------------------------------------
// switch to "idle" animation
//---------------------------------------------------------
void Game::StartAnimWeaponIdle()
{
    const Weapon&        wpn = weapons_[currWeaponIdx_];
    const AnimationID animId = wpn.animIds[WPN_ANIM_TYPE_IDLE];
    const float  animEndTime = pSkeleton_->GetAnimation(animId).GetEndTime();

    currAnimId_ = animId;

    pEnttMgr_->animationSystem_.SetAnimation(
        currWeaponEnttId_,
        animId,
        animEndTime,
        ECS::ANIM_PLAY_LOOP);
}

} // namespace
