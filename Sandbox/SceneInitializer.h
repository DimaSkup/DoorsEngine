#pragma once

// Entity-Component-System
#include "Entity/EntityMgr.h"

namespace Game
{

struct CameraInitParams
{
    float nearZ       = -1;
    float farZ        = -1;
    float fovInRad    = -1;     // vertical field of view in radians 
    float aspectRatio = -1;     // screen width / screen height
    float wndWidth    = 800;
    float wndHeight   = 600;
};

///////////////////////////////////////////////////////////

class SceneInitializer
{
public:
    bool Initialize(
        ID3D11Device* pDevice,
        ECS::EntityMgr& enttMgr,
        const CameraInitParams& editorCamParams,
        const CameraInitParams& gameCamParams);

private:
    bool InitModelEntities(ID3D11Device* pDevice, ECS::EntityMgr& enttMgr);
    void InitPlayer(ID3D11Device* pDevice, ECS::EntityMgr* pEnttMgr);
    bool InitLightSources(ECS::EntityMgr& mgr);

    bool InitCameras(
        ECS::EntityMgr& mgr,
        const CameraInitParams& editorCamParams,
        const CameraInitParams& gameCamParams);
    
};

}
