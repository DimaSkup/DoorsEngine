// ********************************************************************************
// Filename:     RenderDataPreparator.h
// Description:  functional is used to prepare scene data for rendering
// 
// Created:      17.10.24
// ********************************************************************************
#pragma once

#include "../Model/BasicModel.h"
#include <Common/ECSTypes.h>
#include <Types.h>  
#include "Entity/EntityMgr.h"
#include "CRender.h"

namespace Core
{

class RenderDataPreparator
{
private:
    using SRV = ID3D11ShaderResourceView;

public:
    RenderDataPreparator();

    void PrepareEnttsDataForRendering(
        const EntityID* enttsIds,
        const size numEntts,
        ECS::EntityMgr* pEnttMgr,
        Render::RenderDataStorage& storage);


private:

    void PrepareInstancesWorldMatrices(
        const EntityID* enttsIds,
        const size numEntts,
        Render::InstBuffData& instanceBuffData,         
        const cvector<Render::InstanceBatch>& instances);

    void PrepareInstancesMaterials(
        Render::InstBuffData& instanceBuffData,
        const cvector<EntityModelMesh>& instances,
        const cvector<Render::Material>& materialsSortedByInstances);

private:
    //constexpr int     numElems = 1028;
    TexID             texturesIDs_[1028]{ INVALID_TEXTURE_ID };
   // cvector<SRV*>     textureSRVs_;
    cvector<Material> materials_;

    cvector<EntityID>         enttsSortedByInstances_;
    cvector<Render::Material> materialsSortedByInstances_;
};

} // namespace Core
