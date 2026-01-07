#pragma once
#include <types.h>
#include <Entity/EntityMgr.h>
#include <UI/user_interface.h>
#include <Render/CRender.h>            
#include <Engine/engine.h>
#include <Engine/engine_configs.h>
#include <Model/animation_helper.h>
#include "../Types/weapon_types.h"


namespace Game
{

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


//**********************************************************************************

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

    void HandleBulletHit(const IntersectionData& data);

    // weapon/hands animations switching
    void StartAnimWeaponDraw();
    void StartAnimWeaponReload();
    void StartAnimWeaponShoot();
    void StartAnimWeaponRun();
    void StartAnimWeaponIdle();



private:
    GameEventsList gameEventsList_;

    Core::Engine*    pEngine_    = nullptr;
    ECS::EntityMgr*  pEnttMgr_   = nullptr;
    Render::CRender* pRender_    = nullptr;

    float deltaTime_ = 0;
    float gameTime_ = 0;

    //
    // player's weapons and related stuff
    //
    cvector<Weapon> weapons_;

    Core::AnimSkeleton* pSkeleton_ = nullptr;   // skeleton of the player's current hud (hands + weapon)

    float shootTimer_ = 0;                      // time passed since the start of shot
    float shootInterval_ = 0.374994f;           // within this interval we aren't able to make a new shot
    float currActTime_ = 0;                     // time passed since the start of player's animation (handls, weapon, etc.)
    float endActTime_ = 0;                      // duration of the current player's animation

    int         currWeaponIdx_ = 0;
    EntityID    currWeaponEnttId_ = 0;
    AnimationID currAnimId_ = -1;

    //
    // sounds related stuff
    //
    HANDLE eventStepLDone = nullptr;

    bool rainSoundIsPlaying_    = false;
    bool soundStepL_Playing     = false;
    bool soundStepR_Played      = false;
    bool soundShootIsPlaying_   = false;

    float stepTimer_        = 0;
    float currStepInterval_ = 0.5f;
    float stepIntervalWalk_ = 0.5f;
    float stepIntervalRun_  = 0.1f;


    // rain over player
    EntityID rainEnttId_ = 0;

    // each thunderInterval seconds we play some thunder sound
    int thunderInterval_ = 30;

    SoundID thunderSoundIds_[4] = {0};
    SoundID rainSoundId_ = 0;
    SoundID actorStepL_ = 0;
    SoundID actorStepR_ = 0;
};

} // namespace
