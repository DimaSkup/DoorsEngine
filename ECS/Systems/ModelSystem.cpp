// =================================================================================
// Filename:   ModelSystem.cpp
// =================================================================================
#include "../Common/pch.h"
#include "ModelSystem.h"

namespace ECS
{

// static cvectors for transient data
static cvector<index> s_Idxs;

//---------------------------------------------------------

ModelSystem::ModelSystem(Model* pModelComponent) : pModelComponent_(pModelComponent)
{
    CAssert::NotNullptr(pModelComponent, "ptr to the Model component == nullptr");
}

//---------------------------------------------------------
// Desc:   make relations one to one: 'entity_id' => 'model_id'
//         NOTE: enttsIDs are supposed to be SORTED!
//---------------------------------------------------------
void ModelSystem::AddRecords(
    const EntityID* enttsIDs,
    const ModelID modelID,
    const size numEntts)
{
    CAssert::True((enttsIDs != nullptr) && (numEntts > 0), "invalid input args");

    Model& comp = *pModelComponent_;
    const vsize newCapacity = comp.enttsIDs_.size() + numEntts;

    comp.enttsIDs_.get_insert_idxs(enttsIDs, numEntts, s_Idxs);
    comp.enttsIDs_.reserve(newCapacity);

    // sort insert of entities IDs (primary keys)
    for (index i = 0; i < numEntts; ++i)
        comp.enttsIDs_.insert_before(s_Idxs[i] + i, enttsIDs[i]);

    // insert of model ID
    for (index i = 0; i < numEntts; ++i)
        comp.modelIDs_.insert_before(s_Idxs[i] + i, modelID);
}

///////////////////////////////////////////////////////////

void ModelSystem::RemoveRecords(const EntityID* ids, const size numEntts)
{
    CAssert::True(false, "TODO: IMPLEMENT IT!");
}

//---------------------------------------------------------
// return a model ID by related input entt ID
//---------------------------------------------------------
ModelID ModelSystem::GetModelIdRelatedToEntt(const EntityID enttID)
{

    Model& comp = *pModelComponent_;

    // 1. check if we have such entity ID;
    // 2. get idx by value (or get 0 if there is no such)
    const bool has = comp.enttsIDs_.binary_search(enttID);
    const index idx = comp.enttsIDs_.get_idx(enttID) * has;

    return comp.modelIDs_[idx];
}

//---------------------------------------------------------
// in:  array of entities IDs
//
// out: array of related models IDs (1 model Id per one entt)
//---------------------------------------------------------
void ModelSystem::GetModelsIdsPerEntts(
    const EntityID* enttsIds,
    const size numEntts,
    cvector<ModelID>& outModelsIds)
{
    if (!enttsIds)
    {
        LogErr(LOG, "input ptr to entts ids arr == nullptr");
        return;
    }

    outModelsIds.clear();
    outModelsIds.resize(numEntts);

    const Model& comp = *pModelComponent_;
    comp.enttsIDs_.get_idxs(enttsIds, numEntts, s_Idxs);

    for (index i = 0; i < numEntts; ++i)
    {
        outModelsIds[i] = comp.modelIDs_[s_Idxs[i]];
    }
}

//---------------------------------------------------------
// in: array of entts IDs
// 
// out: 1) arr of models which are related to the input entities
//      2) arr of entts sorted by its models
//      3) arr of entts number per model
//---------------------------------------------------------
void ModelSystem::GetModelsIdsRelatedToEntts(
    const EntityID* enttsIDs,                       
    const size numEntts,                            
    cvector<ModelID>& outModelsIDs,
    cvector<EntityID>& outEnttsSortByModels,
    cvector<size>& outNumInstancesPerModel)
{

    CAssert::True(enttsIDs != nullptr, "input ptr to entities IDs arr == nullptr");
    CAssert::True(numEntts > 0,        "input number of entities must be > 0");

    const Model& comp = *pModelComponent_;
    std::map<ModelID, cvector<EntityID>> modelToEntts;

    comp.enttsIDs_.get_idxs(enttsIDs, numEntts, s_Idxs);

    // get related models IDs and use them as keys
    // and group entities by these models IDs
    for (int i = 0; i < numEntts; ++i)
    {
        const ModelID modelID = comp.modelIDs_[s_Idxs[i]];
        const EntityID enttID = comp.enttsIDs_[s_Idxs[i]];
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
