#include "BoundingSystem.h"

#include "../Common/Assert.h"
#include "../Common/log.h"
#include "../Common/Utils.h"

#include <algorithm>

namespace ECS
{


BoundingSystem::BoundingSystem(Bounding* pBoundingComponent) :
	pBoundingComponent_(pBoundingComponent)
{
	Assert::NotNullptr(pBoundingComponent, "ptr to the bounding component == nullptr");
}

///////////////////////////////////////////////////////////

void BoundingSystem::Update(
	const EntityID* ids,
	const XMMATRIX* transforms,
	const size numEntts,
	const size numMatrices)
{
	// apply a transform matrix by idx to all the bounding boxes 
	// of entity by the same idx;

	Assert::True(ids && transforms && (numEntts > 0) && (numEntts == numMatrices), "wrong input data");

	Bounding& comp = *pBoundingComponent_;
	std::vector<index> idxs;
	Utils::GetIdxsInSortedArr(comp.ids_, ids, numEntts, idxs);

	for (index i = 0; i < numEntts; ++i)
	{
		BoundingData& data = comp.data_[idxs[i]];
		
		for (index boxIdx = 0; boxIdx < data.numData_; ++boxIdx)
		{
			data.obbs_[boxIdx].Transform(data.obbs_[boxIdx], transforms[i]);
		}
	}
}

///////////////////////////////////////////////////////////

void BoundingSystem::Add(
	const EntityID id,
	const BoundingType type,
	const DirectX::BoundingBox& aabb)
{
	// add only one entt with only one subset (mesh)
	Add(&id, 1, 1, &type, &aabb);
}

///////////////////////////////////////////////////////////

void BoundingSystem::Add(
	const EntityID* ids,
	const size numEntts,
	const size numSubsets,               // the number of entt's meshes (the num of AABBs)
	const BoundingType* types,           // AABB type per mesh
	const DirectX::BoundingBox* AABBs)   // AABB per mesh
{
	// add BOUNDING BOX for each mesh (subset) of the input entity;

	Assert::True((numEntts > 0) && (numSubsets > 0), "num of entts/subsets must be > 0");
	Assert::True(ids && types && AABBs, "wrong data arrays");

	// check if we already have a record with such ID
	Bounding& comp = *pBoundingComponent_;
	bool canAddComponent = !Utils::CheckValuesExistInSortedArr(comp.ids_, ids, numEntts);
	Assert::True(canAddComponent, "can't add component: there is already a record with some entity id");

	// ---------------------------------------------

	for (index i = 0; i < numEntts; ++i)
	{
		// execute sorted insertion of the data
		const index insertAt = Utils::GetPosForID(comp.ids_, ids[i]);

		Utils::InsertAtPos(comp.ids_, insertAt, ids[i]);
		Utils::InsertAtPos(comp.data_, insertAt, BoundingData(numSubsets, types, AABBs));
	}
}

///////////////////////////////////////////////////////////

void ComputeAABB(
	DirectX::BoundingOrientedBox* obbs,
	const int numOBBs,
	DirectX::BoundingBox& outAABB)
{
	// compute an AABB by array of OBBs

	using namespace DirectX;

	XMVECTOR vMin{ FLT_MAX, FLT_MAX, FLT_MAX };
	XMVECTOR vMax{ FLT_MIN, FLT_MIN, FLT_MIN };

	// go through each subset (mesh)
	for (int i = 0; i < numOBBs; ++i)
	{
		const DirectX::BoundingOrientedBox& subsetAABB = obbs[i];

		// define min/max point of this mesh
		const XMVECTOR center  = XMLoadFloat3(&subsetAABB.Center);
		const XMVECTOR extents = XMLoadFloat3(&subsetAABB.Extents);
		const XMVECTOR max = center + extents;
		const XMVECTOR min = center - extents;

		vMin = XMVectorMin(vMin, min);
		vMax = XMVectorMax(vMax, max);
	}

	// compute a model's AABB
	XMStoreFloat3(&outAABB.Center,  0.5f * (vMin + vMax));
	XMStoreFloat3(&outAABB.Extents, 0.5f * (vMax - vMin));
}

///////////////////////////////////////////////////////////

void BoundingSystem::GetEnttAABB(
	const EntityID id,
	DirectX::BoundingBox& aabb)
{
	// get an axis-aligned bounding box of entity by input ID
	// (AABB of the whole entity)

	Bounding&     comp = *pBoundingComponent_;
	BoundingData& data = comp.data_[GetIdxByID(id)];

	ComputeAABB(data.obbs_.data(), (int)data.obbs_.size(), aabb);
}

///////////////////////////////////////////////////////////

void BoundingSystem::GetOBBs(
	const std::vector<EntityID>& ids,
	std::vector<size>& numBoxesPerEntt,
	std::vector<DirectX::BoundingOrientedBox>& outOBBs)
{
	try
	{
		Bounding& comp = *pBoundingComponent_;
		const bool enttsValid = Utils::CheckValuesExistInSortedArr(comp.ids_, ids);
		Assert::True(enttsValid, "some of input entts doesn't have an AABB");

		std::vector<index> idxs;
		Utils::GetIdxsInSortedArr(comp.ids_, ids, idxs);

		size numEntts = std::ssize(ids);
		size numOBBs = 0;

		numBoxesPerEntt.resize(numEntts);

		// get the number of bounding boxes per each entity
		for (int i = 0; const index idx : idxs)
			numBoxesPerEntt[i++] = comp.data_[idx].numData_;

		// compute the number of all bounding boxes which we will get
		for (index i = 0; i < numEntts; ++i)
			numOBBs += numBoxesPerEntt[i];

		// get AABBs of each input entt
		outOBBs.reserve(numOBBs);

		// go through each entity data and copy each bounding box into the output arr
		for (int obbIdx = 0; const index idx : idxs)
			Utils::AppendToArray(outOBBs, comp.data_[idx].obbs_);
	}
	catch (LIB_Exception& e)
	{
		Log::Error(e);
		Log::Error("can't get bounding oriented boxes");
	}
}

///////////////////////////////////////////////////////////

void BoundingSystem::GetBoxLocalSpaceMatrices(
	const std::vector<EntityID>& ids,
	std::vector<size>& numBoxesPerEntt,
	std::vector<DirectX::XMMATRIX>& local)
{
	// in:   entity ID which will be used to get bounding boxes
	// out:  1. how many bounding boxes this entt has
	//       2. local space matrix of each bounding box

	Bounding& comp = *pBoundingComponent_;
	size numEntts = std::ssize(ids);
	size numOBBs = 0;
	std::vector<index> idxs;

	Utils::GetIdxsInSortedArr(comp.ids_, ids, idxs);
	numBoxesPerEntt.resize(numEntts);

	// get the number of bounding boxes per each entity
	for (int i = 0; const index idx : idxs)
		numBoxesPerEntt[i++] = comp.data_[idx].numData_;

	// compute the number of all bounding boxes which we will get
	for (index i = 0; i < numEntts; ++i)
		numOBBs += numBoxesPerEntt[i];
		

	// compute local world matrix for each bounding box
	local.resize(numOBBs);

	// go through each entt by idx and its each OBB
	for (int enttIdx = 0, obbIdx = 0; const index idx : idxs)
	{
		for (const DirectX::BoundingOrientedBox& obb : comp.data_[idx].obbs_)
		{
			XMVECTOR boxLScale    = XMLoadFloat3(&obb.Extents);
			XMVECTOR boxLRotQuat  = XMLoadFloat4(&obb.Orientation);
			XMVECTOR boxLPos      = XMLoadFloat3(&obb.Center);
			
			XMMATRIX boxLScaleMat = DirectX::XMMatrixScalingFromVector(boxLScale);
			XMMATRIX boxLRotMat   = DirectX::XMMatrixRotationQuaternion(boxLRotQuat);
			XMMATRIX boxLTransMat = DirectX::XMMatrixTranslationFromVector(boxLPos);

			// store local space matrix of this OBB
			local[obbIdx++] = boxLScaleMat * boxLRotMat * boxLTransMat;
		}
	}
}

///////////////////////////////////////////////////////////

void BoundingSystem::GetBoxesLocalSpaceMatrices(
	const std::vector<DirectX::BoundingBox>& boundingBoxes,
	std::vector<DirectX::XMMATRIX>& outMatrices)
{
	// make a local space matrix by bounding box params

	outMatrices.resize(std::ssize(boundingBoxes));

	for (int i = 0; const DirectX::BoundingBox& aabb : boundingBoxes)
	{
		XMVECTOR boxLScale = XMLoadFloat3(&aabb.Extents);
		XMVECTOR boxLPos   = XMLoadFloat3(&aabb.Center);

		XMMATRIX boxLScaleMat = DirectX::XMMatrixScalingFromVector(boxLScale);
		XMMATRIX boxLTransMat = DirectX::XMMatrixTranslationFromVector(boxLPos);

		// store local space matrix of this OBB
		outMatrices[i++] = boxLScaleMat * boxLTransMat;
	}
}

///////////////////////////////////////////////////////////

void BoundingSystem::GetBoxLocalSpaceMatrix(
	const DirectX::BoundingBox& aabb,
	DirectX::XMMATRIX& mat)
{
	// make a local space matrix by bounding box params

	XMVECTOR boxLScale = XMLoadFloat3(&aabb.Extents);
	//XMVECTOR boxLRotQuat = XMLoadFloat4(&obb.Orientation);
	XMVECTOR boxLPos = XMLoadFloat3(&aabb.Center);

	XMMATRIX boxLScaleMat = DirectX::XMMatrixScalingFromVector(boxLScale);
	//XMMATRIX boxLRotMat = DirectX::XMMatrixRotationQuaternion(boxLRotQuat);
	XMMATRIX boxLTransMat = DirectX::XMMatrixTranslationFromVector(boxLPos);

	// store local space matrix of this OBB
	mat = boxLScaleMat * boxLTransMat;
}
} // namespace ECS