// **********************************************************************************
// Filename:      TranformSystem.h
// Description:   Entity-Component-System (ECS) system for handling 
//                transform data of entities
// 
// Created:       20.05.24
// **********************************************************************************
#pragma once

#include "../Common/Utils.h"
#include "../Components/Transform.h"
#include "../Components/WorldMatrix.h"


#include <set>
#include <fstream>

namespace ECS
{

class TransformSystem final
{
public:
	TransformSystem(Transform* pTransform, WorldMatrix* pWorld);
	~TransformSystem() {}

	// public serialization / deserialization API
	void Serialize(std::ofstream& fout, u32& offset);
	void Deserialize(std::ifstream& fin, const u32 offset);

	void AddRecords(
		const std::vector<EntityID>& enttsIDs, 
		const std::vector<XMFLOAT3>& positions,
		const std::vector<XMVECTOR>& dirQuats,      // direction quaternions
		const std::vector<float>& uniformScales);

	void RemoveRecords(const std::vector<EntityID>& enttsIDs);


	// -------------------------------------------------------
	// PUBLIC GETTERS API

	const XMFLOAT3 GetPositionByID(const EntityID id);

	void GetTransformByID(
		const EntityID id,
		XMFLOAT3& pos,
		XMVECTOR& dirQuat,
		float& uniformScale);

	void GetPositionsAndRotations(
		const std::vector<EntityID>& ids,
		std::vector<XMFLOAT3>& positions,
		std::vector<XMVECTOR>& dirQuats);
	
	void GetTransformDataOfEntts(
		const std::vector<EntityID>& enttsIDs,
		std::vector<index>& outDataIdxs,
		std::vector<XMFLOAT3>& outPositions,
		std::vector<XMVECTOR>& outDirQuats,      // direction quaternions
		std::vector<float>& outUniformScales);

	void GetTransformDataByDataIdxs(
		std::vector<index>& dataIdxs,
		std::vector<XMFLOAT3>& outPositions,
		std::vector<XMVECTOR>& outDirQuats,      // direction quaternions
		std::vector<float>& outUniformScales);

	
	DirectX::XMMATRIX GetWorldMatrixOfEntt(const EntityID id);

	void GetWorldMatricesOfEntts(
		const std::vector<EntityID>& enttsIDs,
		std::vector<DirectX::XMMATRIX>& outWorlds);

	void GetWorldMatricesOfEntts(
		const EntityID* ids,
		const size numEntts,
		std::vector<DirectX::XMMATRIX>& outWorlds);

	void GetInverseWorldMatricesOfEntts(
		const std::vector<EntityID>& enttsIDs,
		std::vector<DirectX::XMMATRIX>& outInvWorlds);

	void GetMatricesByIdxs(
		const std::vector<index>& idxs,
		const std::vector<XMMATRIX>& inMatrices,
		std::vector<XMMATRIX>& outMatrices);

	// inline getters
	inline const std::vector<DirectX::XMMATRIX>& GetAllWorldMatrices() const { return pWorldMat_->worlds_; }
	inline const std::vector<EntityID>& GetAllEnttsIDsFromTransformComponent() const { return pTransform_->ids_; }
	inline void GetAllEnttsIDsFromWorldMatrixComponent(std::vector<EntityID>& outEnttsIDs) { outEnttsIDs = pWorldMat_->ids_; }


	// -------------------------------------------------------
	// PUBLIC SETTERS API

	void SetPositionByID(const EntityID id, const XMFLOAT3& position);
	void SetDirectionByID(const EntityID id, const XMVECTOR& dirQuat);
	void SetUniScaleByID(const EntityID id, const float uniformScale);

	void SetTransformDataByIDs(
		const std::vector<EntityID>& enttsIDs,
		const std::vector<XMVECTOR>& positions,
		const std::vector<XMVECTOR>& dirQuats,      // direction quaternions
		const std::vector<float>& uniformScales);

	void SetTransformDataByDataIdxs(
		const std::vector<index>& dataIdxs,
		const std::vector<XMVECTOR>& newPositions,
		const std::vector<XMVECTOR>& newDirQuats,
		const std::vector<float>& newUniformScales);

	void SetWorldMatricesByIDs(
		const std::vector<EntityID>& enttsIDs,
		const std::vector<XMMATRIX>& newWorldMatrices);

	void SetWorldMatricesByDataIdxs(
		const std::vector<index>& dataIdxs,
		const std::vector<XMMATRIX>& newWorldMatrices);


private:
	void AddRecordsToTransformComponent(
		const std::vector<EntityID>& enttsIDs,
		const std::vector<XMFLOAT3>& positions,
		const std::vector<XMVECTOR>& dirQuats,      // direction quaternions
		const std::vector<float>& uniformScales);

	void AddRecordsToWorldMatrixComponent(
		const std::vector<EntityID>& enttsIDs,
		const std::vector<XMFLOAT3>& positions,
		const std::vector<XMVECTOR>& dirQuats,      // direction quaternions
		const std::vector<float>& uniformScales);

	void AddRecordsToWorldMatrixComponent(
		const std::vector<EntityID>& enttsIDs,
		const std::vector<XMFLOAT4>& posAndUniformScales,
		const std::vector<XMVECTOR>& dirQuats);     // direction quaternions

	// ---------------------------------------------

	inline index GetIdxByID(const EntityID id)
	{
		// return valid idx if there is an entity by such ID;
		// or return 0 if there is no such entity;
		const std::vector<EntityID>& ids = pTransform_->ids_;
		return (Utils::BinarySearch(ids, id)) ? Utils::GetIdxInSortedArr(ids, id) : 0;
	}

	inline void RecomputeWorldMatrixByIdx(const index idx)
	{
		// recompute world matrix for the entity by array idx
		pWorldMat_->worlds_[idx] =
			GetMatScaling(pTransform_->posAndUniformScale_[idx].w) *
			GetMatRotation(pTransform_->dirQuats_[idx]) *
			GetMatTranslation(pTransform_->posAndUniformScale_[idx]);
	}

	inline void RecomputeInvWorldMatrixByIdx(const index idx)
	{
		// recompute inverse world matrix based on world by array idx
		pWorldMat_->invWorlds_[idx] = DirectX::XMMatrixInverse(nullptr, pWorldMat_->worlds_[idx]);
	}

	inline XMMATRIX GetMatTranslation(const XMFLOAT4& pos)  const { return DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z); }
	inline XMMATRIX GetMatRotation(const XMVECTOR& dirQuat) const { return DirectX::XMMatrixRotationQuaternion(dirQuat); }
	inline XMMATRIX GetMatScaling(const float uScale)       const { return DirectX::XMMatrixScaling(uScale, uScale, uScale); }

private:
	Transform* pTransform_ = nullptr;   // a ptr to the Transform component
	WorldMatrix* pWorldMat_ = nullptr;  // a ptr to the WorldMatrix component
};

}