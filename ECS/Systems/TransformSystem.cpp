// =================================================================================
// Filename:      TranformSystem.cpp
// Description:   implementation of the TransformSystem's functional
// 
// Created:       20.05.24
// =================================================================================
#include "TransformSystem.h"

#include "../Common/Assert.h"
#include "../Common/log.h"

#include "SaveLoad/TransformSysSerDeser.h"

#include <stdexcept>
#include <algorithm>

using namespace DirectX;
using namespace Utils;

namespace ECS
{

TransformSystem::TransformSystem(
	Transform* pTransform,
	WorldMatrix* pWorld) 
	:
	pTransform_(pTransform),
	pWorldMat_(pWorld)
{
	Assert::NotNullptr(pTransform, "ptr to the Transform component == nullptr");
	Assert::NotNullptr(pWorld, "ptr to the WorldMatrix component == nullptr");

	// add invalid data; this data is returned when we ask for wrong entity
	pTransform_->ids_.push_back(INVALID_ENTITY_ID);
	pTransform_->posAndUniformScale_.push_back(XMFLOAT4{ 0, 0, 0, 1 });
	pTransform_->dirQuats_.push_back(XMVECTOR{ 0, 0, 0, 1 });

	pWorldMat_->ids_.push_back(INVALID_ENTITY_ID);
	pWorldMat_->worlds_.push_back(DirectX::XMMatrixIdentity());
	pWorldMat_->invWorlds_.push_back(DirectX::XMMatrixIdentity()); // inverse world matrix
}

///////////////////////////////////////////////////////////

void TransformSystem::AddRecords(
	const std::vector<EntityID>& ids,
	const std::vector<XMFLOAT3>& positions,
	const std::vector<XMVECTOR>& dirQuats,      // direction quaternions
	const std::vector<float>& uniformScales)
{
	Assert::NotEmpty(ids.empty(), "array of entities IDs is empty");
	Assert::True(CheckArrSizesEqual(ids, positions), "count of entities and positions must be equal");
	Assert::True(CheckArrSizesEqual(ids, dirQuats), "count of entities and directions must be equal");
	Assert::True(CheckArrSizesEqual(ids, uniformScales), "count of entities and scales must be equal");

	AddRecordsToTransformComponent(ids, positions, dirQuats, uniformScales);
	AddRecordsToWorldMatrixComponent(ids, positions, dirQuats, uniformScales);
}

///////////////////////////////////////////////////////////

void TransformSystem::RemoveRecords(const std::vector<EntityID>& enttsIDs)
{
	assert("TODO: IMPLEMENT IT!" && 0);
}

///////////////////////////////////////////////////////////

void TransformSystem::Serialize(std::ofstream& fout, u32& offset)
{
	Transform& t = *pTransform_;

	TransformSysSerDeser::Serialize(
		fout,
		offset,
		static_cast<u32>(ComponentType::TransformComponent),    // data block marker
		t.ids_,
		t.posAndUniformScale_,
		t.dirQuats_);
}

///////////////////////////////////////////////////////////

void TransformSystem::Deserialize(std::ifstream& fin, const u32 offset)
{
	Transform& t = *pTransform_;

	TransformSysSerDeser::Deserialize(
		fin,
		offset,
		t.ids_,
		t.posAndUniformScale_,
		t.dirQuats_);

	// clear data of the World component and build world matrices 
	// from deserialized Transform component data
	pWorldMat_->ids_.clear();
	pWorldMat_->worlds_.clear();
	pWorldMat_->invWorlds_.clear();

	AddRecordsToWorldMatrixComponent(t.ids_, t.posAndUniformScale_,	t.dirQuats_);
}



// =================================================================================
//                           PUBLIC GETTERS API
// =================================================================================

const XMFLOAT3 TransformSystem::GetPositionByID(const EntityID id)
{
	const XMFLOAT4& pos = pTransform_->posAndUniformScale_[GetIdxByID(id)];
	return XMFLOAT3(pos.x, pos.y, pos.z);
}

///////////////////////////////////////////////////////////

const XMVECTOR TransformSystem::GetRotationQuatByID(const EntityID id)
{
	return pTransform_->dirQuats_[GetIdxByID(id)];
}

///////////////////////////////////////////////////////////

void TransformSystem::GetTransformByID(
	const EntityID id,
	XMFLOAT3& pos,
	XMVECTOR& dirQuat,
	float& uniformScale)
{
	const index idx      = GetIdxByID(id);
	const XMFLOAT4& data = pTransform_->posAndUniformScale_[idx];

	dirQuat      = pTransform_->dirQuats_[idx];
	pos          = { data.x, data.y, data.z };
	uniformScale = data.w;
}

///////////////////////////////////////////////////////////

void TransformSystem::GetPositionsAndRotations(
	const std::vector<EntityID>& ids,
	std::vector<XMFLOAT3>& positions,
	std::vector<XMVECTOR>& dirQuats)
{
	// get positions and rotations of input entts by its IDs

	Transform& comp = *pTransform_;

	// check if there are entities by such IDs
	bool exist = Utils::CheckValuesExistInSortedArr(comp.ids_, ids);
	Assert::True(exist, "some of input entt doesn't have the Transform component");

	// get enttities data indices into arrays inside the Transform component
	std::vector<index> idxs;
	Utils::GetIdxsInSortedArr(comp.ids_, ids, idxs);

	const size numEntts = std::ssize(ids);
	positions.resize(numEntts);
	dirQuats.resize(numEntts);

	// get entities positions
	for (int i = 0; const index idx : idxs)
	{
		const DirectX::XMFLOAT4& pos = comp.posAndUniformScale_[idx];
		positions[i++] = { pos.x, pos.y, pos.z };
	}

	// get entities direction quaternions
	for (int i = 0; const index idx : idxs)
		dirQuats[i++] = comp.dirQuats_[idx];
}

///////////////////////////////////////////////////////////

void TransformSystem::GetTransformDataOfEntts(
	const std::vector<EntityID>& enttsIDs,
	std::vector<index>& outDataIdxs,
	std::vector<XMFLOAT3>& outPositions,
	std::vector<XMVECTOR>& outDirQuats,      // direction quaternions
	std::vector<float>& outUniformScales)
{
	// get a component data by ID from the Transform component
	//
	// in:     arr of entts IDs which data we will get
	// 
	// out: 1. arr of data idxs to each entt
	//      2. arr of positions
	//      3. arr of NORMALIZED direction quaternions
	//      4. arr of uniform scales

	
	Transform& comp = *pTransform_;

	// check if there are entities by such IDs
	bool exist = Utils::CheckValuesExistInSortedArr(comp.ids_, enttsIDs);
	Assert::True(exist, "there is some entity which doesn't have the Transform component so we can't get its transform data");

	// get enttities data indices into arrays inside the Transform component
	Utils::GetIdxsInSortedArr(comp.ids_, enttsIDs, outDataIdxs);

	GetTransformDataByDataIdxs(outDataIdxs, outPositions, outDirQuats, outUniformScales);
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
	return pWorldMat_->worlds_[GetIdxByID(id)];
}

///////////////////////////////////////////////////////////

void TransformSystem::GetWorldMatricesOfEntts(
	const std::vector<EntityID>& enttsIDs,
	std::vector<DirectX::XMMATRIX>& outWorldMatrices)
{
	const WorldMatrix& comp = *pWorldMat_;
	std::vector<index> idxs;

	// get data idx by each entt ID 
	// and then get world matrices by these idxs
	Utils::GetIdxsInSortedArr(comp.ids_, enttsIDs, idxs);
	GetMatricesByIdxs(idxs, comp.worlds_, outWorldMatrices);
}

///////////////////////////////////////////////////////////

void TransformSystem::GetWorldMatricesOfEntts(
	const EntityID* ids,
	const size numEntts,
	std::vector<DirectX::XMMATRIX>& outWorlds)
{
	const WorldMatrix& comp = *pWorldMat_;
	std::vector<index> idxs;

	// get data idx by each entt ID 
	// and then get world matrices by these idxs
	Utils::GetIdxsInSortedArr(comp.ids_, ids, numEntts, idxs);
	GetMatricesByIdxs(idxs, comp.worlds_, outWorlds);
}

///////////////////////////////////////////////////////////

const DirectX::XMMATRIX& TransformSystem::GetInverseWorldMatrixOfEntt(const EntityID id)
{
	// return an inverse world matrix of entt by ID;
	// or return an Identity Matrix if there is no such entt by ID;
	const WorldMatrix& comp = *pWorldMat_;
	return comp.invWorlds_[Utils::GetIdxInSortedArr(comp.ids_, id)];
}

///////////////////////////////////////////////////////////

void TransformSystem::GetInverseWorldMatricesOfEntts(
	const std::vector<EntityID>& enttsIDs,
	std::vector<DirectX::XMMATRIX>& outInvWorlds)
{
	const WorldMatrix& comp = *pWorldMat_;
	std::vector<index> idxs;

	// get data idx by each entt ID 
	// and then get inverse world matrices by these idxs
	Utils::GetIdxsInSortedArr(comp.ids_, enttsIDs, idxs);
	GetMatricesByIdxs(idxs, pWorldMat_->invWorlds_, outInvWorlds);
}

///////////////////////////////////////////////////////////

void TransformSystem::GetMatricesByIdxs(
	const std::vector<index>& idxs,
	const std::vector<XMMATRIX>& inMatrices,
	std::vector<XMMATRIX>& outMatrices)
{
	// get matrices (world / inverse world / etc.) by input data idxs

	outMatrices.resize(std::ssize(idxs));

	for (int i = 0; const index idx : idxs)
		outMatrices[i++] = inMatrices[idx];
}



// =================================================================================
//                           PUBLIC SETTERS API
// =================================================================================

void TransformSystem::SetPositionByID(const EntityID id, const XMFLOAT3& pos)
{
	Transform& transformComp = *pTransform_;
	WorldMatrix& worldComp = *pWorldMat_;

	// if we have an entity by such ID
	if (Utils::BinarySearch(transformComp.ids_, id))
	{
		index idx = Utils::GetIdxInSortedArr(transformComp.ids_, id);
		const float uniformScale = transformComp.posAndUniformScale_[idx].w;

		transformComp.posAndUniformScale_[idx] = { pos.x, pos.y, pos.z, uniformScale };

		// recompute world matrix and inverse world matrix for this entity
		RecomputeWorldMatrixByIdx(idx);
		RecomputeInvWorldMatrixByIdx(idx);
	}
}

///////////////////////////////////////////////////////////

void TransformSystem::SetDirectionByID(const EntityID id, const XMVECTOR& dirQuat)
{
	Transform& transformComp = *pTransform_;
	WorldMatrix& worldComp = *pWorldMat_;

	// if we have an entity by such ID
	if (Utils::BinarySearch(transformComp.ids_, id))
	{
		index idx = Utils::GetIdxInSortedArr(transformComp.ids_, id);
		transformComp.dirQuats_[idx] = DirectX::XMQuaternionNormalize(dirQuat);

		// recompute world matrix and inverse world matrix for this entity
		RecomputeWorldMatrixByIdx(idx);
		RecomputeInvWorldMatrixByIdx(idx);
	}
}

///////////////////////////////////////////////////////////

void TransformSystem::SetUniScaleByID(const EntityID id, const float uniformScale)
{
	// set the uniform scale value for the entity by ID

	Transform& transformComp = *pTransform_;
	WorldMatrix& worldComp = *pWorldMat_;

	// if we have an entity by such ID
	if (Utils::BinarySearch(transformComp.ids_, id))
	{
		index idx = Utils::GetIdxInSortedArr(transformComp.ids_, id);
		const XMFLOAT4& pos = transformComp.posAndUniformScale_[idx];

		transformComp.posAndUniformScale_[idx] = { pos.x, pos.y, pos.z, uniformScale };

		// recompute world matrix and inverse world matrix for this entity
		RecomputeWorldMatrixByIdx(idx);
		RecomputeInvWorldMatrixByIdx(idx);
	}
}

///////////////////////////////////////////////////////////

void TransformSystem::SetTransformByID(
	const EntityID id,
	const XMVECTOR& newPosition,
	const XMVECTOR& newRotation,
	const float newScale)
{
	Transform& transformComp = *pTransform_;
	WorldMatrix& worldComp   = *pWorldMat_;

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
	bool idsValid = Utils::CheckValuesExistInSortedArr(comp.ids_, enttsIDs);
	Assert::True(idsValid, "can't set data: not existed record by some id");

	const size enttsCount = std::ssize(enttsIDs);
	Assert::NotZero(enttsCount, "entities IDs arr is empty");
	Assert::True(enttsCount == newPositions.size(), "arr size of entts IDs and positions are not equal");
	Assert::True(enttsCount == newDirQuats.size(), "arr size of entts IDs and directions are not equal");
	Assert::True(enttsCount == newUniformScales.size(), "arr size of entts IDs and scales are not equal");

	// get enttities data indices into arrays inside the Transform component
	std::vector<index> dataIdxs;
	Utils::GetIdxsInSortedArr(comp.ids_, enttsIDs, dataIdxs);

	SetTransformDataByDataIdxs(dataIdxs, newPositions, newDirQuats, newUniformScales);
}

///////////////////////////////////////////////////////////

void TransformSystem::SetTransformDataByDataIdxs(
	const std::vector<index>& dataIdxs,
	const std::vector<XMVECTOR>& newPositions,
	const std::vector<XMVECTOR>& newDirQuats,
	const std::vector<float>& newUniformScales)
{
	const index idxsCount = std::ssize(dataIdxs);
	Assert::NotZero(idxsCount, "data idxs arr is empty");
	Assert::True(idxsCount == newPositions.size(), "arr of idxs and arr of positions are not equal");
	Assert::True(idxsCount == newDirQuats.size(), "arr of idxs and arr of directions are not equal");
	Assert::True(idxsCount == newUniformScales.size(), "arr of idxs and arr of scales are not equal");

	Transform& comp = *pTransform_;

	// set new positions by idxs
	for (index posIdx = 0; index dataIdx : dataIdxs)
		DirectX::XMStoreFloat4(&comp.posAndUniformScale_[dataIdx], newPositions[posIdx]);

	// set new uniform scales by idxs
	for (index scaleIdx = 0; index dataIdx : dataIdxs)
		comp.posAndUniformScale_[dataIdx].w = newUniformScales[scaleIdx++];

	// the Transform component stores only normalized direction quaternions so just do it
	for (index quatIdx = 0; index dataIdx : dataIdxs)
		comp.dirQuats_[dataIdx] = DirectX::XMQuaternionNormalize(newDirQuats[quatIdx]);
}

///////////////////////////////////////////////////////////

void TransformSystem::SetWorldMatricesByIDs(
	const std::vector<EntityID>& enttsIDs,
	const std::vector<XMMATRIX>& newWorldMatrices)
{
	// set new world matrix for each input entity by its ID

	Assert::NotEmpty(enttsIDs.empty(), "entities IDs arr is empty");
	Assert::True(enttsIDs.size() == newWorldMatrices.size(), "array size of entts IDs and new world matrices are not equal");

	// check if there are entities by such IDs
	WorldMatrix& comp = *pWorldMat_;
	bool idsValid = Utils::CheckValuesExistInSortedArr(comp.ids_, enttsIDs);
	Assert::True(idsValid, "can't set data: not existed record by some id");


	std::vector<index> idxs;
	idxs.reserve(std::ssize(enttsIDs));

	// get data idx of each entt ID
	for (const EntityID& id : enttsIDs)
		idxs.push_back(Utils::GetIdxInSortedArr(comp.ids_, id));

	SetWorldMatricesByDataIdxs(idxs, newWorldMatrices);
}

///////////////////////////////////////////////////////////

void TransformSystem::SetWorldMatricesByDataIdxs(
	const std::vector<index>& dataIdxs,
	const std::vector<XMMATRIX>& newWorldMatrices)
{
	// store world matrices by input data idxs

	Assert::True(pWorldMat_->worlds_.size() >= newWorldMatrices.size(), "count of new matrices can't be bigger than the number of matrices in the WorldMatrix component");

	for (index newMatIdx = 0; const index idx : dataIdxs)
		pWorldMat_->worlds_[idx] = newWorldMatrices[newMatIdx++];
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
	// store transformation data of entities into the Transform component

	Transform& component = *pTransform_;

	bool canAddComponent = !Utils::CheckValuesExistInSortedArr(component.ids_, ids);
	Assert::True(canAddComponent, "can't add component: there is already a record with some entity id");

	// ---------------------------------------------

	// normalize all the input direction quaternions
	std::vector<XMVECTOR> normDirQuats;
	normDirQuats.reserve(std::ssize(dirQuats));

	for (const XMVECTOR& quat : dirQuats)
		normDirQuats.emplace_back(DirectX::XMQuaternionNormalize(quat));

	// ---------------------------------------------

	for (u32 idx = 0; const EntityID& id : ids)
	{
		const index insertAt = GetPosForID(component.ids_, id);
		const XMFLOAT3& pos = positions[idx];
		const XMFLOAT4 transAndUniScale = { pos.x, pos.y, pos.z, uniformScales[idx] };
		
		// NOTE: we build a single XMFLOAT4 from position and uniform scale
		InsertAtPos<EntityID>(component.ids_, insertAt, id);
		InsertAtPos<XMFLOAT4>(component.posAndUniformScale_, insertAt, transAndUniScale);
		InsertAtPos<XMVECTOR>(component.dirQuats_, insertAt, normDirQuats[idx]);
	
		++idx;
	}
}

///////////////////////////////////////////////////////////

void TransformSystem::AddRecordsToWorldMatrixComponent(
	const std::vector<EntityID>& ids,
	const std::vector<XMFLOAT3>& pos,
	const std::vector<XMVECTOR>& dirQuats,      // direction quaternions
	const std::vector<float>& uniformScales)
{
	// compute and store world matrices into the WorldMatrix component
	
	WorldMatrix& comp = *pWorldMat_;

	bool canAddComponent = !Utils::CheckValuesExistInSortedArr(comp.ids_, ids);
	Assert::True(canAddComponent, "can't add component: there is already a record with some entity id");

	// ---------------------------------------------

	DirectX::XMVECTOR detWorld;                      // determinant
	std::vector<XMMATRIX> worlds;
	std::vector<XMMATRIX> invWorlds;                 // inverse world matrices
	const size worldMatricesCount = std::ssize(ids);


	worlds.reserve(worldMatricesCount);
	invWorlds.reserve(worldMatricesCount);

	// compute world matrices
	for (index i = 0; i < worldMatricesCount; ++i)
	{
		worlds.emplace_back(
			XMMatrixScaling(uniformScales[i], uniformScales[i], uniformScales[i]) *
			XMMatrixRotationQuaternion(dirQuats[i]) *
			XMMatrixTranslation(pos[i].x, pos[i].y, pos[i].z)
		);
	}

	// compute inverse world matrices
	for (index i = 0; i < worldMatricesCount; ++i)
		invWorlds.emplace_back(DirectX::XMMatrixInverse(&detWorld, worlds[i]));

	// ---------------------------------------------
		
	// store records into the WorldMatrix componemt	
	for (int i = 0; const EntityID id : ids)
	{
		const index insertByIdx = Utils::GetPosForID(comp.ids_, id);

		Utils::InsertAtPos(comp.ids_, insertByIdx, id);
		Utils::InsertAtPos(comp.worlds_, insertByIdx, worlds[i]);
		Utils::InsertAtPos(comp.invWorlds_, insertByIdx, invWorlds[i]);

		++i;
	}
}

///////////////////////////////////////////////////////////

void TransformSystem::AddRecordsToWorldMatrixComponent(
	const std::vector<EntityID>& ids,
	const std::vector<XMFLOAT4>& posAndUniScales,
	const std::vector<XMVECTOR>& dirQuats)     // direction quaternions
{
	// compute and store world matrices into the WorldMatrix component

	WorldMatrix& comp = *pWorldMat_;
	const size worldMatricesCount = std::ssize(posAndUniScales);

	std::vector<XMMATRIX> scaleMatrices;
	std::vector<XMMATRIX> rotationMatrices;
	std::vector<XMMATRIX> translationMatrices;
	std::vector<XMMATRIX> worldMatrices;
	std::vector<index> dataIdxs;

	scaleMatrices.reserve(worldMatricesCount);
	rotationMatrices.reserve(worldMatricesCount);
	translationMatrices.reserve(worldMatricesCount);
	worldMatrices.reserve(worldMatricesCount);
	dataIdxs.reserve(worldMatricesCount);

	// 1. compute matrices for uniform scaling (we use only w component of XMFLOAT4)
	// 2. compute rotation matrices
	// 3. compute translation matrices (we use x,y,z of XMFLOAT4)
	for (index idx = 0; idx < worldMatricesCount; ++idx)
		scaleMatrices.emplace_back(DirectX::XMMatrixScaling(posAndUniScales[idx].w, posAndUniScales[idx].w, posAndUniScales[idx].w));

	for (index idx = 0; idx < worldMatricesCount; ++idx)
		rotationMatrices.emplace_back(DirectX::XMMatrixRotationQuaternion(dirQuats[idx]));

	for (index idx = 0; idx < worldMatricesCount; ++idx)
		translationMatrices.emplace_back(DirectX::XMMatrixTranslation(posAndUniScales[idx].x, posAndUniScales[idx].y, posAndUniScales[idx].z));

	// compute world matrices
	for (index idx = 0; idx < worldMatricesCount; ++idx)
		worldMatrices.emplace_back(scaleMatrices[idx] * rotationMatrices[idx] * translationMatrices[idx]);

	// store records ['entt_id' => 'world_matrix'] into the WorldMatrix componemt
	for (u32 data_idx = 0; const EntityID id : ids)
	{
		const index insertAt = Utils::GetPosForID(comp.ids_, id);

		Utils::InsertAtPos(comp.ids_, insertAt, id);
		Utils::InsertAtPos(comp.worlds_, insertAt, worldMatrices[data_idx++]);
	}
}

}