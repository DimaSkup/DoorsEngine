//==================================================================================
// Filename:   Game.cpp
// Created:    25.07.2025  by DimaSkup
//==================================================================================
#include "pch.h"
#include "Game.h"
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
    Render::CRender* pRender)
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

    pEngine_ = pEngine;
    pEnttMgr_ = pEnttMgr;
    pRender_ = pRender;

    const TerrainGeomip& terrain = g_ModelMgr.GetTerrainGeomip();

    // INIT PARTICLES
    constexpr int maxNumParticles = 10000;
    ECS::ParticleEngine& particleEngine = pEnttMgr->particleEngine_;
    ECS::ParticleSystem& particleSys1 = particleEngine.AddNewParticleSys(maxNumParticles);
    ECS::ParticleSystem& particleSys2 = particleEngine.AddNewParticleSys(maxNumParticles);
    ECS::ParticleSystem& particleSys3 = particleEngine.AddNewParticleSys(maxNumParticles);

    const int offsetOverTerrain = 5;

    // setup the first particle system
    int posX = 250;
    int posZ = 215;
    float posY = terrain.GetScaledHeightAtPoint(posX, posZ) + offsetOverTerrain;
    particleSys1.SetEmitPos((float)posX, posY, (float)posZ);

    particleSys1.SetLife(1000);
    particleSys1.SetColor(0.1f, 1.0f, 0.25f);
    particleSys1.SetSize(0.05f);
    particleSys1.SetMass(1.25f);
    particleSys1.SetFriction(0.01f);
    particleSys1.SetExternalForces(0.0f, -0.001f, 0.0f);

    // setup the second particle system
    posX = 270;
    posZ = 215;
    posY = terrain.GetScaledHeightAtPoint(posX, posZ) + offsetOverTerrain;
    particleSys2.SetEmitPos((float)posX, posY, (float)posZ);

    particleSys2.SetLife(1000);
    particleSys2.SetColor(1.0f, 1.0f, 0.0f);
    particleSys2.SetSize(0.1f);
    particleSys2.SetMass(1.0f);
    particleSys2.SetFriction(0.05f);
    particleSys2.SetExternalForces(0.0f, 0.001f, 0.0f);

    // setup the second particle system
    posX = 260;
    posZ = 215;
    posY = terrain.GetScaledHeightAtPoint(posX, posZ) + offsetOverTerrain;
    particleSys3.SetEmitPos((float)posX, posY, (float)posZ);

    particleSys3.SetLife(10000);
    particleSys3.SetColor(1.0f, 0.96f, 0.0f);
    particleSys3.SetSize(0.05f);
    particleSys3.SetMass(1.0f);
    particleSys3.SetFriction(0.01f);
    particleSys3.SetExternalForces(0.0f, 0.0f, 0.0f);

    return true;
}

//---------------------------------------------------------
// Desc:    update the game
// Args:    - deltaTime:   the time passed since the previous frame
//---------------------------------------------------------
bool Game::Update(const float deltaTime)
{
    pEnttMgr_->particleEngine_.Explode(0.15f, 100);

    if (pEngine_->IsGameMode())
    {
        HandleGameEventKeyboard();
        HandleGameEventMouse(deltaTime);
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
    // switch on/off the player's flashlight

    ECS::PlayerSystem& player = mgr.playerSystem_;
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
