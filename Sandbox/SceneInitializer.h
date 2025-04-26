#pragma once

// Entity-Component-System
#include "Entity/EntityMgr.h"

namespace Game
{

struct CameraInitParams
{
    float nearZ    = -1;
    float farZ     = -1;
    float fovInRad = -1;
    float aspectRatio = -1;   // screen width / screen height
};

///////////////////////////////////////////////////////////

class SceneInitializer
{
public:
    bool Initialize(
        ID3D11Device* pDevice,
        ECS::EntityMgr& enttMgr,
        const CameraInitParams& camParams);

private:
    bool InitModelEntities(ID3D11Device* pDevice, ECS::EntityMgr& enttMgr);
    void InitPlayer(ID3D11Device* pDevice, ECS::EntityMgr* pEnttMgr, const CameraInitParams& camParams);
    bool InitLightSources(ECS::EntityMgr& mgr);
    bool InitCameras(ECS::EntityMgr& mgr, const CameraInitParams& camParams);
    
};

}
