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
	TransformSystem(Transform* pTransform);
	~TransformSystem();

	// public serialization / deserialization API
	void Serialize(std::ofstream& fout, u32& offset);
	void Deserialize(std::ifstream& fin, const u32 offset);

    void AddRecords(
        const EntityID* ids,
        const XMFLOAT3* positions,
        const XMVECTOR* dirQuats,      // direction quaternions
        const float* uniformScales,
        const size numElems);

	void RemoveRecords(const cvector<EntityID>& enttsIDs);

	// -------------------------------------------------------

	// GET/SET  position/direction/uniform_scale

    void GetPositionsByIDs      (const EntityID* ids, const size numEntts, cvector<XMFLOAT3>& outPositions) const;
    void GetDirectionsQuatsByIDs(const EntityID* ids, const size numEntts, cvector<XMVECTOR>& outDirQuats) const;
    void GetUniformScalesByIDs  (const EntityID* ids, const size numEntts, cvector<float>& outScales) const;

    void GetPositionsAndDirectionsByIDs(
        const EntityID* ids,
        const size numEntts,
        cvector<XMFLOAT3>& outPositions,
        cvector<XMFLOAT3>& outDirections);

	const XMFLOAT3 GetPositionByID     (const EntityID id) const;
	const XMVECTOR GetDirectionQuatByID(const EntityID id) const;
	const float    GetUniformScaleByID (const EntityID id) const;

	bool SetPositionByID     (const EntityID id, const XMFLOAT3& position);
	bool SetDirectionQuatByID(const EntityID id, const XMVECTOR& dirQuat);
	bool SetUniScaleByID     (const EntityID id, const float uniformScale);

    bool RotateWorldByQuat(const EntityID id, const XMVECTOR& quat);

	// -------------------------------------------------------

	void GetTransformByID(
		const EntityID id,
		XMFLOAT3& pos,
		XMVECTOR& dirQuat,
		float& uniformScale);

	DirectX::XMMATRIX GetWorldMatrixOfEntt(const EntityID id);

    void GetWorldMatricesOfEntts(
        const EntityID* ids,
        const size numEntts,
        cvector<DirectX::XMMATRIX>& outWorlds);

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
		const XMVECTOR& newDirectionQuat,
		const float newScale);

    void SetTransformDataByIdxs(
        const cvector<index>& idxs,
        const cvector<XMVECTOR>& newPositions,
        const cvector<XMVECTOR>& newDirQuats,
        const cvector<float>& newUniformScales);

	void SetWorldMatricesByIdxs(
		const cvector<index>& dataIdxs,
		const cvector<XMMATRIX>& newWorldMatrices);


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
		const index idx = pTransform_->ids.get_idx(id);

		if (pTransform_->ids[idx] != id)
		{
			Log::Error("there is no transform data for entt by id: " + std::to_string(id));
			return 0;
		}

		return idx;
	}

	inline void RecomputeWorldMatrixByIdx(const index idx)
	{
		// recompute world matrix for the entity by array idx
		pTransform_->worlds[idx] =
			GetMatScaling(pTransform_->posAndUniformScale[idx].w) *
			GetMatRotation(pTransform_->dirQuats[idx]) *
			GetMatTranslation(pTransform_->posAndUniformScale[idx]);
	}

	inline void RecomputeInvWorldMatrixByIdx(const index idx)
	{
		// recompute inverse world matrix based on world by array idx;
		// NOTE: expects the world matrix to be computed already!!!
		pTransform_->invWorlds[idx] = DirectX::XMMatrixInverse(nullptr, pTransform_->worlds[idx]);
	}

	inline XMMATRIX GetMatTranslation(const XMFLOAT4& pos)  const { return DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z); }
	inline XMMATRIX GetMatRotation(const XMVECTOR& dirQuat) const { return DirectX::XMMatrixRotationQuaternion(dirQuat); }
	inline XMMATRIX GetMatScaling(const float uScale)       const { return DirectX::XMMatrixScaling(uScale, uScale, uScale); }

private:
	Transform* pTransform_ = nullptr;   // a ptr to the Transform component
};

}
