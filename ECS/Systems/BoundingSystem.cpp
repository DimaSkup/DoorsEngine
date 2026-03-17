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

using namespace DirectX;

namespace ECS
{

// static arrays for internal purposes
static cvector<index> s_Idxs;


//**********************************************************************************
// PRIVATE INTERNAL HELPERS
//**********************************************************************************

//---------------------------------------------------------
// Desc:  compute a merged AABB based on input array of AABBs
//---------------------------------------------------------
BoundingBox ComputeMergedAABB(const BoundingBox* AABBs, const size count)
{
    assert(AABBs);
    assert(count > 0);

    XMVECTOR vMin{ FLT_MAX, FLT_MAX, FLT_MAX };
    XMVECTOR vMax{ FLT_MIN, FLT_MIN, FLT_MIN };

    // go through each subset (mesh)
    for (int i = 0; i < count; ++i)
    {
        // define min/max point of this mesh
        const XMVECTOR center = XMLoadFloat3(&AABBs[i].Center);
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

//--------------------------------------------------------
// Desc:  compute a bounding sphere AROUND input axis-aligned bounding box
//--------------------------------------------------------
inline BoundingSphere CreateSphereFromBox(const BoundingBox& aabb)
{
    const XMFLOAT3& e = aabb.Extents;
    return BoundingSphere{ aabb.Center, sqrtf(SQR(e.x) + SQR(e.y) + SQR(e.z)) };
}

//--------------------------------------------------------
// Desc:  compute a bounding sphere from input arr of axis-aligned bounding boxes
//--------------------------------------------------------
inline BoundingSphere CreateSphereFromBoxes(const BoundingBox* AABBs, const size count)
{
    assert(AABBs);
    assert(count > 0);
    return CreateSphereFromBox(ComputeMergedAABB(AABBs, count));
}

//--------------------------------------------------------
// Desc:  compute a axis-aligned bounding box AROUND input bounding sphere
//--------------------------------------------------------
inline BoundingBox CreateBoxFromSphere(const BoundingSphere& sphere)
{
    const float r = sphere.Radius;
    return BoundingBox(sphere.Center, XMFLOAT3(r, r, r));
}


//==================================================================================
// BoundingSystem class's methods implementation
//==================================================================================
BoundingSystem::BoundingSystem(Bounding* pComponent, TransformSystem* pTransSys) :
    pBoundingComponent_(pComponent),
    pTransSys_(pTransSys)
{
    assert(pComponent);
    assert(pTransSys);

    // alloc memory ahead
    pComponent->ids.reserve(256);
    pComponent->data.reserve(256);
}

//---------------------------------------------------------
// Desc:  relocate world bounding box and world bounding sphere
//        according to current world position of entity by id
//---------------------------------------------------------
void BoundingSystem::TranslateWorldBoundings(const EntityID id)
{
    TranslateWorldBoundings(&id, 1);
}

void BoundingSystem::TranslateWorldBoundings(const EntityID* ids, const size count)
{
    if (!ids || count <= 0)
    {
        LogErr(LOG, "invalid input args");
        return;
    }

    for (index i = 0; i < count; ++i)
    {
        ECS::BoundData& data = pBoundingComponent_->data[GetIdx(ids[i])];
        const XMFLOAT3&    p = pTransSys_->GetPosition(ids[i]);


        // translate world AABB
        const XMFLOAT3& localBoxC = data.localBox.Center;
        XMFLOAT3&       worldBoxC = data.worldBox.Center;

        worldBoxC.x = localBoxC.x + p.x;
        worldBoxC.y = localBoxC.y + p.y;
        worldBoxC.z = localBoxC.z + p.z;


        // translate world sphere
        const XMFLOAT3& localSphereC = data.localSphere.Center;
        XMFLOAT3&       worldSphereC = data.worldSphere.Center;

        worldSphereC.x = localSphereC.x + p.x;
        worldSphereC.y = localSphereC.y + p.y;
        worldSphereC.z = localSphereC.z + p.z;
    }
}

//---------------------------------------------------------
// Desc:  recompute world bounding box and world bounding sphere
//        according to current world transformation of entity by id
//---------------------------------------------------------
void BoundingSystem::UpdateWorldBoundings(const EntityID id)
{
    UpdateWorldBoundings(&id, 1);
}

void BoundingSystem::UpdateWorldBoundings(const EntityID* ids, const size count)
{
    if (!ids || count <= 0)
    {
        LogErr(LOG, "invalid input args");
        return;
    }

    for (index i = 0; i < count; ++i)
    {
        ECS::BoundData& data = pBoundingComponent_->data[GetIdx(ids[i])];
        const XMMATRIX& W    = pTransSys_->GetWorld(ids[i]);

        data.localBox.Transform(data.worldBox, W);
        data.localSphere.Transform(data.worldSphere, W);
    }
}

//---------------------------------------------------------
// Desc:  add only one entt with only one bounding SPHERE
//---------------------------------------------------------
bool BoundingSystem::Add(
    const EntityID id,
    const BoundingSphere& localSphere,     // in local space
    const BoundingSphere& worldSphere)     // in world space
{
    Bounding& comp = *pBoundingComponent_;

    // check if we already have a record with such ID
    if (comp.ids.binary_search(id))
    {
        LogErr(LOG, "there is already a record with entity: %" PRIu32, id);
        return false;
    }

    const BoundingBox localBox = CreateBoxFromSphere(localSphere);
    const BoundingBox worldBox = CreateBoxFromSphere(worldSphere);

    // if we just need to push back
    if (id > comp.ids.back())
    {
        comp.ids.push_back(id);
        comp.data.push_back(BoundData(localBox, worldBox, localSphere, worldSphere));
    }

    // execute sorted insertion of ID and initial data
    else
    {
        const index idx = comp.ids.get_insert_idx(id);
        comp.ids.insert_before(idx, id);
        comp.data.insert_before(idx, BoundData(localBox, worldBox, localSphere, worldSphere));
    }

    return true;
}

//---------------------------------------------------------
// Desc:  add only one entt with only one axis-aligned bounding box (AABB)
//---------------------------------------------------------
bool BoundingSystem::Add(
    const EntityID id,
    const BoundingBox& localBox,     // in local space
    const BoundingBox& worldBox)     // in world space
{
    Bounding& comp = *pBoundingComponent_;

    // check if we already have a record with such ID
    if (comp.ids.binary_search(id))
    {
        LogErr(LOG, "there is already a record with entity: %" PRIu32, id);
        return false;
    }

    const BoundingSphere localSphere = CreateSphereFromBox(localBox);
    const BoundingSphere worldSphere = CreateSphereFromBox(worldBox);

    // if we just need to push back
    if (id > comp.ids.back())
    {
        comp.ids.push_back(id);
        comp.data.push_back(BoundData(localBox, worldBox, localSphere, worldSphere));
    }

    // execute sorted insertion of ID and initial data
    else
    {
        const index idx = comp.ids.get_insert_idx(id);
        comp.ids.insert_before(idx, id);
        comp.data.insert_before(idx, BoundData(localBox, worldBox, localSphere, worldSphere));
    }
    
    return true;
}

//---------------------------------------------------------
// Desc:  add THE SAME local bounding sphere, but DIFFERENT world bounding sphere
//        for each input entity
// 
// Args:  - ids:        array of entities IDs
//        - numEntts:   how many entities in the input arr
//        - localSphere:    bounding sphere in local space
//        - worldSpheres:   arr of bounding spheres in world space
//---------------------------------------------------------
bool BoundingSystem::Add(
    const EntityID* ids,
    const size numEntts,
    const DirectX::BoundingSphere& localSphere,
    const DirectX::BoundingSphere* worldSpheres)
{
    assert(ids);
    assert(numEntts > 0);
    assert(worldSpheres);

    Bounding& comp = *pBoundingComponent_;
    BoundingBox localBox = CreateBoxFromSphere(localSphere);
    BoundingBox worldBox;

    // check that there are no records with input ids yet
    if (comp.ids.binary_search(ids, numEntts))
    {
        LogErr(LOG, "there is already a record with some input id");
        return false;
    }

    comp.ids.get_insert_idxs(ids, numEntts, s_Idxs);

    // sorted intertion of IDs and initial data
    for (index i = 0; i < numEntts; ++i)
        comp.ids.insert_before(s_Idxs[i]+i, ids[i]);

    for (index i = 0; i < numEntts; ++i)
    {
        worldBox = CreateBoxFromSphere(worldSpheres[i]);
        comp.data.insert_before(s_Idxs[i]+i, BoundData(localBox, worldBox, localSphere, worldSpheres[i]));
    }

    return true;
}

//---------------------------------------------------------
// Desc:  add THE SAME local bounding box, but DIFFERENT world bounding box
//        for each input entity
// 
// Args:  - ids:        array of entities IDs
//        - numEntts:   how many entities in the input arr
//        - localBox:       AABB in local space
//        - worldBoxes:     arr of AABBs in world space
//---------------------------------------------------------
bool BoundingSystem::Add(
    const EntityID* ids,
    const size numEntts,
    const DirectX::BoundingBox& localBox,
    const DirectX::BoundingBox* worldBoxes)
{
    assert(ids);
    assert(numEntts > 0);
    assert(worldBoxes);

    Bounding& comp = *pBoundingComponent_;
    BoundingSphere localSphere = CreateSphereFromBox(localBox);
    BoundingSphere worldSphere;

    // check that there are no records with input IDs yet
    if (comp.ids.binary_search(ids, numEntts))
    {
        LogErr(LOG, "there is already a record with some input ID");
        return false;
    }

    comp.ids.get_insert_idxs(ids, numEntts, s_Idxs);

    // sorted insertion of IDs and initial data
    for (index i = 0; i < numEntts; ++i)
        comp.ids.insert_before(s_Idxs[i]+i, ids[i]);

    for (index i = 0; i < numEntts; ++i)
    {
        worldSphere = CreateSphereFromBox(worldBoxes[i]);
        comp.data.insert_before(s_Idxs[i]+i, BoundData(localBox, worldBoxes[i], localSphere, worldSphere));
    }

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
    assert(ids);
    assert(numEntts);

    Bounding& comp = *pBoundingComponent_;

    comp.ids.get_idxs(ids, numEntts, s_Idxs);
    outSpheres.resize(numEntts);

    for (int i = 0; const index idx : s_Idxs)
        outSpheres[i++] = comp.data[idx].worldSphere;
}

//--------------------------------------------------------
// DEBUG PRINT:  print all the bounding component data into console
//--------------------------------------------------------
void BoundingSystem::PrintDump() const
{
    Bounding& comp = *pBoundingComponent_;

    printf("\n");
    for (index i = 0; i < comp.ids.size(); ++i)
    {
        const BoundingBox&       localBox = comp.data[i].localBox;
        const BoundingBox&       worldBox = comp.data[i].worldBox;
        const BoundingSphere& localSphere = comp.data[i].localSphere;
        const BoundingSphere& worldSphere = comp.data[i].worldSphere;

        printf("[%d] id: %d  ",                                (int)i, (int)comp.ids[i]);
        printf("boxL(%.2f %.2f %.2f, %.2f %.2f %.2f),  ",      localBox.Center.x, localBox.Center.y, localBox.Center.z, localBox.Extents.x, localBox.Extents.y, localBox.Extents.z);
        printf("boxW(% .2f % .2f % .2f, % .2f % .2f % .2f), ", worldBox.Center.x, worldBox.Center.y, worldBox.Center.z, worldBox.Extents.x, worldBox.Extents.y, worldBox.Extents.z);
        printf("sphereL(%.2f %.2f %.2f,  r=%.2f),  ",          localSphere.Center.x, localSphere.Center.y, localSphere.Center.z, localSphere.Radius);
        printf("sphereW(%.2f %.2f %.2f,  r=%.2f)\n",           worldSphere.Center.x, worldSphere.Center.y, worldSphere.Center.z, worldSphere.Radius);
    }
    printf("\n");
}

} // namespace ECS
