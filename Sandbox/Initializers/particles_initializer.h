/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: particles_initializer.h
    Desc:     functional for parsing declarations of particle emitters
              from the config file and create these particle emitters

    Created:  21.01.2025  by DimaSkup
\**********************************************************************************/
#pragma once

#include <Entity/EntityMgr.h>


namespace Game
{

class ParticlesInitializer
{
public:
    bool Init(const char* configFilepath, ECS::EntityMgr& enttMgr);

private:
    void ReadAndCreateEmitter(
        ECS::EntityMgr& enttMgr,
        FILE* pFile,
        const char* emitterName,
        const int emitterIdx);
};

} // namespace
