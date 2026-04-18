/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: game_initializer.h
    Desc:     initialize scene elements
\**********************************************************************************/
#pragma once

#include <Entity/EntityMgr.h>
#include <Render/CRender.h>
#include <Engine/engine_configs.h>

namespace Game
{

//---------------------------------------------------------
// paths to config files for initialization of different parts of the game
//---------------------------------------------------------
struct GameInitPaths
{
    char levelName[64];

    char materialsFilepath[64];
    char terrainFilepath[64];
    char skyFilepath[64];
    char skyPlaneFilepath[64];

    char modelsFilepath[64];
    char animationsFilepath[64];
    char entitiesFilepath[64];
    char natureGenFilepath[64];

    char particlesFilepath[64];
    char grassFilepath[64];
    char lightsFilepath[64];
    char weaponsFilepath[64];

    char sprites2dFilepath[64];
    char playerFilepath[64];
    char texturesFilepath[64];
    char soundsFilepath[64];
};

//---------------------------------------------------------
// parameters for camera initialization
//---------------------------------------------------------
struct CameraInitParams
{
    float nearZ         = -1;
    float farZ          = -1;
    float fovInRad      = -1;     // vertical field of view in radians 
    float wndWidth      = 800;
    float wndHeight     = 600;
    float posX          = 0;
    float posY          = 0;
    float posZ          = 0;
};

///////////////////////////////////////////////////////////

class GameInitializer
{
public:

    void ReadGameInitPaths(const char* level, GameInitPaths& initPaths);

    bool InitEntities(
        ECS::EntityMgr& enttMgr,
        Render::CRender& render,
        const Core::EngineConfigs& cfgs,
        const GameInitPaths& initPaths);


    bool InitCamera(
        ECS::EntityMgr& mgr,
        const char* cameraName,
        CameraInitParams& initParams);

    void InitPlayer     (const char* cfgFilepath, ECS::EntityMgr& mgr);
    void InitParticles  (const char* cfgFilepath, ECS::EntityMgr& mgr);
    void InitGrass      (const char* cfgFilepath, ECS::EntityMgr& mgr);
    void InitLights     (const char* cfgFilepath, ECS::EntityMgr& mgr);

    void Create2dSprites(const char* cfgFilepath, ECS::EntityMgr& mgr, const Render::CRender& render);

private:
    void ReadLevelInitPaths(FILE* pFile, GameInitPaths& initPaths);
};

}
