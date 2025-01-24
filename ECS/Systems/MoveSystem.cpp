// **********************************************************************************
// Filename:      MoveSystem.h
// Description:   implementation of the MoveSystem's functional
// 
// Created:       23.05.24
// **********************************************************************************
#include "MoveSystem.h"

#include "../Common/LIB_Exception.h"
#include "../Common/log.h"
#include "../Common/Utils.h"
#include "./Helpers/MoveSystemUpdateHelpers.h"
#include "SaveLoad/MoveSysSerDeser.h"

#include <stdexcept>

namespace ECS
{
 

MoveSystem::MoveSystem(
	Transform* pTransformComponent,
	WorldMatrix* pWorldMatrixComponent,
	Movement* pMoveComponent)
{
	Assert::NotNullptr(pTransformComponent, "ptr to the Transform component == nullptr");
	Assert::NotNullptr(pWorldMatrixComponent, "ptr to the WorldMatrix component == nullptr");
	Assert::NotNullptr(pMoveComponent, "ptr to the Movement component == nullptr");

	pTransformComponent_ = pTransformComponent;
	pWorldMatComponent_ = pWorldMatrixComponent;
	pMoveComponent_ = pMoveComponent;
}


// ********************************************************************************
// 
//                PUBLIC SERIALIZATION / DESERIALIZATION API
// 
// ********************************************************************************

void MoveSystem::Serialize(std::ofstream& fout, u32& offset)
{
	const Movement& move = *pMoveComponent_;

	MoveSysSerDeser::Serialize(
		fout,
		offset,
		static_cast<u32>(move.type_),   // data block marker
		move.ids_,
		move.translationAndUniScales_,
		move.rotationQuats_);
}

///////////////////////////////////////////////////////////

void MoveSystem::Deserialize(std::ifstream& fin, const u32 offset)
{
	Movement& move = *pMoveComponent_;

	MoveSysSerDeser::Deserialize(
		fin,
		offset,
		move.ids_,
		move.translationAndUniScales_,
		move.rotationQuats_);
}



// ********************************************************************************
// 
//                              PUBLIC UPDATING API
// 
// ********************************************************************************

void MoveSystem::UpdateAllMoves(
	const float deltaTime,
	TransformSystem& transformSys)
{
	const std::vector<EntityID>& enttsToMove = pMoveComponent_->ids_;

	// if we don't have any entities to move we just go out
	if (enttsToMove.size() == 0)
		return;

	try
	{
		Transform& transform = *pTransformComponent_;
		Movement& movement = *pMoveComponent_;

		std::vector<XMFLOAT3> positions;
		std::vector<XMVECTOR> dirQuats;
		std::vector<float> uniformScales;

		std::vector<XMMATRIX> worldMatricesToUpdate;
		std::vector<ptrdiff_t> transformDataIdxs;

		// current transform data of entities as XMVECTORs
		std::vector<XMVECTOR> positionsVec;
		std::vector<XMVECTOR> scalesVec;

		// current movement data (scaled according to the deltaTime)
		std::vector<XMVECTOR> translationsVec;
		std::vector<XMVECTOR> rotQuatsVec;        
		std::vector<float> uniformScaleFactors;

		// get entities transform data to update for this frame
		transformSys.GetTransformDataOfEntts(
			enttsToMove,
			transformDataIdxs,
			positions, 
			dirQuats, 
			uniformScales);

		PrepareTransformData(
			positions,
			positionsVec);

		PrepareMovementData( 
			deltaTime,
			movement.translationAndUniScales_,  // (x: trans_x, y: trans_y, z: trans_z, w: uniform_scale)
			movement.rotationQuats_,
			translationsVec,
			rotQuatsVec,
			uniformScaleFactors);

		// compute new values of transform data using the movement data
		ComputeTransformData(
			deltaTime,
			translationsVec,
			rotQuatsVec,
			uniformScaleFactors,
			positionsVec,
			dirQuats, 
			uniformScales);

		// write updated transform data into the Transform component
		transformSys.SetTransformDataByDataIdxs(
			transformDataIdxs, 
			positionsVec, 
			dirQuats, 
			uniformScales);

		// ------------------------------------------------------

		// get world matrices which will be updated according to new transform data;
		transformSys.GetMatricesByIdxs(
			transformDataIdxs,
			transformSys.GetAllWorldMatrices(),
			worldMatricesToUpdate);

		// rebuild world matrices of that entities which were moved
		ComputeWorldMatrices(
			translationsVec, 
			rotQuatsVec, 
			uniformScaleFactors,
			worldMatricesToUpdate);

		// apply updated world matrices
		transformSys.SetWorldMatricesByDataIdxs(
			transformDataIdxs, 
			worldMatricesToUpdate);
	}
	catch (const std::out_of_range& e)
	{
		Log::Error(e.what());
		throw LIB_Exception("Went out of range during movement updating");
	}
}



// ********************************************************************************
// 
//                      PUBLIC CREATION / DELETING API
// 
// ********************************************************************************

void MoveSystem::AddRecords(
	const std::vector<EntityID>& ids,
	const std::vector<XMFLOAT3>& translations,
	const std::vector<XMVECTOR>& rotationQuats,
	const std::vector<float>& uniformScaleChanges)
{
	using namespace Utils;
	Assert::NotEmpty(ids.empty(), "array of entities IDs is empty");
	Assert::True(CheckArrSizesEqual(ids, translations), "count of entities and translations must be equal");
	Assert::True(CheckArrSizesEqual(ids, rotationQuats), "count of entities and rotationQuats must be equal");
	Assert::True(CheckArrSizesEqual(ids, uniformScaleChanges), "count of entities and scaleFactors must be equal");

	Movement& comp = *pMoveComponent_;


	// check if there is no record with such entities IDs
	bool canAddRecords = CheckValuesExistInSortedArr(comp.ids_, ids);
	Assert::True(canAddRecords, "there is already some record with some input ID (key)");

	const int numNewRecords = (int)std::ssize(ids);
	std::vector<XMFLOAT4> packedTrScales(numNewRecords);
	std::vector<XMVECTOR> normRotQuats(numNewRecords);


	// pack translation and uniformScale into a single XMFLOAT4
	for (int i = 0; i < numNewRecords; ++i)
	{
		packedTrScales[i].x = translations[i].x;
		packedTrScales[i].y = translations[i].y;
		packedTrScales[i].z = translations[i].z;
		packedTrScales[i].w = uniformScaleChanges[i];
	}

	// normalize each input rotation quaternion
	for (int i = 0; i < numNewRecords; ++i)
		normRotQuats[i] = DirectX::XMQuaternionNormalize(rotationQuats[i]);


	// execute sorted insertion into the data arrays
	for (int i = 0; i < numNewRecords; ++i)
	{	
		const index insertAtPos = Utils::GetPosForID(comp.ids_, ids[i]);

		InsertAtPos(comp.ids_, insertAtPos, ids[i]);
		InsertAtPos(comp.translationAndUniScales_, insertAtPos, packedTrScales[i]);
		InsertAtPos(comp.rotationQuats_, insertAtPos, normRotQuats[i]);
	}
}

///////////////////////////////////////////////////////////

void MoveSystem::RemoveRecords(const std::vector<EntityID>& enttsIDs)
{
	throw LIB_Exception("TODO: IMPLEMENT IT!");
}

}