// **********************************************************************************
// Filename:      BoundingSystem.h
// Description:   ECS system for handling bounding data of entts
// 
// Created:       26.09.24
// **********************************************************************************
#pragma once

#include "../Components/Bounding.h"

namespace ECS
{

class BoundingSystem final
{
public:
	BoundingSystem(Bounding* pBoundingComponent);
	~BoundingSystem() {}

	void Update(
		const EntityID* ids,
		const XMMATRIX* transforms,
		const size numEntts,
		const size numMatrices);

	void Add(                                   // add only one entt with only one subset (mesh)
		const EntityID id,
		const BoundingType type,
		const DirectX::BoundingBox& aabb);

	void Add(
		const EntityID* ids,
		const size numEntts,
		const size numSubsets,                  // it is supposed that each input entt has the same number of meshes (for instance: the same trees)
		const BoundingType* types,              // AABB type per mesh
		const DirectX::BoundingBox* AABBs);     // AABB per mesh
#if 0
	const BoundingData& GetBoundingDataByID(const EntityID id);

	void GetBoundingDataByIDs(
		const std::vector<EntityID>& ids,
		std::vector<BoundingData>& outData);
#endif

	void GetOBBs(
		const std::vector<EntityID>& ids,
		std::vector<size>& numBoxesPerEntt,
		std::vector<DirectX::BoundingOrientedBox>& outOBBs);

	void GetBoxLocalSpaceMatrices(
		const std::vector<EntityID>& ids,
		std::vector<size>& numBoxesPerEntt,
		std::vector<DirectX::XMMATRIX>& local);

	void GetBoxesLocalSpaceMatrices(
		const std::vector<DirectX::BoundingBox>& boundingBoxes,
		std::vector<DirectX::XMMATRIX>& outMatrices);

	void GetBoxLocalSpaceMatrix(
		const DirectX::BoundingBox& aabb,
		DirectX::XMMATRIX& mat);

	//inline const std::vector<BoundingData>& GetAllBoundingData() const { return pBoundingComponent_->data_; }

private:
	Bounding* pBoundingComponent_ = nullptr;
};



} // namespace ECS