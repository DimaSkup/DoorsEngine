/*********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: player_initializer.h
    Desc:     functional for reading main player's parameters,
              creation and setup the player

    Created:  03.04.2026  by DimaSkup
\**********************************************************************************/
#pragma once
#include <Entity/EntityMgr.h>

namespace Game
{

class PlayerInitializer
{
public:
    static bool Init(const char* cfgFilepath, ECS::EntityMgr& enttMgr);
};

} // namespace
