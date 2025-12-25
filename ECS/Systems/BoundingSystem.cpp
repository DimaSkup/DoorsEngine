/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: BoundingSystem
    Desc:     ECS system to handle bounding shapes of entities

    Created:  26.09.2024  by DimaSkup
\**********************************************************************************/
#include "../Common/pch.h"
#include "BoundingSystem.h"


namespace ECS
{

using namespace DirectX;

// static arrays for internal purposes
static cvector<index> s_Idxs;


// functions prototypes
BoundingSphere CreateSphereFromBoxes(const BoundingBox* AABBs, const size count);
BoundingSphere CreateSphereFromBox  (const BoundingBox& aabb);
BoundingBox    CreateBoxFromSphere  (const BoundingSphere& sphere);
BoundingBox    ComputeAABB          (const BoundingBox* AABBs, const size count);

///////////////////////////////////////////////////////////

BoundingSystem::BoundingSystem(Bounding* pBoundingComponent) :
    pBoundingComponent_(pBoundingComponent)
{
    CAssert::NotNullptr(pBoundingComponent, "ptr to the bounding component == nullptr");

    Bounding& comp = *pBoundingComponent;

    // alloc memory ahead
    comp.ids.reserve(256);
    comp.data.reserve(256);

    // add "invalid" bounding shape
    comp.ids.push_back(INVALID_ENTITY_ID);
    comp.data.push_back(BoundingData());
}

//---------------------------------------------------------
// Desc:  add only one entt with only one bounding SPHERE
//---------------------------------------------------------
bool BoundingSystem::Add(const EntityID id, const BoundingSphere& sphere)
{
    Bounding& comp = *pBoundingComponent_;

    // check if we already have a record with such ID
    if (comp.ids.binary_search(id))
    {
        LogErr(LOG, "there is already a record with entity: %" PRIu32, id);
        return false;
    }

    const index          idx = comp.ids.get_insert_idx(id);
    const BoundingBox    aabb = CreateBoxFromSphere(sphere);

    // execute sorted insertion of ID and initial data
    comp.data.insert_before(idx, BoundingData(BOUND_SPHERE, sphere, aabb));

    return true;
}

//---------------------------------------------------------
// Desc:  add only one entt with only one axis-aligned bounding box (AABB)
//---------------------------------------------------------
bool BoundingSystem::Add(const EntityID id, const BoundingBox& aabb)
{
    Bounding& comp = *pBoundingComponent_;

    // check if we already have a record with such ID
    if (comp.ids.binary_search(id))
    {
        LogErr(LOG, "there is already a record with entity: %" PRIu32, id);
        return false;
    }

    const index          idx      = comp.ids.get_insert_idx(id);
    const BoundingSphere sphere   = CreateSphereFromBox(aabb);

    // execute sorted insertion of ID and initial data
    comp.ids.insert_before(idx, id);
    comp.data.insert_before(idx, BoundingData(BOUND_SPHERE, sphere, aabb));

    return true;
}

//---------------------------------------------------------
// Desc:  add the same BOUNDING BOX for each input entity;
// 
// Args:  - ids:        array of entities IDs
//        - numEntts:   how many entities in the input arr
//        - aabb:       axis-aligned bounding box
//---------------------------------------------------------
bool BoundingSystem::Add(
    const EntityID* ids,
    const size numEntts,
    const DirectX::BoundingBox& aabb)
{
    if (!ids)
    {
        LogErr(LOG, "input arr of entities IDs == nullptr");
        return false;
    }
    if (numEntts <= 0)
    {
        LogErr(LOG, "input number of entities can't be <= 0");
        return false;
    }

    // check that there is no records with input IDs yet
    Bounding& comp = *pBoundingComponent_;
    if (comp.ids.binary_search(ids, numEntts))
    {
        LogErr(LOG, "there is already a record with some input ID");
        return false;
    }


    const BoundingSphere sphere = CreateSphereFromBox(aabb);
    const BoundingData initData = { BOUND_BOX, sphere, aabb };

    comp.ids.get_insert_idxs(ids, numEntts, s_Idxs);

    // sorted insertion of IDs and initial data
    for (index i = 0; i < numEntts; ++i)
        comp.ids.insert_before(s_Idxs[i] + i, ids[i]);

    for (index i = 0; i < numEntts; ++i)
        comp.data.insert_before(s_Idxs[i] + i, initData);

    return true;
}

//--------------------------------------------------------
// Desc:  get bounding sphere for each input entity so we will be able
//        to execute basic frustum culling test using these sphere
//--------------------------------------------------------
void BoundingSystem::GetBoundSpheres(
    const EntityID* ids,
    const size numEntts,
    cvector<BoundingSphere>& outSpheres)
{
    if (!ids)
    {
        LogErr(LOG, "input ptr to ids arr == nullptr");
        return;
    }
    if (numEntts <= 0)
    {
        LogErr(LOG, "input number of entts can't be <= 0");
        return;
    }

    Bounding& comp = *pBoundingComponent_;

    comp.ids.get_idxs(ids, numEntts, s_Idxs);
    outSpheres.resize(numEntts);

    for (int i = 0; const index idx : s_Idxs)
        outSpheres[i++] = comp.data[idx].sphere;
}

//--------------------------------------------------------
// Desc:  get an axis-aligned bounding box around entity by input ID
//--------------------------------------------------------
const BoundingBox& BoundingSystem::GetAABB(const EntityID id) const
{
    return pBoundingComponent_->data[GetIdxByID(id)].aabb;
}

//--------------------------------------------------------
// Desc:  get a bounding spehre around entity by input ID
//--------------------------------------------------------
const BoundingSphere& BoundingSystem::GetBoundSphere(const EntityID id) const
{
    return pBoundingComponent_->data[GetIdxByID(id)].sphere;
}



//**********************************************************************************
// PRIVATE INTERNAL HELPERS
//**********************************************************************************

//--------------------------------------------------------
// Desc:  compute a bounding sphere from input arr of axis-aligned bounding boxes
//--------------------------------------------------------
BoundingSphere CreateSphereFromBoxes(const BoundingBox* AABBs, const size count)
{
    if (!AABBs || count <= 0)
    {
        LogErr(LOG, "invalid input args; return default bound sphere (center: <0,0,0>, radius: 1.0");
        return BoundingSphere({ 0,0,0 }, 1);
    }

    return CreateSphereFromBox(ComputeAABB(AABBs, count));
}

//--------------------------------------------------------
// Desc:  compute a bounding sphere from input axis-aligned bounding box
//--------------------------------------------------------
BoundingSphere CreateSphereFromBox(const BoundingBox& aabb)
{
    return BoundingSphere {
        aabb.Center,
        sqrtf(SQR(aabb.Extents.x) + SQR(aabb.Extents.y) + SQR(aabb.Extents.z))
    };
}

//--------------------------------------------------------
// Desc:  compute a axis-aligned bounding box from input bounding sphere
//--------------------------------------------------------
BoundingBox CreateBoxFromSphere(const BoundingSphere& sphere)
{
    return BoundingBox(sphere.Center, XMFLOAT3(sphere.Radius, sphere.Radius, sphere.Radius));
}

//---------------------------------------------------------
// Desc:  compute a merged AABB based on input array of AABBs
//---------------------------------------------------------
BoundingBox ComputeAABB(const BoundingBox* AABBs, const size count)
{
    if (!AABBs || count <= 0)
    {
        LogErr(LOG, "invalid input args; return default AABB (center: <0,0,0>, ext: <1,1,1>)");
        return BoundingBox(XMFLOAT3(0,0,0), XMFLOAT3(1,1,1));
    }

    XMVECTOR vMin{ FLT_MAX, FLT_MAX, FLT_MAX };
    XMVECTOR vMax{ FLT_MIN, FLT_MIN, FLT_MIN };

    // go through each subset (mesh)
    for (int i = 0; i < count; ++i)
    {
        // define min/max point of this mesh
        const XMVECTOR center  = XMLoadFloat3(&AABBs[i].Center);
        const XMVECTOR extents = XMLoadFloat3(&AABBs[i].Extents);

        vMin = XMVectorMin(vMin, center - extents);
        vMax = XMVectorMax(vMax, center + extents);
    }

    // compute a model's AABB
    BoundingBox outAABB;
    XMStoreFloat3(&outAABB.Center, 0.5f * (vMin + vMax));
    XMStoreFloat3(&outAABB.Extents, 0.5f * (vMax - vMin));

    return outAABB;
}

} // namespace ECS
