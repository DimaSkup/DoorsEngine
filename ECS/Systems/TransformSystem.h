// **********************************************************************************
// Filename:      TranformSystem.h
// Description:   Entity-Component-System (ECS) system for handling 
//                transform data of entities
// 
// Created:       20.05.24
// **********************************************************************************
#pragma once

#include "../Common/log.h"
#include "../Components/Transform.h"

#include <fstream>

namespace ECS
{

class TransformSystem final
{
public:
	TransformSystem();
	~TransformSystem();

	void Initialize(Transform* pTransform);

	// public serialization / deserialization API
	void Serialize(std::ofstream& fout, u32& offset);
	void Deserialize(std::ifstream& fin, const u32 offset);

    void AddRecords(
        const EntityID* ids,
        const XMFLOAT3* positions,
        const XMVECTOR* dirQuats,      // direction quaternions
        const float* uniformScales,
        const size numElems);

	void RemoveRecords(const std::vector<EntityID>& enttsIDs);


	// -------------------------------------------------------

	// GET/SET  position/rotation/uniform_scale

	void GetPositionsByIDs(const EntityID* ids, XMFLOAT3* outPositions, const size numEntts) const;

	const XMFLOAT3 GetPositionByID            (const EntityID id) const;
	const XMVECTOR GetRotationQuatByID        (const EntityID id) const;
	const XMFLOAT3 GetRotationPitchYawRollByID(const EntityID id) const;
	const float    GetUniformScaleByID        (const EntityID id) const;

	bool SetPositionByID    (const EntityID id, const XMFLOAT3& position);
	bool SetRotationQuatByID(const EntityID id, const XMVECTOR& dirQuat);
	bool SetUniScaleByID    (const EntityID id, const float uniformScale);

	// -------------------------------------------------------

	void GetTransformByID(
		const EntityID id,
		XMFLOAT3& pos,
		XMVECTOR& dirQuat,
		float& uniformScale);

	DirectX::XMMATRIX GetWorldMatrixOfEntt(const EntityID id);

	void GetWorldMatricesOfEntts(
		const EntityID* ids,
		DirectX::XMMATRIX* outWorlds,
		const size numEntts);

	const DirectX::XMMATRIX& GetInverseWorldMatrixOfEntt(const EntityID id);

	void GetInverseWorldMatricesOfEntts(
		const EntityID* enttsIDs,
		DirectX::XMMATRIX* outInvWorlds,
		const int numEntts);

	void GetMatricesByIdxs(
		const index* idxs,
		const XMMATRIX* inMatrices,
		XMMATRIX* outMatrices,
		int numMatrices);

	void SetTransformByID(
		const EntityID id,
		const XMVECTOR& newPosition,
		const XMVECTOR& newRotation,
		const float newScale);

    void SetTransformDataByDataIdxs(
        const cvector<index>& idxs,
        const cvector<XMVECTOR>& newPositions,
        const cvector<XMVECTOR>& newDirQuats,
        const cvector<float>& newUniformScales);

	void SetWorldMatricesByDataIdxs(
		const std::vector<index>& dataIdxs,
		const std::vector<XMMATRIX>& newWorldMatrices);


private:
    void AddRecordsToTransformComponent(
        const EntityID* ids,
        const XMFLOAT3* positions,
        const XMVECTOR* dirQuats,      // direction quaternions
        const float* uniformScales,
        const size numElems);

	// ---------------------------------------------

	inline index GetIdxByID(const EntityID id) const
	{
		// return valid idx if there is an entity by such ID;
		// or return 0 if there is no such entity;
		const index idx = pTransform_->ids_.get_idx(id);

		if (pTransform_->ids_[idx] != id)
		{
			Log::Error("there is no transform data for entt by id: " + std::to_string(id));
			return 0;
		}

		return idx;
	}

	inline void RecomputeWorldMatrixByIdx(const index idx)
	{
		// recompute world matrix for the entity by array idx
		pTransform_->worlds_[idx] =
			GetMatScaling(pTransform_->posAndUniformScale_[idx].w) *
			GetMatRotation(pTransform_->dirQuats_[idx]) *
			GetMatTranslation(pTransform_->posAndUniformScale_[idx]);
	}

	inline void RecomputeInvWorldMatrixByIdx(const index idx)
	{
		// recompute inverse world matrix based on world by array idx;
		// NOTE: expects the world matrix to be computed already!!!
		pTransform_->invWorlds_[idx] = DirectX::XMMatrixInverse(nullptr, pTransform_->worlds_[idx]);
	}

	inline XMMATRIX GetMatTranslation(const XMFLOAT4& pos)  const { return DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z); }
	inline XMMATRIX GetMatRotation(const XMVECTOR& dirQuat) const { return DirectX::XMMatrixRotationQuaternion(dirQuat); }
	inline XMMATRIX GetMatScaling(const float uScale)       const { return DirectX::XMMatrixScaling(uScale, uScale, uScale); }

private:
	Transform* pTransform_ = nullptr;   // a ptr to the Transform component
	//WorldMatrix* pWorldMat_ = nullptr;  // a ptr to the WorldMatrix component
};

}
