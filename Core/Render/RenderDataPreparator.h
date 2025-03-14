// ********************************************************************************
// Filename:     RenderDataPreparator.h
// Description:  functional is used to prepare scene data for rendering
// 
// Created:      17.10.24
// ********************************************************************************
#pragma once

#include "../Texture/TextureHelperTypes.h"
#include "../Model/BasicModel.h"
#include "../Model/ModelStorage.h"

#include <CoreCommon/Types.h>    // ECS types
#include "Render.h"
#include "Entity/EntityMgr.h"
#include "../Texture/TextureMgr.h"

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
		const std::vector<EntityID>& enttsIds,
		Render::InstBuffData& instanceBuffData,      // data for the instance buffer
		std::vector<Render::Instance>& instances);   // instances (models subsets) data for rendering
		
	void PrepareInstanceData(
		const BasicModel& model,
		Render::Instance& instance,
		TextureMgr& texMgr);

	void PrepareEnttsBoundingLineBox(
		const std::vector<EntityID>& visibleEntts,
		Render::Instance& instance,
		Render::InstBuffData& instanceBuffer);

	void PrepareEnttsMeshesBoundingLineBox(
		const std::vector<EntityID>& visibleEntts,
		Render::Instance& instance,
		Render::InstBuffData& instanceBuffer);

	void PrepareInstancesData(
		const std::vector<EntityID>& ids,
		std::vector<Render::Instance>& instances,
		std::vector<EntityID>& enttsSortedByModels);            // alpha clipping flags for each instance);

	void PrepareInstancesOfEnttWithOwnTex(
		const std::vector<EntityID>& enttsWithOwnTex,
		ECS::EntityMgr& mgr,
		std::vector<Render::Instance>& instances,
		std::map<ModelID, Render::Instance*>& modelIdToInstance,
		ModelStorage& storage,
		TextureMgr& texMgr,
		int& instanceIdx);

	void PrepareSubsetsData(
		const MeshGeometry::Subset* subsets,
		std::vector<Render::Subset>& instanceSubsets);

	void PrepareSubsetsMaterials(
		const MeshMaterial* srcModelMats,
		const int numMats,
		Render::Material* instanceMats);

private:
	Render::Render* pRender_ = nullptr;
	ECS::EntityMgr* pEnttMgr_ = nullptr;
};

} // namespace Core