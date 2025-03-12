#include "ModelSystem.h"

#include "../Common/Assert.h"
#include "../Common/log.h"
//#include "SaveLoad/ModelSysSerDeser.h"
#include <map>


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
    const EntityID* enttsIDs,
    const ModelID modelID,
    const size numEntts)
{
    // make relations one to one: 'entity_id' => 'model_id'
    // NOTE: enttsIDs are supposed to be SORTED!

    Assert::True((enttsIDs != nullptr) && (numEntts > 0), "invalid input args");

    Model& comp = *pModelComponent_;
    cvector<index> idxs;
    const vsize newCapacity = comp.enttsIDs_.size() + numEntts;

    comp.enttsIDs_.get_insert_idxs(enttsIDs, numEntts, idxs);
    comp.enttsIDs_.reserve(newCapacity);

    // sort insert of entities IDs (primary keys)
    for (index i = 0; i < numEntts; ++i)
        comp.enttsIDs_.insert_before(idxs[i] + i, enttsIDs[i]);

    // insert of model ID
    for (index i = 0; i < numEntts; ++i)
        comp.modelIDs_.insert_before(idxs[i] + i, modelID);
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

    Model& comp = *pModelComponent_;

    // 1. check if we have such entity ID;
    // 2. get idx by value (or get 0 if there is no such)
    const bool has = comp.enttsIDs_.binary_search(enttID);
    const index idx = comp.enttsIDs_.get_idx(enttID) * has;

    return comp.modelIDs_[idx];
}

///////////////////////////////////////////////////////////

void ModelSystem::GetModelsIdsRelatedToEntts(
    const std::vector<EntityID>& enttsIDs,         // by these entts we will get models
    std::vector<ModelID>& outModelsIDs,            // models by these IDs will be rendered for this frame
    std::vector<EntityID>& outEnttsSortByModels,
    std::vector<size>& outNumInstancesPerModel)
{
    // in: array of entts IDs
    // 
    // out: 1) arr of models which are related to the input entities
    //      2) arr of entts sorted by its models
    //      3) arr of entts number per model
    
    cvector<index> idxs;
    const Model& comp = *pModelComponent_;
    const size numEntts = std::ssize(enttsIDs);
    std::map<ModelID, cvector<EntityID>> modelToEntts;

    comp.enttsIDs_.get_idxs(enttsIDs.data(), std::ssize(enttsIDs), idxs);

    // get related models IDs and use them as keys
    // and group entities by these models IDs
    for (int i = 0; i < numEntts; ++i)
    {
        const ModelID modelID = comp.modelIDs_[idxs[i]];
        const EntityID enttID = comp.enttsIDs_[idxs[i]];
        modelToEntts[modelID].push_back(enttID);
    }

    // prepare memory
    const size numModels = std::ssize(modelToEntts);

    outModelsIDs.resize(numModels);
    outNumInstancesPerModel.resize(numModels);
    outEnttsSortByModels.resize(numEntts);

    // get models IDs
    for (int i = 0; const auto & it : modelToEntts)
        outModelsIDs[i++] = it.first;

    // compute the num of instances per each model
    for (int i = 0; const auto & it : modelToEntts)
        outNumInstancesPerModel[i++] = std::ssize(it.second);

    // sort entts by models: copy sorted entts IDs by model into output array
    for (index i = 0, offset = 0; const auto & it : modelToEntts)
    {
        std::copy(it.second.begin(), it.second.end(), outEnttsSortByModels.begin() + offset);
        offset += outNumInstancesPerModel[i++];
    }
}

///////////////////////////////////////////////////////////

void ModelSystem::GetAllEntts(const EntityID*& ids, size& numEntts)
{
    Model& comp = *pModelComponent_;

    ids      = comp.enttsIDs_.data();
    numEntts = comp.enttsIDs_.size();
}


} // namespace ECS
