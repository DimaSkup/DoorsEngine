#pragma once

// Entity-Component-System
#include "Entity/EntityMgr.h"

namespace Game
{

class SceneInitializer
{
public:
    bool Initialize(ID3D11Device* pDevice, ECS::EntityMgr& enttMgr);

private:
    bool InitializeModelEntities(ID3D11Device* pDevice, ECS::EntityMgr& enttMgr);
};

}
