/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: weapons_initializer.h
    Desc:     functional for weapons initialization and binding some to the player

    Created:  05.01.2025  by DimaSkup
\**********************************************************************************/
#pragma once
#include "../Types/weapon_types.h"
#include <Entity/EntityMgr.h>
#include <cvector.h>

namespace Game
{

class WeaponsInitializer
{
public:
    void Init(
        const char* weaponsCfgFile,
        ECS::EntityMgr* pEnttMgr,
        cvector<Weapon>& playerWeapons);
};

} // namespace Game
