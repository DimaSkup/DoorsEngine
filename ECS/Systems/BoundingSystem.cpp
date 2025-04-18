#include "../Common/pch.h"
#include "BoundingSystem.h"

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
    cvector<index> idxs;

    comp.ids.get_idxs(ids, numEntts, idxs);

    for (index i = 0; i < numEntts; ++i)
    {
        BoundingData& data = comp.data[idxs[i]];
        
        for (index boxIdx = 0; boxIdx < data.numData; ++boxIdx)
        {
            data.obbs[boxIdx].Transform(data.obbs[boxIdx], transforms[i]);
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
    bool canAddComponent = !comp.ids.binary_search(ids, numEntts);
    Assert::True(canAddComponent, "can't add component: there is already a record with some entity id");

    // ---------------------------------------------

    cvector<index> idxs;
    comp.ids.get_insert_idxs(ids, numEntts, idxs);

    // exec indices correction
    for (index i = 0; i < numEntts; ++i)
        idxs[i] += i;

    // execute sorted insertion of input values
    for (index i = 0; i < numEntts; ++i)
        comp.ids.insert_before(idxs[i], ids[i]);

    for (index i = 0; i < numEntts; ++i)
        comp.data.insert_before(idxs[i], BoundingData(numSubsets, types, AABBs));
}

///////////////////////////////////////////////////////////

void BoundingSystem::Add(
    const EntityID* ids,
    const size numEntts,
    const DirectX::BoundingSphere* spheres)
{
    assert(0 && "IMPLEMENT IT FOR SPHERES");
    Assert::True((ids != nullptr) && (spheres != nullptr) && (numEntts > 0), "input data is invalid");
}

///////////////////////////////////////////////////////////

void ComputeAABB(
    const DirectX::BoundingOrientedBox* obbs,
    const size numOBBs,
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
        const XMVECTOR max     = center + extents;
        const XMVECTOR min     = center - extents;

        vMin = XMVectorMin(vMin, min);
        vMax = XMVectorMax(vMax, max);
    }

    // compute a model's AABB
    XMStoreFloat3(&outAABB.Center,  0.5f * (vMin + vMax));
    XMStoreFloat3(&outAABB.Extents, 0.5f * (vMax - vMin));
}

///////////////////////////////////////////////////////////

void BoundingSystem::GetEnttAABB(const EntityID id, DirectX::BoundingBox& outAABB)
{
    // get an axis-aligned bounding box of entity by input ID
    // (AABB of the whole entity)

    Bounding&     comp = *pBoundingComponent_;
    BoundingData& data = comp.data[GetIdxByID(id)];

    ComputeAABB(data.obbs.data(), data.obbs.size(), outAABB);
}

///////////////////////////////////////////////////////////

void BoundingSystem::GetOBBs(
    const EntityID* ids,
    const size numEntts,
    cvector<size>& outNumBoxesPerEntt,
    cvector<DirectX::BoundingOrientedBox>& outOBBs)
{
    Assert::True((ids != nullptr) && (numEntts > 0), "can't get OBBs: invalid input args");

    Bounding& comp = *pBoundingComponent_;
    size numOBBs = 0;
    cvector<index> idxs;


    // get indices to responsible data by IDs
    comp.ids.get_idxs(ids, numEntts, idxs);

    // get the number of bounding boxes per each entity
    outNumBoxesPerEntt.resize(numEntts);

    for (int i = 0; const index idx : idxs)
        outNumBoxesPerEntt[i++] = comp.data[idx].numData;

    // compute the number of all bounding boxes which we will get
    for (index i = 0; i < numEntts; ++i)
        numOBBs += outNumBoxesPerEntt[i];


    // get oriented bounding boxes for each mesh of each entity
    outOBBs.resize(numOBBs);

    for (int obbIdx = 0; const index idx : idxs)
    {
        for (DirectX::BoundingOrientedBox& obb : comp.data[idx].obbs)
            outOBBs[obbIdx++] = obb;
    }
}

///////////////////////////////////////////////////////////

void BoundingSystem::GetBoxLocalSpaceMatricesByIDs(
    const EntityID* ids,
    const size numEntts,
    cvector<size>& outNumBoxesPerEntt,
    cvector<DirectX::XMMATRIX>& outLocalMatrices)
{
    // in:   entity ID which will be used to get bounding boxes
    // out:  1. how many bounding boxes this entt has
    //       2. local space matrix of each bounding box

    Assert::True((ids != nullptr) && (numEntts > 0), "can't get local space matrices: invalid input args");

    Bounding& comp = *pBoundingComponent_;
    size numOBBs = 0;
    cvector<index> idxs;


    comp.ids.get_idxs(ids, numEntts, idxs);

    // get the number of bounding boxes per each entity
    outNumBoxesPerEntt.resize(numEntts);

    for (int i = 0; const index idx : idxs)
        outNumBoxesPerEntt[i++] = comp.data[idx].numData;

    // compute the number of all bounding boxes which we will get
    for (index i = 0; i < numEntts; ++i)
        numOBBs += outNumBoxesPerEntt[i];


    // compute local world matrix for each bounding box
    outLocalMatrices.resize(numOBBs);

    for (int enttIdx = 0, obbIdx = 0; const index idx : idxs)
    {
        for (const DirectX::BoundingOrientedBox& obb : comp.data[idx].obbs)
        {
            const XMVECTOR boxLScale    = XMLoadFloat3(&obb.Extents);
            const XMVECTOR boxLRotQuat  = XMLoadFloat4(&obb.Orientation);
            const XMVECTOR boxLPos      = XMLoadFloat3(&obb.Center);
            
            const XMMATRIX boxLScaleMat = DirectX::XMMatrixScalingFromVector(boxLScale);
            const XMMATRIX boxLRotMat   = DirectX::XMMatrixRotationQuaternion(boxLRotQuat);
            const XMMATRIX boxLTransMat = DirectX::XMMatrixTranslationFromVector(boxLPos);

            // store local space matrix of this OBB
            outLocalMatrices[obbIdx++] = boxLScaleMat * boxLRotMat * boxLTransMat;
        }
    }
}

///////////////////////////////////////////////////////////

void BoundingSystem::GetBoxesLocalSpaceMatrices(
    const DirectX::BoundingBox* boundingBoxes,
    const size numBoundingBoxes,
    cvector<DirectX::XMMATRIX>& outMatrices)
{
    // make a local space matrices by input bounding boxes params

    Assert::True((boundingBoxes != nullptr) && (numBoundingBoxes > 0), "invalid input args");

    outMatrices.resize(numBoundingBoxes);

    for (index i = 0; i < numBoundingBoxes; ++i)
    {
        const DirectX::BoundingBox& aabb = boundingBoxes[i];
        const XMVECTOR boxLScale    = XMLoadFloat3(&aabb.Extents);
        const XMVECTOR boxLPos      = XMLoadFloat3(&aabb.Center);

        const XMMATRIX boxLScaleMat = DirectX::XMMatrixScalingFromVector(boxLScale);
        const XMMATRIX boxLTransMat = DirectX::XMMatrixTranslationFromVector(boxLPos);

        // store local space matrix of this OBB
        outMatrices[i++] = boxLScaleMat * boxLTransMat;
    }
}

///////////////////////////////////////////////////////////

void BoundingSystem::GetBoxLocalSpaceMatrix(
    const DirectX::BoundingBox& aabb,
    DirectX::XMMATRIX& mat)
{
    // make a local space matrix by input bounding box params

    const XMVECTOR boxLScale = XMLoadFloat3(&aabb.Extents);
    const XMVECTOR boxLPos   = XMLoadFloat3(&aabb.Center);

    // Local_space = Scale * Transform
    mat = DirectX::XMMatrixScalingFromVector(boxLScale) * DirectX::XMMatrixTranslationFromVector(boxLPos);
}

} // namespace ECS
