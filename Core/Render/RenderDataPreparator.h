// ********************************************************************************
// Filename:     RenderDataPreparator.h
// Description:  functional is used to prepare scene data for rendering
// 
// Created:      17.10.24
// ********************************************************************************
#pragma once

#include "../Texture/TextureTypes.h"
#include "../Model/BasicModel.h"
#include "../Model/ModelMgr.h"

#include <CoreCommon/Types.h>  
#include "CRender.h"
#include "Entity/EntityMgr.h"
#include "../Texture/TextureMgr.h"
#include "../Mesh/MaterialMgr.h"

#include <vector>

namespace Core
{

class RenderDataPreparator
{
public:
    RenderDataPreparator();

    void PrepareInstanceFromModel(
        BasicModel& model,
        Render::Instance& instance);

    void PrepareEnttsDataForRendering(
        const EntityID* enttsIds,
        const size numEntts,
        ECS::EntityMgr* pEnttMgr,
        Render::InstBuffData& instanceBuffData,      // data for the instance buffer
        Render::cvector<Render::Instance>& instances);   // instances (models subsets) data for rendering

    // ----------------------------------------------------

    void PrepareInstancesData(
        const EntityID* ids,
        const size numEntts,
        ECS::EntityMgr* pEnttMgr,
        Render::cvector<Render::Instance>& instances,
        cvector<EntityID>&        outEnttsSortedByModels,
        cvector<Render::Material>& outMaterialsSortedByInstances);

    void PrepareInstanceData(const BasicModel& model, Render::Instance& instance);
      
    // ----------------------------------------------------

    void PrepareEnttsBoundingLineBox(
        ECS::EntityMgr* pEnttMgr,
        Render::Instance& instance,
        Render::InstBuffData& instanceBuffer);

    void PrepareEnttsMeshesBoundingLineBox(
        ECS::EntityMgr* pEnttMgr,
        Render::Instance& instance,
        Render::InstBuffData& instanceBuffer);

private:
    void SeparateEnttsByMaterialGroups(
        const ECS::EntityMgr& mgr,
        const EntityID* ids,
        const size numEntts,
        cvector<EntityID>& outEnttsWithOrigMat,
        cvector<EntityID>& outEnttsWithUniqueMat);

    void PrepareInstancesForEntts(
        ECS::EntityMgr& mgr,
        const EntityID* ids,
        const size numEntts,
        Render::cvector<Render::Instance>& instances,
        cvector<EntityID>& outEnttsSortedByInstances);

    void PrepareInstancesForEnttsWithUniqueMaterials(
        ECS::EntityMgr& mgr,
        const EntityID* ids,
        const size numEntts,
        Render::cvector<Render::Instance>& instances,
        cvector<EntityID>& outEnttsSortedByInstances);

    void PrepareTexturesForInstance(Render::Instance& instance);
    void PrepareMaterialForInstance(const Render::Instance& instance, cvector<Render::Material>& outMat);
};

} // namespace Core
