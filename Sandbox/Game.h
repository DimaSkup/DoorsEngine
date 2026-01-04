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

enum eWeaponType
{
    WPN_TYPE_KNIFE,
    WPN_TYPE_PISTOL,        // PM, TT, revolvers, etc. (shoot only by single LMB click)
    WPN_TYPE_SHOTGUN,
    WPN_TYPE_MACHINE_GUN,   // AK-47/74, etc.          (shoot by pressing/holding LMB)
};

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

    eWeaponType         type = WPN_TYPE_MACHINE_GUN;

    SoundID             soundIds[NUM_WPN_SOUND_TYPES]{0};
    HANDLE              eventShootDone = nullptr;

    Core::AnimSkeleton* pSkeleton = nullptr;
    AnimationID         animIds[NUM_WPN_ANIM_TYPES];

    IDirectSoundNotify8* pNotifyShootDone = nullptr;
};

//---------------------------------------------------------

enum eGameEventType
{
    NONE,
    PLAYER_SWITCH_WEAPON,
    PLAYER_RELOAD_WEAPON,
    PLAYER_SINGLE_SHOT,
    PLAYER_MULTIPLE_SHOTS,
    PLAYER_RUN,
    PLAYER_SWITCH_FLASHLIGHT,
};

// =================================================================================
// Basic event
// =================================================================================
struct GameEvent
{
    eGameEventType type;
    float x;
};

//---------------------------------------------------------

struct GameEventsList
{
    void Reset()
    {
        numEvents = 0;
    }

    void PushEvent(const GameEvent e)
    {
        events[numEvents] = e;
        ++numEvents;
    }

    GameEvent events[16];
    int numEvents = 0;     // for the current frame
};

//---------------------------------------------------------

class Game
{
public:
    Game() {}
    ~Game();

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
    void HandleGameEvents();

    void InitWeapons();
    void InitWeapon(FILE* pFile, const char* line, Weapon& wpn);
    void InitSoundsStuff();
    void InitParticles(ECS::EntityMgr& mgr);

    void UpdateMovementRelatedStuff();

    void StartFootstepSequence(const float dt);
    void UpdateFootstepsSound(const float dt);

    void UpdateRainbowAnomaly();
    void UpdateRainPos();

    void UpdateShootSound(const float dt);
    void StartPlayShootSound();

    inline Weapon& GetCurrentWeapon() { return weapons_[currWeaponIdx_]; }

    // events handlers
    void HandleEventWeaponSwitch(const int newWeaponIdx);
    void HandleEventWeaponReload();
    void HandleEventWeaponSingleShot();
    void HandleEventWeaponMultipleShots();

    // weapon/hands animations switching
    void StartAnimWeaponDraw();
    void StartAnimWeaponReload();
    void StartAnimWeaponShoot();
    void StartAnimWeaponRun();
    void StartAnimWeaponIdle();



private:
    Core::Engine*    pEngine_    = nullptr;
    ECS::EntityMgr*  pEnttMgr_   = nullptr;
    Render::CRender* pRender_    = nullptr;

    cvector<Weapon> weapons_;


    GameEventsList gameEventsList_;


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
    float thunderTimer_ = 0;
    int thunderInterval_ = 30;

    bool isShooting_ = false;
    bool execDrawing_ = false;
    bool execShot_ = false;
    bool execShotSequence_ = false;

    float shotTime_ = 0;

    SoundID thunderSoundIds_[4] = {0};
    SoundID rainSoundId_ = 0;
    SoundID actorStepL_ = 0;
    SoundID actorStepR_ = 0;
};

} // namespace
