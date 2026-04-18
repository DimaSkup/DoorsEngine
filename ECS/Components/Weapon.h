// =================================================================================
// Filename:   Weapon.h
// Desc:       ECS component for weapon's data
//
// Created:    11.04.2026  by DimaSkup
// =================================================================================
#pragma once
#include <types.h>
#include <cvector.h>

namespace ECS
{

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
    WPN_ANIM_TYPE_SHOT,
    WPN_ANIM_TYPE_RUN,
    WPN_ANIM_TYPE_IDLE,

    NUM_WPN_ANIM_TYPES,
};

//---------------------------------------------------------

enum eWeaponSoundType
{
    WPN_SOUND_TYPE_DRAW,     // sound when weapon appears
    WPN_SOUND_TYPE_RELOAD,
    WPN_SOUND_TYPE_SHOT,

    NUM_WPN_SOUND_TYPES,
};

//---------------------------------------------------------
// weapon data
//---------------------------------------------------------
struct Weapon
{
    Weapon() :
        enttId(0),
        type(WPN_TYPE_PISTOL),
        skeletonId(0),
        shotInterval(0.374994f)
    {
        memset(soundIds, 0, sizeof(soundIds));
        memset(animIds, 0, sizeof(animIds));
    }


    EntityID      enttId;

    // what kind of weapon it represents(shotgun, pistol, etc.)
    eWeaponType   type;

    // sound fx per each weapon action (shoot, reload, etc.)
    SoundID       soundIds[NUM_WPN_SOUND_TYPES];

    // identifier of skeleton to animate this weapon
    SkeletonID    skeletonId;

    // animations for this weapon
    AnimationID   animIds[NUM_WPN_ANIM_TYPES];

    // within this interval we aren't able to make a new shot
    float         shotInterval;
};

//---------------------------------------------------------
// ECS component
//---------------------------------------------------------
struct WeaponComp
{
    WeaponComp()
    {
        ids.resize(8);
        weapons.resize(8);

        // create "dummy" weapon
        ids.push_back(INVALID_ENTT_ID);
        weapons.push_back(Weapon());
    }

    cvector<EntityID> ids;
    cvector<Weapon>   weapons;
};

} // namespace 
