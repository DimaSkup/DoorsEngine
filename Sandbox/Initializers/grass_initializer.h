/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: grass_initializer.h
    Desc:     functional for parsing grass parameters from a config file
              and create all the grass instances

    Created:  25.01.2026  by DimaSkup
\**********************************************************************************/
#pragma once

#include <Entity/EntityMgr.h>

namespace Game
{

class GrassInitializer
{
public:
    bool Init(const char* cfgFilepath, ECS::EntityMgr& mgr);
};

}
