// ********************************************************************************
// Filename:     RenderDataPreparator.cpp
// 
// Created:      17.10.24
// ********************************************************************************
#include "RenderDataPreparator.h"

#include "../Common/Utils.h"
#include "../Texture/TextureMgr.h"


using RenderDataStorage = Render::RenderDataStorage;


///////////////////////////////////////////////////////////

RenderDataPreparator::RenderDataPreparator(
	Render::Render& render,
	ECS::EntityMgr& enttMgr)
	:
	pRender_(&render),
	pEnttMgr_(&enttMgr)
{
}

///////////////////////////////////////////////////////////

void RenderDataPreparator::PrepareEnttsDataForRendering(
	const std::vector<EntityID>& enttsIds,
	Render::InstBuffData& instanceBuffData,              // prepared data for the instance buffer
	std::vector<Render::Instance>& instances)       // models data for rendering
{
	std::vector<EntityID> enttsSortedByModels;
	std::vector<MeshMaterial> materials;

	//static float t = 0.0f;
	//t += 0.01f;

	std::vector<XMMATRIX> enttsWorlds;
	std::vector<XMMATRIX> enttsTexTransforms;
	int numElems = 0;


	// get data of models which are related to the input entts
	PrepareInstancesData(enttsIds, instances, enttsSortedByModels);

	// compute num of elements for the instanced buffer
	for (const Render::Instance& instance : instances)
		numElems += (instance.numInstances * (int)std::ssize(instance.subsets));

	// prepare memory for instances transient data buffer
	instanceBuffData.Resize(numElems);


	// get world matrix and texture transforma matrix for each entity
	pEnttMgr_->transformSystem_.GetWorldMatricesOfEntts(
		enttsSortedByModels, 
		enttsWorlds);

	pEnttMgr_->texTransformSystem_.GetTexTransformsForEntts(
		enttsSortedByModels, 
		enttsTexTransforms);



	// setup world matrices
	for (int worldIdx = 0, i = 0; const Render::Instance& instance : instances)
	{
		for (int subsetIdx = 0; subsetIdx < instance.subsets.size(); ++subsetIdx)
		{
			for (int j = 0; j < instance.numInstances; ++j)
			{
				instanceBuffData.worlds_[i++] = enttsWorlds[worldIdx + j];
			}
		}
		
		worldIdx += instance.numInstances;
	}

	// setup texture transform matrices
	for (int transformIdx = 0, i = 0; const Render::Instance& instance : instances)
	{
		for (int subsetIdx = 0; subsetIdx < instance.subsets.size(); ++subsetIdx)
		{
			for (int j = 0; j < instance.numInstances; ++j)
			{
				instanceBuffData.texTransforms_[i++] = enttsTexTransforms[transformIdx + j];
			}
		}

		transformIdx += instance.numInstances;
	}


	// setup materials
	for (int i = 0; const Render::Instance& instance : instances)
	{
		for (int subsetIdx = 0; subsetIdx < instance.subsets.size(); ++subsetIdx)
		{
			for (int j = 0; j < instance.numInstances; ++j)
			{
				instanceBuffData.materials_[i++] = instance.materials[subsetIdx];
			}
		}
	}
}

// ********************************************************************************
// 
//                                PRIVATE METHODS
// 
// ********************************************************************************

void PrepareSubsetsData(
	const MeshGeometry::Subset* subsets,
	std::vector<Render::Subset>& instanceSubsets)
{
	// prepare data of each subset
	for (int meshSubIdx = 0; Render::Subset & subset : instanceSubsets)
	{
		const MeshGeometry::Subset& subsetData = subsets[meshSubIdx++];
		subset.name = subsetData.name_;          // for debug

#if 0
		// copy 4 ints: vertex start/count, index start/count
		memcpy(&subset.vertexStart, &subsetData.vertexStart_, sizeof(int) * 4);
#else
		subset.vertexStart = subsetData.vertexStart_;
		subset.vertexCount = subsetData.vertexCount_;
		subset.indexStart = subsetData.indexStart_;
		subset.indexCount = subsetData.indexCount_;
#endif
	}
}

///////////////////////////////////////////////////////////

void PrepareSubsetsMaterials(
	const MeshMaterial* srcModelMats,
	const int numMats,
	Render::Material* instanceMats)
{
#if 1
	// prepare data of each subset material  
	memcpy(
		instanceMats,                     // dst
		srcModelMats,                     // src
		4 * numMats * sizeof(XMFLOAT4));  // 4 mat params * subsets_count * sz(FLOAT4)

#else
	// prepare data of each subset material  
	for (int matIdx = 0; matIdx < model.numMats_; ++matIdx)
	{
		const MeshMaterial& meshMat = model.materials_[matIdx];
		Render::Material& instMat = instance.materials[matIdx];

		instMat.ambient_ = meshMat.ambient_;
		instMat.diffuse_ = meshMat.diffuse_;
		instMat.specular_ = meshMat.specular_;
		instMat.reflect_ = meshMat.reflect_;
	}
#endif
}

///////////////////////////////////////////////////////////

void RenderDataPreparator::PrepareInstanceData(
	const BasicModel& model,
	Render::Instance& instance,
	TextureMgr& texMgr)
{
	const MeshGeometry& meshes = model.meshes_;

	instance.name = model.name_;
	instance.subsets.resize(model.GetNumSubsets());
	instance.materials.resize(model.GetNumSubsets());

	instance.vertexStride = meshes.vb_.GetStride();
	instance.pVB = meshes.vb_.Get();
	instance.pIB = meshes.ib_.Get();

	// copy subsets data into instance
	PrepareSubsetsData(meshes.subsets_, instance.subsets);

	// copy materials data from model into instance
	PrepareSubsetsMaterials(
		model.materials_,
		model.numMats_,
		instance.materials.data());

	// prepare textures (SRVs) for this instance
	texMgr.GetSRVsByTexIDs(model.texIDs_, model.numTextures_, instance.texSRVs);
}

///////////////////////////////////////////////////////////

void RenderDataPreparator::PrepareInstancesOfEnttWithOwnTex(
	const std::vector<EntityID>& enttsWithOwnTex,
	ECS::EntityMgr& mgr,
	std::vector<Render::Instance>& instances,
	std::map<ModelID, Render::Instance*>& modelIdToInstance,
	ModelStorage& storage,
	TextureMgr& texMgr,
	int& instanceIdx)
{
	// prepare instances of entts which have own textures in a separate way
	for (const EntityID enttID : enttsWithOwnTex)
	{
		ModelID modelID = mgr.modelSystem_.GetModelIdRelatedToEntt(enttID);
		Render::Instance& instance = instances[instanceIdx];
		const ECS::TexturedData& texData = mgr.texturesSystem_.GetDataByEnttID(enttID);

		// if we already prepared an instance data for this model
		if (modelIdToInstance.contains(modelID))
		{
			// just copy it 
			instance = *modelIdToInstance[modelID];

			// define what part of textures arr will be rewritten 
			int texSRVOffset = texData.submeshID_ * 22;

			// get SRVs for this submesh
			std::vector<SRV*> srvs;
			texMgr.GetSRVsByTexIDs(texData.texIDs_, 22, srvs);

			// rewrite part of SRVs with new values
			std::copy(srvs.begin(), srvs.end(), instance.texSRVs.data() + texSRVOffset);
		}

		// we haven't loaded this model data yet (to make an instance)
		else
		{
			const BasicModel& model = storage.GetModelByID(modelID);

			modelIdToInstance.insert({ modelID, &instance });
			PrepareInstanceData(model, instance, texMgr);

			// -------------------------------------------
			// setup textures

			// copy textures IDs into temp buffer
			TexID* tempTexIDs = new TexID[model.numTextures_];
			std::copy(model.texIDs_, model.texIDs_ + model.numTextures_, tempTexIDs);

			// define what part of textures arr will be rewritten 
			int startTexIdOffset = texData.submeshID_ * 22;

			// copy entt own textures into temp buffer
			std::copy(texData.texIDs_, texData.texIDs_ + 22, tempTexIDs + startTexIdOffset);

			// get SRVs by textures IDs
			texMgr.GetSRVsByTexIDs(tempTexIDs, model.numTextures_, instance.texSRVs);

			// release textures IDs temp buffer
			SafeDeleteArr(tempTexIDs);
		}

		instance.numInstances = 1;
		++instanceIdx;
	}
}

///////////////////////////////////////////////////////////

void RenderDataPreparator::PrepareInstancesData(
	const std::vector<EntityID>& ids,
	std::vector<Render::Instance>& instances,
	std::vector<EntityID>& enttsSortedByModels)
{
	TextureMgr& texMgr = *TextureMgr::Get();
	ModelStorage& storage = *ModelStorage::Get();

	std::vector<ModelID> modelsIDs;
	std::vector<int> numEnttsPerModel;
	std::vector<EntityID> enttsWithOwnTex;
	std::vector<EntityID> enttsWithMeshTex;

	std::map<ModelID, Render::Instance*> modelIdToInstance;
	int instanceIdx = 0;


	// separate entts into two arrays: 
	// 1. entts which have own textures (differ from its mesh textures)
	//    and will be rendered as separate instances
	// 2. entts which will be painted with its mesh textures
	pEnttMgr_->texturesSystem_.FilterEnttsWhichHaveOwnTex(
		ids,
		enttsWithOwnTex,
		enttsWithMeshTex);

	// get models IDs for entts with mesh tex
	pEnttMgr_->modelSystem_.GetModelsIdsRelatedToEntts(
		enttsWithMeshTex,
		modelsIDs,
		enttsSortedByModels,
		numEnttsPerModel);

	// define how many instances will we have at all
	instances.resize(std::ssize(modelsIDs) + std::ssize(enttsWithOwnTex));

	
	// prepare instances of entts which haven't own textures set
	// (they will be textures with mesh textures)
	for (const ModelID id : modelsIDs)
	{
		Render::Instance& instance = instances[instanceIdx];

		modelIdToInstance.insert({ id, &instance });            // make pair ['model_id' => 'ptr_instance']
		instance.numInstances = numEnttsPerModel[instanceIdx];  // how many instances of this model will be rendered
		++instanceIdx;

		PrepareInstanceData(storage.GetModelByID(id), instance, texMgr);
	}

	// prepare instances of entts which have textures differ from 
	// textures of its mesh;
	PrepareInstancesOfEnttWithOwnTex(
		enttsWithOwnTex,
		*pEnttMgr_,
		instances,
		modelIdToInstance,
		storage,
		texMgr,
		instanceIdx);

	// store ID of entts which have own textures
	for (const EntityID enttID : enttsWithOwnTex)
		enttsSortedByModels.push_back(enttID);
}

///////////////////////////////////////////////////////////

void RenderDataPreparator::PrepareEnttsBoundingLineBox(
	const std::vector<EntityID>& visibleEntts,
	Render::Instance& instance,
	Render::InstBuffData& instanceBuffer)
{
	// prepare data to render bounding box of each visible entity
	// (BB of the whole model)

	ModelStorage&   modelStorage = *ModelStorage::Get();
	ECS::EntityMgr&       mgr = *pEnttMgr_;
	std::vector<ModelID>  modelsIDs;
	std::vector<EntityID> enttsSortByModels;
	std::vector<int>      numInstancesPerModel;
	std::vector<DirectX::XMMATRIX> boxLocals;         // local space matrix of the AABB of the whole model
	std::vector<DirectX::XMMATRIX> worlds;            // world matrix of each visible entity


	mgr.modelSystem_.GetModelsIdsRelatedToEntts(
		visibleEntts,
		modelsIDs,
		enttsSortByModels,
		numInstancesPerModel);
	
	// get AABB of each visible model
	std::vector<DirectX::BoundingBox> modelsAABBs(std::ssize(modelsIDs));

	for (int i = 0; const ModelID id : modelsIDs)
		modelsAABBs[i++] = modelStorage.GetModelByID(id).GetModelAABB();

	// get world matrix for each entts and local space matrix by each model's AABB
	mgr.transformSystem_.GetWorldMatricesOfEntts(enttsSortByModels, worlds);
	mgr.boundingSystem_.GetBoxesLocalSpaceMatrices(modelsAABBs, boxLocals);

	// prepare instances buffer data
	instance.numInstances = (int)std::ssize(visibleEntts);
	instanceBuffer.Resize(instance.numInstances);

	// prepare world matrix for each bounding box to render
	for (index i = 0, instIdx = 0; i < std::ssize(boxLocals); ++i)
	{
		for (index j = 0; j < numInstancesPerModel[i]; ++j, ++instIdx)
			instanceBuffer.worlds_[instIdx] = boxLocals[i] * worlds[instIdx];
	}
}

///////////////////////////////////////////////////////////

void RenderDataPreparator::PrepareEnttsMeshesBoundingLineBox(
	const std::vector<EntityID>& visibleEntts,
	Render::Instance& instance,
	Render::InstBuffData& instanceBuffer)
{
	// prepare data to render bounding box of each mesh of each visible entity

	ECS::EntityMgr& mgr = *pEnttMgr_;
	std::vector<size> numBoxesPerEntt;                     // how many boxes each entt has
	std::vector<DirectX::XMMATRIX> locals;                 // local space of each OBB of each entity
	std::vector<DirectX::XMMATRIX> worlds;                 // world matrix of each visible entity

	mgr.boundingSystem_.GetBoxLocalSpaceMatrices(visibleEntts, numBoxesPerEntt, locals);
	mgr.transformSystem_.GetWorldMatricesOfEntts(visibleEntts, worlds);

	// prepare instances buffer data
	instance.numInstances = (int)std::ssize(locals);
	instanceBuffer.Resize(instance.numInstances);

	for (int instanceIdx = 0, i = 0; const DirectX::XMMATRIX & world : worlds)
	{
		for (index localIdx = 0; localIdx < numBoxesPerEntt[i]; ++localIdx, ++instanceIdx)
			instanceBuffer.worlds_[instanceIdx] = locals[instanceIdx] * world;
		++i;
	}
}