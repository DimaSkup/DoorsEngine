/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: light_initializer.h
    Desc:     functional for parsing lights parameters from a config file
              and create all the light sources

    Created:  06.03.2026  by DimaSkup
\**********************************************************************************/
#pragma once
#include <Entity/EntityMgr.h>

namespace Game
{

class LightInitializer
{
public:
    bool Init(const char* filepath, ECS::EntityMgr& mgr);
};

}
