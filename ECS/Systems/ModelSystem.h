// **********************************************************************************
// Filename:      ModelSystem.h
// Description:   Entity-Component-System (ECS) system for control 
//                model data of entities
// 
// Created:       21.05.24
// **********************************************************************************
#pragma once

#include "../Components/Model.h"
#include <fstream>

namespace ECS
{


class ModelSystem final
{
public:
	ModelSystem(Model* pModelComponent);
	~ModelSystem() {}

	void Serialize(std::ofstream& fout, u32& offset);
	void Deserialize(std::ifstream& fin, const u32 offset);

    void AddRecords(
        const EntityID* enttsIDs,
        const ModelID modelID,
        const size numEntts);

	void RemoveRecords(const std::vector<EntityID>& enttsIDs);

	ModelID GetModelIdRelatedToEntt(const EntityID enttID);

	void GetModelsIdsRelatedToEntts(
		const std::vector<EntityID>& enttsIDs,
		std::vector<ModelID>& outModelsIDs,
		std::vector<EntityID>& outEnttsSortByModels,
		std::vector<size>& outNumInstancesPerModel);

    void GetAllEntts(const EntityID*& ids, size& numEntts);

private:
	Model* pModelComponent_ = nullptr;
};

}
