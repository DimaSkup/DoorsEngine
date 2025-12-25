// ********************************************************************************
// Filename:     RenderDataPreparator.h
// Description:  functional is used to prepare scene data for rendering
// 
// Created:      17.10.24
// ********************************************************************************
#pragma once

#include <Types.h>  
#include "Entity/EntityMgr.h"
#include <Render/CRender.h>

namespace Core
{

class RenderDataPreparator
{
private:
    using SRV = ID3D11ShaderResourceView;

public:
    RenderDataPreparator();

    void PrepareEnttsDataForRendering(
        cvector<EntityID>& visibleEntts,
        const DirectX::XMFLOAT3& cameraPos,
        ECS::EntityMgr* pEnttMgr,
        Render::RenderDataStorage& storage);

private:
    void PrepareInstancesWorldMatrices(
        const cvector<EntityID>& enttIdPerInstance,
        Render::RenderDataStorage& storage);
};

} // namespace Core
