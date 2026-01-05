/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: GameInitializer.h
    Desc:     initialize scene elements
\**********************************************************************************/
#pragma once

#include <Entity/EntityMgr.h>
#include <Render/CRender.h>
#include <Engine/engine_configs.h>

namespace Game
{


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
    bool InitModelEntities(
        ECS::EntityMgr& enttMgr,
        Render::CRender& render,
        const Core::EngineConfigs* pConfigs);

    void InitPlayer(
        ID3D11Device* pDevice,
        ECS::EntityMgr* pEnttMgr,
        const Core::EngineConfigs* pConfigs);

    bool InitCamera(
        ECS::EntityMgr& mgr,
        const char* cameraName,
        CameraInitParams& initParams);

    void InitParticles    (const char* filepath, ECS::EntityMgr& mgr);
    void InitLightEntities(const char* filepath, ECS::EntityMgr& mgr);
};

}
