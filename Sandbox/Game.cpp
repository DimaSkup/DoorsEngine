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
    StartFootstepSequence();


    // manually add animations
    using namespace Core;
    using namespace DirectX;

    AnimSkeleton&  skeleton   = g_AnimationMgr.GetSkeleton("boblampclean");
    const int      animTestId = skeleton.AddAnimation("test");
    AnimationClip& anim       = skeleton.GetAnimation(animTestId);

    skeleton.DumpBoneParents();

    // add a couple keyframes to a single bone
    anim.boneAnimations.resize(skeleton.GetNumBones());
    cvector<Keyframe>& keyframes = anim.boneAnimations[0].keyframes;
    keyframes.resize(5);

    const XMVECTOR q0 = XMQuaternionRotationAxis({ 0,1,0,0 }, DEG_TO_RAD(+30));
    const XMVECTOR q1 = XMQuaternionRotationAxis({ 1,1,2,0 }, DEG_TO_RAD(+45));
    const XMVECTOR q2 = XMQuaternionRotationAxis({ 0,1,0,0 }, DEG_TO_RAD(-30));
    const XMVECTOR q3 = XMQuaternionRotationAxis({ 1,0,0,0 }, DEG_TO_RAD(+70));

    

    keyframes[0].timePos     = 0.0f;
    keyframes[0].translation = XMFLOAT3(-7, 0, 0);
    keyframes[0].scale       = 0.25f;
    XMStoreFloat4(&keyframes[0].rotQuat, q0);

    keyframes[1].timePos = 2.0f;
    keyframes[1].translation = XMFLOAT3(0, 2, 10);
    keyframes[1].scale = 0.5f;
    XMStoreFloat4(&keyframes[1].rotQuat, q1);

    keyframes[2].timePos = 4.0f;
    keyframes[2].translation = XMFLOAT3(7, 0, 0);
    keyframes[2].scale = 0.25;
    XMStoreFloat4(&keyframes[2].rotQuat, q2);

    keyframes[3].timePos = 6.0f;
    keyframes[3].translation = XMFLOAT3(0, 1, -10);
    keyframes[3].scale = 0.5f;
    XMStoreFloat4(&keyframes[3].rotQuat, q3);

    keyframes[4].timePos = 8.0f;
    keyframes[4].translation = XMFLOAT3(-7, 0, 0);
    keyframes[4].scale = 0.25f;
    XMStoreFloat4(&keyframes[4].rotQuat, q0);

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
    UpdateRainbowAnomaly();

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
    }

    return true;
}

//---------------------------------------------------------
// Desc:  call it each frame
//        update player's footsteps to step right goes properly after step left
//---------------------------------------------------------
void Game::UpdateFootstepsSound(const float dt)
{
    stepTimer += dt;

    if (soundStepL_Playing && !soundStepR_Played)
    {
        if (stepTimer < stepInterval)
            return;

        // check if stepL finished playing
        if (WaitForSingleObject(eventStepLDone, 0) == WAIT_OBJECT_0)
        {
            // play second sound
            soundStepR_.GetBuffer()->SetCurrentPosition(0);
            soundStepR_.GetBuffer()->Play(0, 0, 0);

            soundStepR_Played = true;
            soundStepL_Playing = false;

            stepTimer = 0;
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
            break;
        }
        case KEY_D:
        {
            pEnttMgr_->AddEvent(EventPlayerMove(EVENT_PLAYER_MOVE_RIGHT));
            StartFootstepSequence();
            UpdateRainPos();
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
            break;
        }
        case KEY_W:
        {
            pEnttMgr_->AddEvent(EventPlayerMove(EVENT_PLAYER_MOVE_FORWARD));

            if (pEnttMgr_->playerSystem_.IsRunning())
                stepInterval = 0.0f;
            else
                stepInterval = 0.5f;

            UpdateMovementRelatedStuff();

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
}

//---------------------------------------------------------
// Desc:   hangle mouse input events
//---------------------------------------------------------
void Game::HandleGameEventMouse(const float deltaTime)
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
                const float rotY  = mouseEvent.GetPosX() * deltaTime;
                const float pitch = mouseEvent.GetPosY() * deltaTime;

                ECS::PlayerSystem& player = pEnttMgr_->playerSystem_;
                player.RotateY(rotY);
                player.Pitch(pitch);
                break;
            }
            case MouseEvent::EventType::LPress:
            {
                pEngine_->GetGraphicsClass().GetRayIntersectionPoint(mouseEvent.GetPosX(), mouseEvent.GetPosY());
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
// Desc:  update position of the rain so it always over the player
//---------------------------------------------------------
void Game::UpdateRainPos()
{
    const EntityID          rainEnttId = pEnttMgr_->nameSystem_.GetIdByName("rain_over_player");
    const DirectX::XMFLOAT3 playerPos  = pEnttMgr_->playerSystem_.GetPosition();
    const ECS::EventTranslate evnt(rainEnttId, playerPos.x, playerPos.y, playerPos.z);

    pEnttMgr_->AddEvent(evnt);
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

    const long volume = 0;
    const long rainVolume = -1500;
    const long stepsVolume = -2000;   // -2000 hundredths of a dB is roughly -20db

    IDirectSound8* pDirectSound = directSound_.GetDirectSound();


    result = soundRain_.LoadTrack(pDirectSound, rainFilename, rainVolume);
    if (!result)
        LogErr(LOG, "can't load a rain sound from file: %s", rainFilename);

    result = soundStepL_.LoadTrack(pDirectSound, stepLFilename, stepsVolume);
    if (!result)
        LogErr(LOG, "can't load a sound from file: %s", stepLFilename);

    result = soundStepR_.LoadTrack(pDirectSound, stepRFilename, stepsVolume);
    if (!result)
        LogErr(LOG, "can't load a sound from file: %s", stepRFilename);


    // play the sound
    IDirectSoundBuffer8* pSoundBufStepL = soundStepL_.GetBuffer();

    // create notification event
    eventStepLDone = CreateEvent(NULL, FALSE, FALSE, NULL);

    // attach notification to the END of stepL buffer
    IDirectSoundNotify8* pNotify = nullptr;
    pSoundBufStepL->QueryInterface(IID_IDirectSoundNotify8, (void**)&pNotify);

    DSBPOSITIONNOTIFY notify = {};
    notify.dwOffset = DSBPN_OFFSETSTOP; // end of buuffer
    notify.hEventNotify = eventStepLDone;

    pNotify->SetNotificationPositions(1, &notify);
    pNotify->Release();
}

} // namespace
