/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: sky_initializer.h
    Desc:     functional for parsing sky parameters from a config file
              and create all the sky related stuff (sky box, sky dome, sky plane, etc.)

    Created:  02.04.2026  by DimaSkup
\**********************************************************************************/
#pragma once

#include <Entity/EntityMgr.h>

namespace Game
{

class SkyInitializer
{
public:
    bool Init(const char* cfgFilepath, ECS::EntityMgr& enttMgr);
};

}
