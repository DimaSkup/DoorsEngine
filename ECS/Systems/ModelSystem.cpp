#include "ModelSystem.h"

#include "../Common/Assert.h"
#include "../Common/Utils.h"
#include "../Common/log.h"
#include "SaveLoad/ModelSysSerDeser.h"

#include <stdexcept>

namespace ECS
{

ModelSystem::ModelSystem(Model* pModelComponent) : pModelComponent_(pModelComponent)
{
	Assert::NotNullptr(pModelComponent, "ptr to the Model component == nullptr");
}

///////////////////////////////////////////////////////////

void ModelSystem::Serialize(std::ofstream& fout, u32& offset)
{
#if 0
	MeshSysSerDeser::Serialize(
		fout,
		offset,
		static_cast<u32>(ComponentType::ModelComponent),  // data block marker
		pModelComponent_->enttToModel_);
#endif
}

///////////////////////////////////////////////////////////

void ModelSystem::Deserialize(std::ifstream& fin, const u32 offset)
{
#if 0
	MeshSysSerDeser::Deserialize(
		fin,
		offset,
		pModelComponent_->enttToModel_,
		pModelComponent_->modelToEntt_);
#endif
}

///////////////////////////////////////////////////////////

void ModelSystem::AddRecords(
	const std::vector<EntityID>& enttsIDs,
	const ModelID modelID)
{
	// add the same model to each input entt

	Model& comp = *pModelComponent_;

	// make relations one to one: 'entity_id' => 'model_id'
	for (int i = 0; const EntityID & enttID : enttsIDs)
		comp.enttToModel_[enttID] = modelID;

	// make relations one to many: 'model_id' => 'arr_of_entts_ids'
	for (int i = 0; const EntityID& enttID : enttsIDs)
		comp.modelToEntt_[modelID].insert(enttID);
}

///////////////////////////////////////////////////////////

void ModelSystem::AddRecords(
	const std::vector<EntityID>& enttsIDs,
	const std::vector<ModelID>& modelsIDs)
{
	Model& comp = *pModelComponent_;

	// make relations one to one: 'entity_id' => 'model_id'
	for (int i = 0; const EntityID& enttID : enttsIDs)
		comp.enttToModel_[enttID] = modelsIDs[i++];

	// make relations one to many: 'model_id' => 'arr_of_entts_ids'
	for (int i = 0; const ModelID& modelID : modelsIDs)
		comp.modelToEntt_[modelID].insert(enttsIDs[i++]);
}

///////////////////////////////////////////////////////////

void ModelSystem::RemoveRecords(const std::vector<EntityID>& enttsIDs)
{
	Assert::True(false, "TODO: IMPLEMENT IT!");
}

///////////////////////////////////////////////////////////

ModelID ModelSystem::GetModelIdRelatedToEntt(const EntityID enttID)
{
	// return a model ID by related input entt ID
	try
	{
		return pModelComponent_->enttToModel_.at(enttID);
	}
	catch (const std::out_of_range& e)
	{
		Log::Error(e.what());
		Log::Error("no such entity: " + std::to_string(enttID));

		return 0;  // ID of invalid model
	}
}

///////////////////////////////////////////////////////////

void ModelSystem::GetModelsIdsRelatedToEntts(
	const std::vector<EntityID>& enttsIDs,         // by these entts we will get models
	std::vector<ModelID>& outModelsIDs,            // models by these IDs will be rendered for this frame
	std::vector<EntityID>& outEnttsSortByModels,
	std::vector<int>& outNumInstancesPerModel)
{
	// in: array of entts IDs
	// 
	// out: 1) arr of models which are related to the input entities
	//      2) arr of entts sorted by its models
	//      3) arr of entts number per model
	
	const Model& comp = *pModelComponent_;
	std::map<ModelID, std::set<EntityID>> modelToEntts;
	
	// make a map of: 'model_id' => 'arr_entts_ids'
	for (const EntityID enttID : enttsIDs)
	{
		const ModelID modelID = comp.enttToModel_.at(enttID);
		modelToEntts[modelID].insert(enttID);
	}

	// prepare memory
	const size numEntts  = std::ssize(enttsIDs);
	const size numModels = std::ssize(modelToEntts);

	outModelsIDs.resize(numModels);
	outNumInstancesPerModel.resize(numModels);
	outEnttsSortByModels.resize(numEntts);
	
	// get models IDs
	for (int i = 0; const auto& it : modelToEntts)
		outModelsIDs[i++] = it.first;

	// compute the num of instances per each model
	for (int i = 0; const auto& it : modelToEntts)
		outNumInstancesPerModel[i++] = static_cast<int>(std::ssize(it.second));

	// sort entts by models: copy sorted entts IDs by model into output array
	for (int i = 0, offset = 0; const auto & it : modelToEntts)
	{
		std::copy(it.second.begin(), it.second.end(), outEnttsSortByModels.begin() + offset);
		offset += outNumInstancesPerModel[i++];
	}
}

///////////////////////////////////////////////////////////


} // namespace ECS