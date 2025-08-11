// =================================================================================
// Filename:      ModelSystem.h
// Description:   Entity-Component-System (ECS) system for control 
//                model data of entities
// 
// Created:       21.05.24
// =================================================================================
#pragma once

#include "../Components/Model.h"


namespace ECS
{

class ModelSystem
{
public:
    ModelSystem(Model* pModelComponent);
    ~ModelSystem() {}

    void AddRecords(
        const EntityID* enttsIDs,
        const ModelID modelID,
        const size numEntts);

    void RemoveRecords(const EntityID* ids, const size numEntts);

    ModelID GetModelIdRelatedToEntt(const EntityID enttID);

    void GetModelsIdsRelatedToEntts(
        const EntityID* enttsIDs,
        const size numEntts,
        cvector<ModelID>& outModelsIDs,
        cvector<EntityID>& outEnttsSortByModels,
        cvector<size>& outNumInstancesPerModel);

    void GetAllEntts(const EntityID*& ids, size& numEntts);

private:
    Model* pModelComponent_ = nullptr;
};

} // namespace ECS
