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
#include "Render.h"
#include "Entity/EntityMgr.h"
#include "../Texture/TextureMgr.h"
#include "../Mesh/MaterialMgr.h"

#include <vector>

namespace Core
{

class RenderDataPreparator
{
public:
    RenderDataPreparator(
        Render::Render& render,
        ECS::EntityMgr& enttMgr);

    void PrepareInstanceFromModel(
        BasicModel& model,
        Render::Instance& instance);

    void PrepareEnttsDataForRendering(
        const EntityID* enttsIds,
        const size numEntts,
        Render::InstBuffData& instanceBuffData,      // data for the instance buffer
        std::vector<Render::Instance>& instances);   // instances (models subsets) data for rendering

    // ----------------------------------------------------

    void PrepareInstancesData(
        const EntityID* ids,
        const size numEntts,
        std::vector<Render::Instance>& instances,
        cvector<EntityID>& enttsSortedByModels);

    void PrepareInstanceData(const BasicModel& model, Render::Instance& instance);
      
    // ----------------------------------------------------

    void PrepareEnttsBoundingLineBox(
        const EntityID* visibleEntts,
        const size numEntts,
        Render::Instance& instance,
        Render::InstBuffData& instanceBuffer);

    void PrepareEnttsMeshesBoundingLineBox(
        const EntityID* visibleEntts,
        const size numEntts,
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
        std::vector<Render::Instance>& instances,
        cvector<EntityID>& outEnttsSortedByInstances);

    void PrepareInstancesForEnttsWithUniqueMaterials(
        ECS::EntityMgr& mgr,
        const EntityID* ids,
        const size numEntts,
        std::vector<Render::Instance>& instances,
        cvector<EntityID>& outEnttsSortedByInstances);

    void PrepareTexturesForInstance(Render::Instance& instance);

private:
    Render::Render* pRender_ = nullptr;    // a ptr to the Render class of the Render module
    ECS::EntityMgr* pEnttMgr_ = nullptr;   // a ptr to the EntityMgr class of the ECS module
};

} // namespace Core
