#pragma once
#include <types.h>
#include <Entity/EntityMgr.h>
#include <UI/user_interface.h>
#include <Render/CRender.h>            
#include <Engine/engine.h>
#include <Engine/engine_configs.h>
#include <Model/animation_helper.h>
#include "event_mgr.h"



namespace Game
{

#if 0
enum eGameEventType
{
    NONE,
    PLAYER_SWITCH_WEAPON,
    PLAYER_RELOAD_WEAPON,
    PLAYER_SINGLE_SHOT,           // we clicked shooting only once
    PLAYER_MULTIPLE_SHOTS,        // we holding down "shooting" button
    PLAYER_RUN,
    PLAYER_TOGGLE_FLASHLIGHT,     // turn on/off the flashlight
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
#endif

//**********************************************************************************

class Game
{
private:
    EventMgr eventMgr_;


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
    
private:
    void HandleGameEvents();
    void InitSoundsStuff();

    void UpdatePlayerFootstepsSound(const float dt);
    void UpdateRainbowAnomaly();
    void UpdateRainPos();
    void UpdateShootSound(const float dt);

private:
   // GameEventsList gameEventsList_;

    Core::Engine*    pEngine_    = nullptr;
    ECS::EntityMgr*  pEnttMgr_   = nullptr;
    Render::CRender* pRender_    = nullptr;

    float deltaTime_ = 0;
    float gameTime_ = 0;

    //
    // player's weapons and related stuff
    //
    Core::AnimSkeleton* pSkeleton_ = nullptr;   // skeleton of the player's current hud (hands + weapon)

    

    int         currWeaponIdx_ = 0;
    EntityID    currWeaponEnttId_ = 0;
    AnimationID currAnimId_ = -1;

    //
    // sounds related stuff
    //
    HANDLE eventStepLDone = nullptr;

    bool rainSoundIsPlaying_    = false;



    // rain over player
    EntityID rainEnttId_ = 0;

    // each thunderInterval seconds we play some thunder sound
    int thunderInterval_ = 30;

    SoundID thunderSoundIds_[4] = {0};
    SoundID rainSoundId_ = 0;

};

} // namespace
