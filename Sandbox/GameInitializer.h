#pragma once

#include <Entity/EntityMgr.h>
#include <Render/CRender.h>

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
    bool InitModelEntities(ID3D11Device* pDevice, ECS::EntityMgr& enttMgr, Render::CRender& render);
    void InitPlayer       (ID3D11Device* pDevice, ECS::EntityMgr* pEnttMgr);
    bool InitLightSources(ECS::EntityMgr& mgr);

    bool InitCamera(
        ECS::EntityMgr& mgr,
        const char* cameraName,
        CameraInitParams& initParams);

    void InitParticles(ECS::EntityMgr& mgr);
};

}
