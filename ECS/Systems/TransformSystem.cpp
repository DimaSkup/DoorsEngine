// =================================================================================
// Filename:      TranformSystem.cpp
// Description:   implementation of the TransformSystem's functional
// 
// Created:       20.05.24
// =================================================================================
#include "TransformSystem.h"

#include "../Common/Assert.h"
#include "../Common/MathHelper.h"
#include "SaveLoad/TransformSysSerDeser.h"

#include <stdexcept>
#include <algorithm>

using namespace DirectX;


namespace ECS
{

TransformSystem::TransformSystem()
{
}

TransformSystem::~TransformSystem()
{
}

///////////////////////////////////////////////////////////

void TransformSystem::Initialize(Transform* pTransform) 
{
	Assert::NotNullptr(pTransform, "ptr to the Transform component == nullptr");
	pTransform_ = pTransform;

	// add invalid data; this data is returned when we ask for wrong entity
	pTransform_->ids_.push_back(INVALID_ENTITY_ID);
	pTransform_->posAndUniformScale_.push_back(XMFLOAT4{ NAN, NAN, NAN, NAN });
	pTransform_->dirQuats_.push_back(XMVECTOR{ NAN, NAN, NAN, NAN });

	const std::vector<float> nanArray(16, NAN);
	XMMATRIX nanMatrix(nanArray.data());

	pTransform_->worlds_.push_back(nanMatrix);
	pTransform_->invWorlds_.push_back(nanMatrix); // inverse world matrix
}

///////////////////////////////////////////////////////////

void TransformSystem::AddRecords(
	const std::vector<EntityID>& ids,
	const std::vector<XMFLOAT3>& positions,
	const std::vector<XMVECTOR>& dirQuats,      // direction quaternions
	const std::vector<float>& uniformScales)
{
	Assert::NotEmpty(ids.empty(), "array of entities IDs is empty");
	Assert::True(Utils::CheckArrSizesEqual(ids, positions), "count of entities and positions must be equal");
	Assert::True(Utils::CheckArrSizesEqual(ids, dirQuats), "count of entities and directions must be equal");
	Assert::True(Utils::CheckArrSizesEqual(ids, uniformScales), "count of entities and scales must be equal");

	AddRecordsToTransformComponent(ids, positions, dirQuats, uniformScales);
}

///////////////////////////////////////////////////////////

void TransformSystem::RemoveRecords(const std::vector<EntityID>& enttsIDs)
{
	assert("TODO: IMPLEMENT IT!" && 0);
}

///////////////////////////////////////////////////////////

void TransformSystem::Serialize(std::ofstream& fout, u32& offset)
{
#if 0
	Transform& t = *pTransform_;

	TransformSysSerDeser::Serialize(
		fout,
		offset,
		static_cast<u32>(ComponentType::TransformComponent),    // data block marker
		t.ids_,
		t.posAndUniformScale_,
		t.dirQuats_);
#endif
}

///////////////////////////////////////////////////////////

void TransformSystem::Deserialize(std::ifstream& fin, const u32 offset)
{
#if 0
	Transform& t = *pTransform_;

	TransformSysSerDeser::Deserialize(
		fin,
		offset,
		t.ids_,
		t.posAndUniformScale_,
		t.dirQuats_);

	pTransform_->worlds_.clear();
	pTransform_->invWorlds_.clear();
#endif
}


// =================================================================================
// GET position/rotation/uniform_scale
// =================================================================================

void TransformSystem::GetPositionsByIDs(
	const EntityID* ids,
	XMFLOAT3* outPositions,
	const size numEntts) const
{
	// get positions of entities by input IDs array;
	// 
	// NOTE: it is supposed that ids.size() == outPositions.size()

	assert((ids != nullptr) && (outPositions != nullptr) && (numEntts > 0) && "invalid input arguments");

	Transform& comp = *pTransform_;
    cvector<index> idxs;

	comp.ids_.get_idxs(ids, numEntts, idxs);

	for (int i = 0; const index idx : idxs)
	{
		XMFLOAT4& data = comp.posAndUniformScale_[idx];   // pos (float3) + scale (float)
		outPositions[i++] = { data.x, data.y, data.z };
	}
}

///////////////////////////////////////////////////////////

const XMFLOAT3 TransformSystem::GetPositionByID(const EntityID id) const
{
	const XMFLOAT4& pos = pTransform_->posAndUniformScale_[GetIdxByID(id)];
	return XMFLOAT3(pos.x, pos.y, pos.z);
}

///////////////////////////////////////////////////////////

const XMVECTOR TransformSystem::GetRotationQuatByID(const EntityID id) const
{
	return pTransform_->dirQuats_[GetIdxByID(id)];
}

///////////////////////////////////////////////////////////

const XMFLOAT3 TransformSystem::GetRotationPitchYawRollByID(const EntityID id) const
{
	const index idx = GetIdxByID(id);

	if (idx == 0)
	{
		// if there is no transformation data for entity by ID
		return XMFLOAT3{ NAN, NAN, NAN };
	}
	else
	{
		// convert quaternion into Euler angles (in order: pitch, yaw, roll)
		return MathHelper::QuatToEulerAngles(pTransform_->dirQuats_[idx]);
	}
}

///////////////////////////////////////////////////////////

const float TransformSystem::GetUniformScaleByID(const EntityID id) const
{
	return pTransform_->posAndUniformScale_[GetIdxByID(id)].w;
}


// =================================================================================
// SET position/rotation/uniform_scale
// =================================================================================

bool TransformSystem::SetPositionByID(const EntityID id, const XMFLOAT3& pos)
{
	const index idx = GetIdxByID(id);

	// if there is no transformation data for entity by ID
	if (idx == 0) 
		return false;

	Transform& comp = *pTransform_;
	XMFLOAT4& data = comp.posAndUniformScale_[idx];

	// position is stored in x,y,z components (w-component stores the uniform scale)
	data.x = pos.x;
	data.y = pos.y;
	data.z = pos.z;

	// recompute world matrix and inverse world matrix for this entity
	RecomputeWorldMatrixByIdx(idx);
	RecomputeInvWorldMatrixByIdx(idx);

	return true;
}

///////////////////////////////////////////////////////////

bool TransformSystem::SetRotationQuatByID(const EntityID id, const XMVECTOR& dirQuat)
{
	// update rotation of entity using input rotation quaternion

	const index idx = GetIdxByID(id);

	// if there is no transformation data for entity by ID
	if (idx == 0)      
		return false;

	pTransform_->dirQuats_[idx] = XMQuaternionNormalize(dirQuat);

	// recompute world matrix and inverse world matrix for this entity
	RecomputeWorldMatrixByIdx(idx);
	RecomputeInvWorldMatrixByIdx(idx);

	return true;
}

///////////////////////////////////////////////////////////

bool TransformSystem::SetUniScaleByID(const EntityID id, const float uniformScale)
{
	// set the uniform scale value for the entity by ID

	const index idx = GetIdxByID(id);

	// if there is no transformation data for entity by ID
	if (idx == 0)
		return false;

	// uniform scale is stored in the w-component
	pTransform_->posAndUniformScale_[idx].w = uniformScale;

	// recompute world matrix and inverse world matrix for this entity
	RecomputeWorldMatrixByIdx(idx);
	RecomputeInvWorldMatrixByIdx(idx);

	return true;
}


// =================================================================================
// Get/Set transformation
// =================================================================================

void TransformSystem::GetTransformByID(
	const EntityID id,
	XMFLOAT3& pos,
	XMVECTOR& dirQuat,
	float& uniformScale)
{
	const index idx = GetIdxByID(id);
	const XMFLOAT4& data = pTransform_->posAndUniformScale_[idx];

	dirQuat = pTransform_->dirQuats_[idx];
	pos = { data.x, data.y, data.z };
	uniformScale = data.w;
}

///////////////////////////////////////////////////////////

void TransformSystem::GetTransformDataByDataIdxs(
	std::vector<index>& dataIdxs,
	std::vector<XMFLOAT3>& outPositions,
	std::vector<XMVECTOR>& outDirQuats,      // direction quaternions
	std::vector<float>& outUniformScales)
{
	Transform& comp = *pTransform_;

	const size numElems = std::ssize(dataIdxs);

	outPositions.reserve(numElems);
	outDirQuats.reserve(numElems);
	outUniformScales.reserve(numElems);

	// get entities positions
	for (const index idx : dataIdxs)
	{
		const DirectX::XMFLOAT4& pos = comp.posAndUniformScale_[idx];
		outPositions.emplace_back(pos.x, pos.y, pos.z);
	}

	// get uniform scales (each value is stored in the w-component of XMFLOAT4)
	for (const index idx : dataIdxs)
		outUniformScales.push_back(comp.posAndUniformScale_[idx].w);

	// get entities direction quaternions
	for (const index idx : dataIdxs)
		outDirQuats.push_back(comp.dirQuats_[idx]);
}

///////////////////////////////////////////////////////////

DirectX::XMMATRIX TransformSystem::GetWorldMatrixOfEntt(const EntityID id)
{
	// return a world matrix of entt by ID or return a matrix of NANs if there is no such entt by ID
	return pTransform_->worlds_[GetIdxByID(id)];
}

///////////////////////////////////////////////////////////

const DirectX::XMMATRIX& TransformSystem::GetInverseWorldMatrixOfEntt(const EntityID id)
{
	// return an inverse world matrix of entt by ID or return a matrix of NANs if there is no such entt by ID
	return pTransform_->invWorlds_[GetIdxByID(id)];
}

///////////////////////////////////////////////////////////

void TransformSystem::GetWorldMatricesOfEntts(
	const EntityID* ids,
	DirectX::XMMATRIX* outWorlds,
	const size numEntts)
{
	// NOTE: size of arrays ids and outWorlds must be equal !!!
	Assert::True((ids != nullptr) && (outWorlds != nullptr) && (numEntts > 0), "invalid input arguments");

	const Transform& comp = *pTransform_;
    cvector<index> idxs(numEntts);
    cvector<DirectX::XMMATRIX> worlds(numEntts);

	// get data idx by each ID and then get world matrices by these idxs
	comp.ids_.get_idxs(ids, numEntts, idxs);
	comp.worlds_.get_data_by_idxs(idxs, worlds);
	memcpy(outWorlds, worlds.data(), worlds.size() * sizeof(XMMATRIX));
}

///////////////////////////////////////////////////////////

void TransformSystem::GetInverseWorldMatricesOfEntts(
	const EntityID* ids,
	DirectX::XMMATRIX* outInvWorlds,
	const int numEntts)
{
	// NOTE: size of arrays enttsIDs and outInvWorlds must be equal !!!
	Assert::True((ids != nullptr) && (outInvWorlds != nullptr) && (numEntts > 0), "invalid input data");

	const Transform& comp = *pTransform_;
    cvector<index> idxs(numEntts);
    cvector<XMMATRIX> invWorlds(numEntts);

	// get data idx by each ID and then get inverse world matrices by these idxs
	comp.ids_.get_idxs(ids, numEntts, idxs);
	comp.invWorlds_.get_data_by_idxs(idxs, invWorlds);
	memcpy(outInvWorlds, invWorlds.data(), invWorlds.size() * sizeof(XMMATRIX));
}

///////////////////////////////////////////////////////////

void TransformSystem::GetMatricesByIdxs(
	const index* idxs,
	const XMMATRIX* inMatrices,
	XMMATRIX* outMatrices,
	int numMatrices)
{
	// get matrices (world / inverse world / etc.) by input data idxs;
	// out: array of chosen matrices

	for (int i = 0; i < numMatrices; ++i)
		outMatrices[i++] = inMatrices[idxs[i]];
}

// =================================================================================

void TransformSystem::SetTransformByID(
	const EntityID id,
	const XMVECTOR& newPosition,
	const XMVECTOR& newRotation,
	const float newScale)
{
	Transform& transformComp = *pTransform_;
	//WorldMatrix& worldComp   = *pWorldMat_;

	index idx = GetIdxByID(id);
	XMFLOAT4& posAndScale = transformComp.posAndUniformScale_[idx];

	DirectX::XMStoreFloat4(&posAndScale, newPosition);  // xyz - translation
	posAndScale.w = newScale;                           // w   - contains a uniform scale value
	transformComp.dirQuats_[idx] = newRotation;

	// recompute world matrix and inverse world matrix for this entity
	RecomputeWorldMatrixByIdx(idx);
	RecomputeInvWorldMatrixByIdx(idx);
}

///////////////////////////////////////////////////////////

void TransformSystem::SetTransformDataByIDs(
	const std::vector<EntityID>& enttsIDs,
	const std::vector<XMVECTOR>& newPositions,
	const std::vector<XMVECTOR>& newDirQuats,      // direction quaternions
	const std::vector<float>& newUniformScales)
{
	// setup position, direction, scale for all the input entities by its IDs;
	// NOTE: input data has XMFLOAT3 type so we can write it directly

	Transform& comp = *pTransform_;

	// check if there are entities by such IDs
	//bool idsValid = Utils::CheckValuesExistInSortedArr(comp.ids_, enttsIDs);
	//Assert::True(idsValid, "can't set data: not existed record by some id");

	const size enttsCount = std::ssize(enttsIDs);
	Assert::NotZero(enttsCount, "entities IDs arr is empty");
	Assert::True(enttsCount == newPositions.size(), "arr size of entts IDs and positions are not equal");
	Assert::True(enttsCount == newDirQuats.size(), "arr size of entts IDs and directions are not equal");
	Assert::True(enttsCount == newUniformScales.size(), "arr size of entts IDs and scales are not equal");

	// get enttities data indices into arrays inside the Transform component
    cvector<index> idxs;
	comp.ids_.get_idxs(enttsIDs.data(), enttsIDs.size(), idxs);
	SetTransformDataByDataIdxs(idxs, newPositions, newDirQuats, newUniformScales);
}

///////////////////////////////////////////////////////////

void TransformSystem::SetTransformDataByDataIdxs(
	const cvector<index>& idxs,
	const std::vector<XMVECTOR>& newPositions,
	const std::vector<XMVECTOR>& newDirQuats,
	const std::vector<float>& newUniformScales)
{
	const size numElems = idxs.size();
	Assert::NotZero(numElems, "data idxs arr is empty");
	Assert::True(numElems == newPositions.size(), "arr of idxs and arr of positions are not equal");
	Assert::True(numElems == newDirQuats.size(), "arr of idxs and arr of directions are not equal");
	Assert::True(numElems == newUniformScales.size(), "arr of idxs and arr of scales are not equal");

	Transform& comp = *pTransform_;

	// set new positions by idxs
	for (index posIdx = 0; index idx : idxs)
		DirectX::XMStoreFloat4(&comp.posAndUniformScale_[idx], newPositions[posIdx]);

	// set new uniform scales by idxs
	for (index scaleIdx = 0; index idx : idxs)
		comp.posAndUniformScale_[idx].w = newUniformScales[scaleIdx++];

	// the Transform component stores only normalized direction quaternions so just do it
	for (index quatIdx = 0; index idx : idxs)
		comp.dirQuats_[idx] = DirectX::XMQuaternionNormalize(newDirQuats[quatIdx]);
}

///////////////////////////////////////////////////////////

void TransformSystem::SetWorldMatricesByDataIdxs(
	const std::vector<index>& dataIdxs,
	const std::vector<XMMATRIX>& newWorldMatrices)
{
	// store world matrices by input data idxs

	Assert::True(pTransform_->worlds_.size() >= std::ssize(newWorldMatrices), "count of new matrices can't be bigger than the number of matrices in the component");

	for (index newMatIdx = 0; const index idx : dataIdxs)
		pTransform_->worlds_[idx] = newWorldMatrices[newMatIdx++];
}


// =================================================================================
//                            PRIVATE HELPERS
// =================================================================================

void TransformSystem::AddRecordsToTransformComponent(
	const std::vector<EntityID>& ids,
	const std::vector<XMFLOAT3>& positions,
	const std::vector<XMVECTOR>& dirQuats,      // direction quaternions
	const std::vector<float>& uniformScales)
{
	// store transformation data of input entities into the Transform component

	Transform& comp = *pTransform_;

	const size numElems = std::ssize(ids);
	bool canAddComponent = !comp.ids_.binary_search(ids.data(), numElems);
	Assert::True(canAddComponent, "can't add component: there is already a record with some entity id");


	// normalize all the input direction quaternions
	std::vector<XMVECTOR> normDirQuats(numElems);

	for (int idx = 0; const XMVECTOR& quat : dirQuats)
		normDirQuats[idx++] = DirectX::XMQuaternionNormalize(quat);


	// store input data into the component
	for (int idx = 0; idx < (int)numElems; ++idx)
	{
		const index insertAt = comp.ids_.get_insert_idx(ids[idx]);
		const XMFLOAT3& pos  = positions[idx];
		const float scale    = uniformScales[idx];

		// NOTE: we build a single XMFLOAT4 from position(float3) and uniform scale(float)
		comp.ids_.insert_before(insertAt, ids[idx]);
		comp.posAndUniformScale_.insert_before(insertAt, { pos.x, pos.y, pos.z, scale });
		comp.dirQuats_.insert_before(insertAt, normDirQuats[idx]);

		// compute a world matrix and store it
		// and also compute an inverse world matrix and store it as well
		XMMATRIX world = XMMatrixScaling(scale, scale, scale) * XMMatrixRotationQuaternion(dirQuats[idx]) * XMMatrixTranslation(pos.x, pos.y, pos.z);

		comp.worlds_.insert_before(insertAt, world);
		comp.invWorlds_.insert_before(insertAt, XMMatrixInverse(nullptr, world));
	}
}

}
