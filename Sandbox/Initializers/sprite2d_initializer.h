/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: sprite2d_initializer.h
    Desc:     functional for parsing sprites parameters from a config file
              and create all the 2d sprites

    Created:  02.04.2026  by DimaSkup
\**********************************************************************************/
#pragma once

#include <Entity/EntityMgr.h>

namespace Game
{

class Sprite2dInitializer
{
public:
    bool Init(
        const char* cfgFilepath,
        ECS::EntityMgr& enttMgr,
        const int wndWidth,
        const int wndHeight);
};

}
