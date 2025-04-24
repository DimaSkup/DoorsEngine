#pragma once

// Entity-Component-System
#include "Entity/EntityMgr.h"

namespace Game
{

struct CameraInitParams
{
    float nearZ    = 1;
    float farZ     = 100;
    float fovInRad = 1.30796f;

    float sensitivity = 1.0;     // camera rotation speed
    float walkSpeed   = 10.0f;
    float runSpeed    = 20.0f;

    SIZE windowedSize   = {800, 600};
    SIZE fullscreenSize = {800, 600};
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
    bool InitLightSources(ECS::EntityMgr& mgr);
    bool InitCameras(ECS::EntityMgr& mgr, const CameraInitParams& camParams);
    
};

}
