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

    InitParticles(*pEnttMgr);

    pRender->ShadersHotReload(pEngine->GetGraphicsClass().GetD3DClass().GetDevice());

    return true;
}

//---------------------------------------------------------
// Desc:    update the game
// Args:    - deltaTime:   the time passed since the previous frame
//---------------------------------------------------------
bool Game::Update(const float deltaTime)
{
    cvector<ECS::ParticleSystem>& particleSystems = pEnttMgr_->particleEngine_.particleSystems_;

    // generate particles for different particle systems
#if 0
    particleSystems[0].CreateParticles(10);
    //particleSystems[1].CreateParticle();
    //particleSystems[2].CreateParticles(5);
#else

    particleSystems[0].CreateParticles(deltaTime);
    particleSystems[1].CreateParticles(deltaTime);
    particleSystems[2].CreateParticles(deltaTime);

#endif


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

//---------------------------------------------------------
//---------------------------------------------------------
void Game::InitParticles(ECS::EntityMgr& enttMgr)
{
    // INIT PARTICLES SYSTEMS
    ECS::ParticleEngine& particleEngine = enttMgr.particleEngine_;
    particleEngine.LoadFromFile("data/particles/particles.cfg");


    // TEMP: manually setup material for each particles system
    const MaterialID matIdFlame = g_MaterialMgr.GetMaterialIdByName("flameMat");
    const MaterialID matIdFlare = g_MaterialMgr.GetMaterialIdByName("flareMat");
    const MaterialID matIdCat   = g_MaterialMgr.GetMaterialIdByName("catParticleMat");

    ECS::ParticleSystem& particleSysSparcles   = particleEngine.GetSystemByName("sparcles");
    ECS::ParticleSystem& particleSysFire       = particleEngine.GetSystemByName("fire");
    ECS::ParticleSystem& particleSysMagicSpell = particleEngine.GetSystemByName("magic_spell");

    particleSysSparcles.SetMaterialId(matIdFlare);
    particleSysFire.SetMaterialId(matIdFlame);
    particleSysMagicSpell.SetMaterialId(matIdCat);

    const DirectX::XMFLOAT3 aabbCenter  = { 0,0,0 };
    const DirectX::XMFLOAT3 aabbExtents = { 0.5f, 0.5f, 0.5f };
    const DirectX::BoundingBox& aabb = { aabbCenter, aabbExtents };

    // setup point light for flame
    ECS::PointLightsInitParams pointLightsParams;
    pointLightsParams.data.resize(1);
    ECS::PointLight& initData = pointLightsParams.data[0];

    initData.diffuse = { 0.8f, 0.6f, 0.05f, 1.0f };
    initData.ambient = initData.diffuse * 0.25f;
    initData.specular = initData.diffuse;
    initData.att = { 0.1f, 0.1f, 0.005f };
    initData.range = 50;

    // create flame entity
    const EntityID flame1EnttId = enttMgr.CreateEntity("flame1");
    enttMgr.AddTransformComponent(flame1EnttId, { 257,101,235 });
    enttMgr.AddParticleEmitterComponent(flame1EnttId, particleSysFire);
    enttMgr.AddBoundingComponent(flame1EnttId, ECS::BoundingType::BOUND_BOX, aabb);
    enttMgr.AddLightComponent(&flame1EnttId, 1, pointLightsParams);

    // create flame entity
    const EntityID flame2EnttId = enttMgr.CreateEntity("flame2");
    enttMgr.AddTransformComponent(flame2EnttId, { 240,77,215 });
    enttMgr.AddParticleEmitterComponent(flame2EnttId, particleSysFire);
    enttMgr.AddBoundingComponent(flame2EnttId, ECS::BoundingType::BOUND_BOX, aabb);
    enttMgr.AddLightComponent(&flame2EnttId, 1, pointLightsParams);


    initData.diffuse  = { 0.0f, 0.8f, 0.05f, 1.0f };
    initData.ambient  = initData.diffuse * 0.25f;
    initData.specular = initData.diffuse;
    initData.att      = { 0, 0.1f, 0.005f };
    initData.range    = 50;

    // create green sparcles entity
    const EntityID sparclesEnttId = enttMgr.CreateEntity("sparcles");
    enttMgr.AddTransformComponent(sparclesEnttId, { 250,77,215 });
    enttMgr.AddParticleEmitterComponent(sparclesEnttId, particleSysSparcles);
    enttMgr.AddBoundingComponent(sparclesEnttId, ECS::BoundingType::BOUND_BOX, aabb);
    enttMgr.AddLightComponent(&sparclesEnttId, 1, pointLightsParams);
}

} // namespace
