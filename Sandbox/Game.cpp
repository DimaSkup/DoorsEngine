//==================================================================================
// Filename:   Game.cpp
// Created:    25.07.2025  by DimaSkup
//==================================================================================
#include "pch.h"
#include "Game.h"
#include "GameInitializer.h"
#include <Engine/Engine.h>
#include <Model/model_mgr.h>
#include <Render/debug_draw_manager.h>
#include <Model/animation_mgr.h>

using namespace Core;


namespace Game
{

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


    LogMsg(LOG, "scene initialization (start)");

    GameInitializer gameInit;
    Core::CGraphics& graphics   = pEngine->GetGraphicsClass();
    const Render::D3DClass& d3d = pRender->GetD3D();
    bool result = false;


    // create and init scene elements
    if (!gameInit.InitModelEntities(d3d.GetDevice(), *pEnttMgr, *pRender, &configs))
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
    const bool startInGameMode = configs.GetBool("START_IN_GAME_MODE");
    const char* cameraEnttName = (startInGameMode) ? "game_camera" : "editor_camera";
    const EntityID    cameraID = pEnttMgr->nameSystem_.GetIdByName(cameraEnttName);
    graphics.SetCurrentCamera(cameraID);

    //-----------------------------------------------------

    gameInit.InitParticles(*pEnttMgr);
    gameInit.InitLightEntities(*pEnttMgr, "data/light.dentt");
    gameInit.InitPlayer(d3d.GetDevice(), pEnttMgr, &configs);

    InitSounds(*pEnttMgr);
    StartFootstepSequence();   // prevent lagging when we move player for the first time


    // get id of rain entity which is always over the player
    rainEnttId_ = pEnttMgr_->nameSystem_.GetIdByName("rain_over_player");
    assert(rainEnttId_ != INVALID_ENTITY_ID);


    // get player animations ids
    currHudId_      = pEnttMgr_->nameSystem_.GetIdByName("ak_74_hud");
    currSkeletonId_ = Core::g_AnimationMgr.GetSkeletonId("ak_74_hud");

    assert(currHudId_      != INVALID_ENTITY_ID);
    assert(currSkeletonId_ != 0);

    const AnimSkeleton& skeleton = Core::g_AnimationMgr.GetSkeleton(currSkeletonId_);

    animIdReload_ = skeleton.GetAnimationIdx("wpn_ak74_hud_ogf_reload");
    animIdShoot_  = skeleton.GetAnimationIdx("wpn_ak74_hud_ogf_shoot");
    animIdRun_    = skeleton.GetAnimationIdx("wpn_ak74_hud_ogf_idle_sprint_");
    animIdIdle_   = skeleton.GetAnimationIdx("wpn_ak74_hud_ogf_idle");

    LogMsg(LOG, "is initialized");
    return true;
}

//---------------------------------------------------------
// Desc:    update the game
// Args:    - dt:   the time passed since the previous frame
//---------------------------------------------------------
bool Game::Update(const float dt, const float gameTime)
{
    // generate particles
    pEnttMgr_->particleSystem_.CreateParticles(dt);

    UpdateFootstepsSound(dt);
    UpdateShootSound(dt);
    UpdateRainbowAnomaly();

    //pEnttMgr_->playerSystem_.SetIsIdle();
    //pEnttMgr_->playerSystem_.ResetStates();

    // handle events
    if (pEngine_->IsGameMode())
    {
        HandleGameEventKeyboard();
        HandleGameEventMouse(dt);

        if (!rainSoundIsPlaying_)
        {
            soundRain_.PlayTrack(DSBPLAY_LOOPING);
            rainSoundIsPlaying_ = true;
        }

        SwitchPlayerHudAnimations();
    }

    return true;
}

//---------------------------------------------------------
// Desc:  call it each frame
//        update player's footsteps to step right goes properly after step left
//---------------------------------------------------------
void Game::UpdateFootstepsSound(const float dt)
{
    stepTimer_ += dt;

    if (soundStepL_Playing && !soundStepR_Played)
    {
        if (stepTimer_ < stepInterval_)
            return;

        // check if stepL finished playing
        if (WaitForSingleObject(eventStepLDone, 0) == WAIT_OBJECT_0)
        {
            // play second sound
            soundStepR_.GetBuffer()->SetCurrentPosition(0);
            soundStepR_.GetBuffer()->Play(0, 0, 0);

            soundStepR_Played = true;
            soundStepL_Playing = false;

            stepTimer_ = 0;
        }
    }
}

//---------------------------------------------------------
//---------------------------------------------------------
void Game::UpdateShootSound(const float dt)
{
    shootTimer_ += dt;

    if (soundShootIsPlaying_)
    {
        if (shootTimer_ < shootInterval_)
            return;

        Mouse& mouse = pEngine_->GetMouse();

        if (mouse.IsLeftDown())
        {
            shootTimer_ = 0;
            soundShoot_.StopTrack();
            soundShootIsPlaying_ = false;
        }
    }
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
                    soundRain_.StopTrack();    
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

                    printf("collect GPU: %d\n", (int)state.collectGpuMetrics);
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
    
    switch (code)
    {
        // switch weapon
        case KEY_1:
        case KEY_2:
        case KEY_3:
        case KEY_4:
        case KEY_5:
        case KEY_6:
        {
            
            if (!pEngine_->GetKeyboard().WasPressedBefore(code))
            {
                const int itemIdx = (int)code - (int)KEY_1;

                ECS::InventorySystem& inventorySys = pEnttMgr_->inventorySystem_;
                ECS::PlayerSystem& playerSys       = pEnttMgr_->playerSystem_;

                const EntityID playerId            = playerSys.GetPlayerID();
                const EntityID prevWeaponId        = playerSys.GetActiveWeapon();
                const EntityID itemId              = inventorySys.GetItemByIdx(playerId, itemIdx);

                playerSys.SetActiveWeapon(itemId);

                pEnttMgr_->RemoveComponent(prevWeaponId, ECS::RenderedComponent);
                pEnttMgr_->AddRenderingComponent(itemId);

                const char* itemName = pEnttMgr_->nameSystem_.GetNameById(itemId);
                printf("%s switch to weapon: %s%s\n", YELLOW, itemName, RESET);
            }
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
            StartFootstepSequence();
            UpdateRainPos();

            //pEnttMgr_->playerSystem_.SetIsWalking();
            break;
        }
        case KEY_D:
        {
            pEnttMgr_->AddEvent(EventPlayerMove(EVENT_PLAYER_MOVE_RIGHT));
            StartFootstepSequence();
            UpdateRainPos();

            //pEnttMgr_->playerSystem_.SetIsWalking();
            break;
        }
        case KEY_L:
        {
            // switch the flashlight
            if (!pEngine_->GetKeyboard().WasPressedBefore(KEY_L))
                SwitchFlashLight(*pEnttMgr_, *pRender_);
            break;
        }
        case KEY_S:
        {
            pEnttMgr_->AddEvent(EventPlayerMove(EVENT_PLAYER_MOVE_BACK));
            StartFootstepSequence();
            UpdateRainPos();

            //pEnttMgr_->playerSystem_.SetIsWalking();
            break;
        }
        case KEY_R:
        {
            AnimSkeleton& skeleton = g_AnimationMgr.GetSkeleton("ak_74_hud");
            const AnimationID    animId = skeleton.GetAnimationIdx("wpn_ak74_hud_ogf_reload");
            const AnimationClip& anim = skeleton.GetAnimation(animId);
            pEnttMgr_->playerSystem_.SetIsReloading(anim.GetEndTime());

            break;
        }
        case KEY_W:
        {
            pEnttMgr_->AddEvent(EventPlayerMove(EVENT_PLAYER_MOVE_FORWARD));

            AnimSkeleton& skeleton = g_AnimationMgr.GetSkeleton("ak_74_hud");
            const AnimationID    animId = skeleton.GetAnimationIdx("wpn_ak74_hud_ogf_idle_sprint__");
            const AnimationClip& anim = skeleton.GetAnimation(animId);
            pEnttMgr_->playerSystem_.SetIsWalking(anim.GetEndTime());

            if (pEnttMgr_->playerSystem_.IsRunning())
                stepInterval_ = 0.0f;
            else
                stepInterval_ = 0.5f;

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

    if (mouse.IsLeftDown())
    {
        //printf("shoot\n");
        StartPlayShootSound();
        AnimSkeleton& skeleton = g_AnimationMgr.GetSkeleton("ak_74_hud");
        const AnimationID    animId = skeleton.GetAnimationIdx("wpn_ak74_hud_ogf_shoot");
        const AnimationClip& anim = skeleton.GetAnimation(animId);

        pEnttMgr_->playerSystem_.SetIsShooting(anim.GetEndTime());
        //pEngine_->GetGraphicsClass().GetRayIntersectionPoint(mouseEvent.GetPosX(), mouseEvent.GetPosY());
    }

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

                ECS::PlayerSystem& player = pEnttMgr_->playerSystem_;
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
                //pEnttMgr_->playerSystem_.SetIsShooting(false, 0);
                break;
            }
        }
    }
}

//---------------------------------------------------------
// Desc:   switch on/off the player's flashlight
//---------------------------------------------------------
void Game::SwitchFlashLight(ECS::EntityMgr& mgr, Render::CRender& render)
{
    ECS::PlayerSystem& player     = mgr.playerSystem_;
    const bool isFlashlightActive = !player.IsFlashLightActive();;
    player.SwitchFlashLight(isFlashlightActive);

    // update the state of the flashlight entity
    const EntityID flashlightID = mgr.nameSystem_.GetIdByName("player_flashlight");
    mgr.lightSystem_.SetLightIsActive(flashlightID, isFlashlightActive);

    // set of flashlight is visible
    render.SwitchFlashLight(isFlashlightActive);


    // if we just turned on the flashlight we update its position and direction
    if (isFlashlightActive)
    {
        mgr.transformSystem_.SetPosition(flashlightID, player.GetPosition());
        mgr.transformSystem_.SetDirection(flashlightID, player.GetDirVec());
    }
}

//---------------------------------------------------------
// Desc:  call it each time when player's position is changed
//---------------------------------------------------------
void Game::UpdateMovementRelatedStuff()
{
    StartFootstepSequence();
    UpdateRainPos();
}

//---------------------------------------------------------
// Desc:  call it when player's position is changed:
//        start playing footsteps sound
//---------------------------------------------------------
void Game::StartFootstepSequence()
{
    if (!soundStepL_Playing)
    {
        soundStepL_.PlayTrack();

        soundStepL_Playing = true;
        soundStepR_Played  = false;
    }
}

//---------------------------------------------------------
//---------------------------------------------------------
void Game::StartPlayShootSound()
{
    if (!soundShootIsPlaying_)
    {
        soundShoot_.PlayTrack();
        soundShootIsPlaying_ = true;
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

    const EntityID rainbowAnomaly0     = nameSys.GetIdByName("anomaly_rainbow_0");
    const EntityID rainbowAnomaly1     = nameSys.GetIdByName("anomaly_rainbow_1");

    const cvector<ECS::Particle>& particles0 = particleSys.GetParticlesOfEmitter(rainbowAnomaly0);
    const cvector<ECS::Particle>& particles1 = particleSys.GetParticlesOfEmitter(rainbowAnomaly1);
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

    // place point light in exact position of related particle
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
void Game::InitSounds(ECS::EntityMgr& mgr)
{
    bool result = false;

    result = directSound_.Init(pEngine_->GetHWND());
    if (!result)
    {
        LogErr(LOG, "can't init direct sound");
        return;
    }

    const char* rainFilename      = "data/sounds/rain.wav";
    const char* stepLFilename     = "data/sounds/stepL_44khz.wav";
    const char* stepRFilename     = "data/sounds/stepR_44khz.wav";
    const char* ak74ShootFilename = "data/sounds/ak74_shoot.wav";

    const long volume      = 0;
    const long shootVolume = -2000;
    const long rainVolume  = -1500;
    const long stepsVolume = -2000;   // -2000 hundredths of a dB is roughly -20db

    IDirectSound8* pDirectSound = directSound_.GetDirectSound();

    //
    // load sound files
    //
    result = soundRain_.LoadTrack(pDirectSound, rainFilename, rainVolume);
    if (!result)
        LogErr(LOG, "can't load a rain sound from file: %s", rainFilename);

    result = soundStepL_.LoadTrack(pDirectSound, stepLFilename, 0);
    if (!result)
        LogErr(LOG, "can't load a sound from file: %s", stepLFilename);

    result = soundStepR_.LoadTrack(pDirectSound, stepRFilename, 0);
    if (!result)
        LogErr(LOG, "can't load a sound from file: %s", stepRFilename);

    result = soundShoot_.LoadTrack(pDirectSound, ak74ShootFilename, shootVolume);
    if (!result)
        LogErr(LOG, "can't load a sound: %s", ak74ShootFilename);


    //
    // create event handler for some sounds so we will be able to know when
    // sound is over, or it is currently playing
    //
    IDirectSoundBuffer8* pSoundBufStepL = soundStepL_.GetBuffer();
    IDirectSoundBuffer8* pSoundBufShoot = soundShoot_.GetBuffer();

    // create notification event
    eventStepLDone = CreateEvent(NULL, FALSE, FALSE, NULL);
    eventShootDone = CreateEvent(NULL, FALSE, FALSE, NULL);

    // attach notification to the END of buffer
    IDirectSoundNotify8* pNotifyStep = nullptr;
    IDirectSoundNotify8* pNotifyShoot = nullptr;
    pSoundBufStepL->QueryInterface(IID_IDirectSoundNotify8, (void**)&pNotifyStep);
    pSoundBufShoot->QueryInterface(IID_IDirectSoundNotify8, (void**)&pNotifyShoot);

    DSBPOSITIONNOTIFY notify = {};
    notify.dwOffset = DSBPN_OFFSETSTOP; // end of buffer
    
    notify.hEventNotify = eventStepLDone;
    pNotifyStep->SetNotificationPositions(1, &notify);
    pNotifyStep->Release();

    notify.hEventNotify = eventShootDone;
    pNotifyShoot->SetNotificationPositions(1, &notify);
    pNotifyShoot->Release();
}

//---------------------------------------------------------
// Desc:  switch animation of player's hands/weapon according to its state
//---------------------------------------------------------
void Game::SwitchPlayerHudAnimations()
{
    const ECS::PlayerSystem& player   = pEnttMgr_->playerSystem_;
    const AnimSkeleton&      skeleton = Core::g_AnimationMgr.GetSkeleton(currSkeletonId_);

    if (player.IsReloading())
    {
        const AnimationClip& anim = skeleton.GetAnimation(animIdReload_);
        pEnttMgr_->animationSystem_.SetAnimation(currHudId_, animIdReload_, anim.GetEndTime());
    }
    else if (player.IsShooting())
    {
        const AnimationClip& anim = skeleton.GetAnimation(animIdShoot_);
        pEnttMgr_->animationSystem_.SetAnimation(currHudId_, animIdShoot_, anim.GetEndTime());
    }
    else if (player.IsWalking())
    {
        const AnimationClip& anim = skeleton.GetAnimation(animIdRun_);
        pEnttMgr_->animationSystem_.SetAnimation(currHudId_, animIdRun_, anim.GetEndTime());
    }
    else if (player.IsIdle())
    {
        const AnimationClip& anim = skeleton.GetAnimation(animIdIdle_);
        pEnttMgr_->animationSystem_.SetAnimation(currHudId_, animIdIdle_, anim.GetEndTime());
    }
}

} // namespace
