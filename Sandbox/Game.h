#pragma once
#include <Entity/EntityMgr.h>
#include <UI/user_interface.h>
#include <Render/CRender.h>            
#include <Input/inputcodes.h>
#include <Engine/engine.h>
#include <Engine/engine_configs.h>
#include <Sound/direct_sound.h>
#include <Sound/sound.h>

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

    void SwitchPlayerHudAnimations();

    void UpdateShootSound(const float dt);
    void StartPlayShootSound();

private:
    Core::Engine*    pEngine_    = nullptr;
    ECS::EntityMgr*  pEnttMgr_   = nullptr;
    Render::CRender* pRender_    = nullptr;

    Core::DirectSound directSound_;
    Core::Sound       soundRain_;
    Core::Sound       soundStepL_;
    Core::Sound       soundStepR_;
    Core::Sound       soundShoot_;

    HANDLE eventStepLDone = nullptr;
    HANDLE eventShootDone = nullptr;

    bool engineModeWasChanged_ = false;  // if we did switching from editor to game mode or vise versa

    bool rainSoundIsPlaying_ = false;
    bool soundStepL_Playing = false;
    bool soundStepR_Played  = false;
    bool soundShootIsPlaying_ = false;

    float stepTimer_ = 0;
    float stepInterval_ = 0.5f;
    float shootTimer_ = 0;
    float shootInterval_ = 0.374994f;

    // rain over player
    EntityID    rainEnttId_     = 0;

    // player animation stuff
    EntityID    currHudId_      = 0;
    SkeletonID  currSkeletonId_ = 0;
    AnimationID animIdReload_   = 0;
    AnimationID animIdShoot_    = 0;
    AnimationID animIdRun_      = 0;
    AnimationID animIdIdle_     = 0;

    
};

} // namespace
