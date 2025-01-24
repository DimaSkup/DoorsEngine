// **********************************************************************************
// Filename:      ModelSystem.h
// Description:   Entity-Component-System (ECS) system for control 
//                model data of entities
// 
// Created:       21.05.24
// **********************************************************************************
#pragma once

#include "../Components/Model.h"
#include <vector>
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


	// many to one: entts => model
	void AddRecords(
		const std::vector<EntityID>& enttsIDs,
		const ModelID modelID);

	// one to one: entt => model
	void AddRecords(
		const std::vector<EntityID>& enttsIDs,
		const std::vector<ModelID>& modelsIDs);

	void RemoveRecords(const std::vector<EntityID>& enttsIDs);

	ModelID GetModelIdRelatedToEntt(const EntityID enttID);

	void GetModelsIdsRelatedToEntts(
		const std::vector<EntityID>& enttsIDs,
		std::vector<ModelID>& outModelsIDs,
		std::vector<EntityID>& outEnttsSortByModels,
		std::vector<int>& outNumInstancesPerModel);

private:
	Model* pModelComponent_ = nullptr;
};

}