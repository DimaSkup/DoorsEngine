#pragma once
#include <Entity/EntityMgr.h>
#include <UI/user_interface.h>
#include <Render/CRender.h>            
#include <Input/inputcodes.h>
#include <Engine/engine.h>
#include <Engine/engine_configs.h>
#include <Sound/direct_sound.h>
#include <Sound/sound.h>
#include <Model/animation_mgr.h>

namespace Game
{

class Game
{
public:
    Game() {}
    ~Game() {}

    bool Init(
        Core::Engine* pEngine,
        ECS::EntityMgr* pEntityMgr,
        Render::CRender* pRender,
        const Core::EngineConfigs& settings);

    bool Update(const float deltaTime, const float gameTime);

    // handle input
    void HandleGameEventMouse(const float deltaTime);
    void HandleGameEventKeyboard();
    void HandlePlayerActions(const eKeyCodes code);
    

    void SwitchFlashLight(ECS::EntityMgr& mgr, Render::CRender& render);

private:
    void InitParticles(ECS::EntityMgr& mgr);
    void InitSounds(ECS::EntityMgr& mgr);

    void UpdateMovementRelatedStuff();

    void StartFootstepSequence();
    void UpdateFootstepsSound(const float dt);

    void UpdateRainbowAnomaly();
    void UpdateRainPos();

private:
    Core::Engine*    pEngine_    = nullptr;
    ECS::EntityMgr*  pEnttMgr_ = nullptr;
    Render::CRender* pRender_    = nullptr;

    Core::DirectSound directSound_;
    Core::Sound       soundRain_;
    Core::Sound       soundStepL_;
    Core::Sound       soundStepR_;

    HANDLE eventStepLDone = nullptr;

    bool engineModeWasChanged_ = false;  // if we did switching from editor to game mode or vise versa

    bool rainSoundIsPlaying_ = false;
    bool soundStepL_Playing = false;
    bool soundStepR_Played  = false;

    float stepTimer = 0;
    float stepInterval = 0.5f;
};

} // namespace
