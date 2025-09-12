//==================================================================================
// Filename:   Game.cpp
// Created:    25.07.2025  by DimaSkup
//==================================================================================
#include "pch.h"
#include "Game.h"
#include "GameInitializer.h"
#include <Engine/Engine.h>
#include <Model/ModelMgr.h>

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
    const EngineConfigs& configs)
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

  
    LogMsg(LOG, "scene initialization (start)");

    GameInitializer gameInit;
    bool result = false;

    Core::CGraphics& graphics   = pEngine->GetGraphicsClass();
    const Render::D3DClass& d3d = graphics.GetD3DClass();
    const SIZE windowedSize     = d3d.GetWindowedWndSize();
    const SIZE fullscreenSize   = d3d.GetFullscreenWndSize();
    ID3D11Device* pDevice       = d3d.GetDevice();


    // create and init scene elements
    if (!gameInit.InitModelEntities(pDevice, *pEnttMgr, *pRender))
    {
        LogErr(LOG, "can't initialize models");
    }


    // init all the light source on the scene
    if (!gameInit.InitLightSources(*pEnttMgr))
    {
        LogErr(LOG, "can't initialize light sources");
    }


    //-----------------------------------------------------

    // init all the cameras
    const float nearZ       = configs.GetFloat("NEAR_Z");
    const float farZ        = configs.GetFloat("FAR_Z");
    const float fovInRad    = configs.GetFloat("FOV_IN_RAD");         // field of view in radians

    CameraInitParams editorCamParams;
    editorCamParams.wndWidth    = (float)windowedSize.cx;
    editorCamParams.wndHeight   = (float)windowedSize.cy;
    editorCamParams.nearZ       = nearZ;
    editorCamParams.farZ        = farZ;
    editorCamParams.fovInRad    = fovInRad;
#if 1
    editorCamParams.posX        = 235.0f;
    editorCamParams.posY        = 80.0f;
    editorCamParams.posZ        = 200.0f;
#else
    editorCamParams.posX = 0;
    editorCamParams.posY = 0;
    editorCamParams.posZ = 0;
#endif

    CameraInitParams gameCamParams;
    gameCamParams.wndWidth      = (float)fullscreenSize.cx;
    gameCamParams.wndHeight     = (float)fullscreenSize.cy;
    gameCamParams.nearZ         = nearZ;
    gameCamParams.farZ          = farZ;
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

    // create and setup a player entity
    gameInit.InitPlayer(pDevice, pEnttMgr);

    // add some entities with particle emitters
    gameInit.InitParticles(*pEnttMgr);


    LogMsg(LOG, "is initialized");
    return true;
}

//---------------------------------------------------------
// Desc:    update the game
// Args:    - dt:   the time passed since the previous frame
//---------------------------------------------------------
bool Game::Update(const float dt)
{
    // generate particles
    pEnttMgr_->particleSystem_.CreateParticles(dt);

    // handle events
    if (pEngine_->IsGameMode())
    {
        HandleGameEventKeyboard();
        HandleGameEventMouse(dt);
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
            case KEY_F1:
            {
                // switch from game to the editor mode
                if (!keyboard.WasPressedBefore(KEY_F1))
                    pEngine_->SwitchEngineMode();

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
            break;
        }
        case KEY_D:
        {
            pEnttMgr_->AddEvent(EventPlayerMove(EVENT_PLAYER_MOVE_RIGHT));
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
            break;
        }
        case KEY_W:
        {
            pEnttMgr_->AddEvent(EventPlayerMove(EVENT_PLAYER_MOVE_FORWARD));
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
    const EntityID flashlightID = mgr.nameSystem_.GetIdByName("flashlight");
    mgr.lightSystem_.SetLightIsActive(flashlightID, isFlashlightActive);

    // set of flashlight is visible
    ID3D11DeviceContext* pContext = pEngine_->GetD3DClass().GetDeviceContext();
    render.SwitchFlashLight(pContext, isFlashlightActive);


    // if we just turned on the flashlight we update its position and direction
    if (isFlashlightActive)
    {
        mgr.transformSystem_.SetPosition(flashlightID, player.GetPosition());
        mgr.transformSystem_.SetDirection(flashlightID, player.GetDirVec());
    }
}

} // namespace
