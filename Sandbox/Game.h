#pragma once
#include <Entity/EntityMgr.h>
#include <UI/user_interface.h>
#include <Render/CRender.h>            
#include <Input/inputcodes.h>
#include <Engine/engine.h>
#include <Engine/engine_configs.h>
#include <Sound/direct_sound.h>
#include <Sound/sound.h>
#include <Model/animation_helper.h>


namespace Game
{

constexpr int NUM_WEAPONS = 3;

//---------------------------------------------------------

enum eWeaponAnimType
{
    WPN_ANIM_TYPE_DRAW,
    WPN_ANIM_TYPE_RELOAD,
    WPN_ANIM_TYPE_SHOOT,
    WPN_ANIM_TYPE_RUN,
    WPN_ANIM_TYPE_IDLE,

    NUM_WPN_ANIM_TYPES,
};

//---------------------------------------------------------

enum eWeaponSoundType
{
    WPN_SOUND_TYPE_DRAW,     // sound when weapon appears
    WPN_SOUND_TYPE_RELOAD,
    WPN_SOUND_TYPE_SHOOT,

    NUM_WPN_SOUND_TYPES,
};

//---------------------------------------------------------

struct Weapon
{
    EntityID            enttId = 0;

    Core::Sound         sounds[NUM_WPN_SOUND_TYPES];
    HANDLE              eventShootDone = nullptr;

    Core::AnimSkeleton* pSkeleton = nullptr;
    AnimationID         animIds[NUM_WPN_ANIM_TYPES];
};

//---------------------------------------------------------

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
    void InitWeapons();

    void InitParticles(ECS::EntityMgr& mgr);
    void InitSounds(ECS::EntityMgr& mgr);

    void UpdateMovementRelatedStuff();

    void StartFootstepSequence(const float dt);
    void UpdateFootstepsSound(const float dt);

    void UpdateRainbowAnomaly();
    void UpdateRainPos();

    void UpdatePlayerAnimHud();

    void UpdateShootSound(const float dt);
    void StartPlayShootSound();

    inline Weapon& GetCurrentWeapon() { return weapons_[currWeaponIdx_]; }

private:
    Core::Engine*    pEngine_    = nullptr;
    ECS::EntityMgr*  pEnttMgr_   = nullptr;
    Render::CRender* pRender_    = nullptr;

    Weapon weapons_[NUM_WEAPONS];


    Core::DirectSound directSound_;
    Core::Sound       soundRain_;
    Core::Sound       soundStepL_;
    Core::Sound       soundStepR_;

    HANDLE eventStepLDone = nullptr;

    bool canSwitchAnimation_ = true;

    bool engineModeWasChanged_ = false;  // if we did switching from editor to game mode or vise versa

    bool rainSoundIsPlaying_ = false;
    bool soundStepL_Playing = false;
    bool soundStepR_Played  = false;
    bool soundShootIsPlaying_ = false;

    float stepTimer_ = 0;
    float currStepInterval_ = 0.5f;
    float stepIntervalWalk_ = 0.5f;
    float stepIntervalRun_ = 0.1f;

    float shootTimer_ = 0;
    float shootInterval_ = 0.374994f;
    //float shootInterval_ = 0.2f;

    // rain over player
    EntityID    rainEnttId_     = 0;

    // player animation stuff
    Core::AnimSkeleton* pSkeleton_ = nullptr;

    int         currWeaponIdx_    = 0;
    EntityID    currWeaponEnttId_ = 0;
    AnimationID currAnimId_       = -1;

    float currActTime_ = 0;
    float endActTime_ = 0;

    float deltaTime_ = 0;
    float gameTime_ = 0;

    int thunderSoundIdx = 0;
    Core::Sound thunderSounds[4];
    float thunderTimer_ = 0;
    int thunderInterval_ = 30;
};

} // namespace
