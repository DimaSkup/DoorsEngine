/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: weapon_types.h
    Desc:     enums, structures, typedefs which are related to weapons

    Created:  05.01.2025  by DimaSkup
\**********************************************************************************/
#pragma once

#include <types.h>
#include <Sound/sound_mgr.h>
#include <Model/animation_mgr.h>

namespace Game
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
    Weapon() {}

    EntityID             enttId = 0;
    eWeaponType          type = WPN_TYPE_MACHINE_GUN;

    SoundID              soundIds[NUM_WPN_SOUND_TYPES]{ 0 };

    Core::AnimSkeleton*  pSkeleton = nullptr;
    AnimationID          animIds[NUM_WPN_ANIM_TYPES];
};

} // namespace Game
