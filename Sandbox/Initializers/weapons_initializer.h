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
#include <Entity/EntityMgr.h>

namespace Game
{

class WeaponsInitializer
{
public:
    void Init(const char* weaponsCfgFilepath, ECS::EntityMgr* pEnttMgr);
};

} // namespace Game
