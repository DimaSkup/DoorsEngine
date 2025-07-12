// =================================================================================
// Filename:      TranformSystem.cpp
// Description:   implementation of the TransformSystem's functional
// 
// Created:       20.05.24
// =================================================================================
#include "../Common/pch.h"
#include "TransformSystem.h"

#pragma warning (disable : 4996)

using namespace DirectX;

namespace ECS
{

TransformSystem::TransformSystem(Transform* pTransform)
{
    CAssert::NotNullptr(pTransform, "ptr to the Transform component == nullptr");
    pTransform_ = pTransform;
}

///////////////////////////////////////////////////////////

TransformSystem::~TransformSystem()
{
}


// =================================================================================
// PUBLIC METHODS
// =================================================================================
void TransformSystem::AddRecords(
    const EntityID* ids,
    const XMFLOAT3* positions,
    const XMVECTOR* directions,
    const float* uniformScales,
    const size numElems)
{
    CAssert::True(ids && positions && directions && uniformScales && (numElems > 0), "invalid input args");

    AddRecordsToTransformComponent(ids, positions, directions, uniformScales, numElems);
}

///////////////////////////////////////////////////////////

void TransformSystem::RemoveRecords(const cvector<EntityID>& enttsIDs)
{
    assert("TODO: IMPLEMENT IT!" && 0);
}

// =================================================================================
// GET position/direction/uniform_scale
// =================================================================================

void TransformSystem::GetPositions(
    const EntityID* ids,
    const size numEntts,
    cvector<XMFLOAT3>& outPositions) const
{
    // get positions of entities by input IDs array;

    if (!ids)
    {
        LogErr("input ptr to entities IDs arr == nullptr");
        return;
    }

    Transform& comp = *pTransform_;
    cvector<index> idxs;

    comp.ids.get_idxs(ids, numEntts, idxs);

    // get positions by idxs
    outPositions.resize(numEntts);

    for (int i = 0; const index idx : idxs)
    {
        XMFLOAT4& data = comp.posAndUniformScale[idx];   // pos (float3) + scale (float)
        outPositions[i++] = { data.x, data.y, data.z };
    }
}

///////////////////////////////////////////////////////////

void TransformSystem::GetDirections(
    const EntityID* ids,
    const size numEntts,
    cvector<XMVECTOR>& outDirections) const
{
    if (!ids)
    {
        LogErr("input ptr to entities IDs arr == nullptr");
        return;
    }

    Transform& comp = *pTransform_;
    cvector<index> idxs;

    comp.ids.get_idxs(ids, numEntts, idxs);

    // get directions by idxs
    outDirections.resize(numEntts);

    for (int i = 0; const index idx : idxs)
        outDirections[i++] = comp.directions[idx];
}

///////////////////////////////////////////////////////////

void TransformSystem::GetUniformScales(
    const EntityID* ids,
    const size numEntts,
    cvector<float>& outScales) const
{
    if (!ids)
    {
        LogErr("input ptr to entities IDs arr == nullptr");
        return;
    }

    Transform& comp = *pTransform_;
    cvector<index> idxs;

    comp.ids.get_idxs(ids, numEntts, idxs);

    // get uniform scales by idxs
    outScales.resize(numEntts);

    for (int i = 0; const index idx : idxs)
        outScales[i++] = comp.posAndUniformScale[idx].w;   // uniform scale values (float) is packed into float4 in the w-component
}

///////////////////////////////////////////////////////////

void TransformSystem::GetPositionsAndDirections(
    const EntityID* ids,
    const size numEntts,
    cvector<XMFLOAT3>& outPositions,
    cvector<XMFLOAT3>& outDirections) const
{
    // get positions and directions as float3 of entities by input ids

    if (!ids)
    {
        LogErr("input ptr to entities IDs arr == nullptr");
        return;
    }

    outPositions.resize(numEntts);
    outDirections.resize(numEntts);

    Transform& comp = *pTransform_;
    cvector<index> idxs;

    comp.ids.get_idxs(ids, numEntts, idxs);

    // get positions and directions by idxs
    for (int i = 0; const index idx : idxs)
    {
        const XMFLOAT4& data = comp.posAndUniformScale[idx];
        outPositions[i++] = { data.x, data.y, data.z };      // position is stored in (x,y,z) of float4
    }

    for (int i = 0; const index idx : idxs)
    {
        DirectX::XMStoreFloat3(&outDirections[i], comp.directions[idx]);
        ++i;
    }       
}

///////////////////////////////////////////////////////////

void TransformSystem::GetPosAndDir(
    const EntityID id,
    XMVECTOR& outPos,
    XMVECTOR& outDir) const
{
    // get position and direction of entity by ID
    const index idx = GetIdx(id);
    Transform& comp = *pTransform_;
    XMFLOAT4& pos = comp.posAndUniformScale[idx];

    outPos = { pos.x, pos.y, pos.z, 1.0f };           // in w-component we store uniform scale, but not we need it to be just 1
    outDir = comp.directions[idx];
}

///////////////////////////////////////////////////////////

XMFLOAT3 TransformSystem::GetPosition(const EntityID id) const
{
    const XMFLOAT4& pos = pTransform_->posAndUniformScale[GetIdx(id)];
    return XMFLOAT3(pos.x, pos.y, pos.z);
}

///////////////////////////////////////////////////////////

XMFLOAT4 TransformSystem::GetPositionFloat4(const EntityID id) const
{
    XMFLOAT4 pos = pTransform_->posAndUniformScale[GetIdx(id)];
    pos.w = 1.0f;
    return pos;
}

///////////////////////////////////////////////////////////

XMVECTOR TransformSystem::GetPositionVec(const EntityID id) const
{
    const XMFLOAT4& pos = pTransform_->posAndUniformScale[GetIdx(id)];
    return XMVECTOR{ pos.x, pos.y, pos.z, 1 };
}

///////////////////////////////////////////////////////////

const XMVECTOR& TransformSystem::GetDirectionVec(const EntityID id) const
{
    return pTransform_->directions[GetIdx(id)];
}

///////////////////////////////////////////////////////////

const XMFLOAT3 TransformSystem::GetDirection(const EntityID id) const
{
    XMFLOAT3 dir;
    XMStoreFloat3(&dir, GetDirectionVec(id));
    return dir;
}

///////////////////////////////////////////////////////////

const float TransformSystem::GetUniformScale(const EntityID id) const
{
    return pTransform_->posAndUniformScale[GetIdx(id)].w;
}


// =================================================================================
// SET position/direction/uniform_scale
// =================================================================================
bool TransformSystem::AdjustPositions(
    const EntityID* ids,
    const size numEntts,
    const XMVECTOR& adjustBy)
{
    // adjust position of each input entity

    if (!ids)
    {
        LogErr("input ptr to the entities IDs arr == nullptr");
        return false;
    }

    Transform& comp = *pTransform_;

    cvector<index> idxs;
    comp.ids.get_idxs(ids, numEntts, idxs);

    XMFLOAT3 offset;
    XMStoreFloat3(&offset, adjustBy);

    // update positions by idxs
    for (const index idx : idxs)
    {
        XMFLOAT4& pos = comp.posAndUniformScale[idx];
        pos.x += offset.x;
        pos.y += offset.y;
        pos.z += offset.z;
    }

    // update worlds by idxs
    for (const index idx : idxs)
        comp.worlds[idx].r[3] += adjustBy;

    // update inverse worlds by idxs
    for (const index idx : idxs)
        RecomputeInvWorldMatrixByIdx(idx);

    return true;
}

///////////////////////////////////////////////////////////

bool TransformSystem::AdjustPosition(const EntityID id, const XMVECTOR& adjustBy)
{
    // adjust position of input entity by ID

    const index idx = GetIdx(id);

    // if there is no data by ID
    if (idx == 0)
        return false;

    Transform& comp = *pTransform_;
    XMFLOAT4& pos = comp.posAndUniformScale[idx];

    pos.x += XMVectorGetX(adjustBy);
    pos.y += XMVectorGetY(adjustBy);
    pos.z += XMVectorGetZ(adjustBy);

    comp.worlds[idx].r[3] += adjustBy;
    RecomputeInvWorldMatrixByIdx(idx);

    return true;
}

///////////////////////////////////////////////////////////

bool TransformSystem::SetPositionVec(const EntityID id, const XMVECTOR& pos)
{
    const index idx = GetIdx(id);

    // if there is no transformation data for entity by ID
    if (idx == 0)
        return false;

    XMFLOAT3 p;
    XMStoreFloat3(&p, pos);

    Transform& comp = *pTransform_;
    XMFLOAT4& data = comp.posAndUniformScale[idx];

    // position is stored in x,y,z components (w-component stores the uniform scale so we don't change it)
    data.x = p.x;
    data.y = p.y;
    data.z = p.z;

    comp.worlds[idx].r[3] = XMVECTOR{p.x, p.y, p.z, 1.0f};
    RecomputeInvWorldMatrixByIdx(idx);

    return true;
}

///////////////////////////////////////////////////////////

bool TransformSystem::SetPosition(const EntityID id, const XMFLOAT3& pos)
{
    const index idx = GetIdx(id);

    // if there is no transformation data for entity by ID
    if (idx == 0)
        return false;

    Transform& comp = *pTransform_;
    XMFLOAT4& data = comp.posAndUniformScale[idx];

    // position is stored in x,y,z components (w-component stores the uniform scale so we don't change it)
    data.x = pos.x;
    data.y = pos.y;
    data.z = pos.z;

    // recompute world matrix and inverse world matrix for this entity
    comp.worlds[idx].r[3] = XMVECTOR{ pos.x, pos.y, pos.z, 1.0f };
    RecomputeInvWorldMatrixByIdx(idx);

    return true;
}

///////////////////////////////////////////////////////////

bool TransformSystem::SetDirection(const EntityID id, const XMVECTOR& direction)
{
    // update direction of entity using input direction

    const index idx = GetIdx(id);

    // if there is no transformation data for entity by ID
    if (idx == 0)
        return false;

    pTransform_->directions[idx] = XMVector3Normalize(direction);

    // recompute world matrix and inverse world matrix for this entity
    //RecomputeWorldMatrixByIdx(idx);
    //RecomputeInvWorldMatrixByIdx(idx);

    return true;
}

///////////////////////////////////////////////////////////

bool TransformSystem::SetUniScale(const EntityID id, const float uniformScale)
{
    // set the uniform scale value for the entity by ID

    const index idx = GetIdx(id);

    // if there is no transformation data for entity by ID
    if (idx == 0)
        return false;

    // uniform scale is stored in the w-component
    pTransform_->posAndUniformScale[idx].w = uniformScale;

    // recompute world matrix and inverse world matrix for this entity
    RecomputeWorldMatrixByIdx(idx);
    RecomputeInvWorldMatrixByIdx(idx);

    return true;
}

///////////////////////////////////////////////////////////

bool TransformSystem::RotateLocalSpacesByQuat(
    const EntityID* ids,
    const size numEntts,
    const XMVECTOR& quat)
{
    // rotate each input entity around itself using input rotation quat(axis, angle)
    // 1. rotate the direction vector
    // 2. update the world matrix using quaternion
    // 3. recompute the world inverse matrix

    if (!ids)
    {
        LogErr("input ptr to the entities IDs arr == nullptr");
        return false;
    }

    using namespace DirectX;
    Transform& comp = *pTransform_;

    cvector<index> idxs;
    comp.ids.get_idxs(ids, numEntts, idxs);

    // TODO: maybe put here the check if all input entities are actually exist?

    // rotate the direction vectors using input quaternion
    const XMVECTOR invQuat = XMQuaternionInverse(quat);

    for (const index idx : idxs)
    {
        // rotated_direction = inv_quat * orig_direction * quat; 
        XMVECTOR newVec      = XMQuaternionMultiply(invQuat, comp.directions[idx]);
        comp.directions[idx] = XMQuaternionMultiply(newVec, quat);
    }

    // rotate the worlds
    const XMMATRIX R = XMMatrixRotationQuaternion(quat);

    for (const index idx : idxs)
    {
        XMMATRIX oldWorld = comp.worlds[idx];
        XMVECTOR translation = oldWorld.r[3];

        oldWorld.r[3] = { 0,0,0,1 };           // translate it to the world's origin
        comp.worlds[idx] = oldWorld * R;       // rotate world 
        comp.worlds[idx].r[3] = translation;   // translate world to original position
    }

    // compute inverse matrices of updated worlds
    for (const index idx : idxs)
        RecomputeInvWorldMatrixByIdx(idx);

    return true;
}

///////////////////////////////////////////////////////////

bool TransformSystem::RotateLocalSpaceByQuat(const EntityID id, const XMVECTOR& quat)
{
    // rotate entity around itself using input rotation quaternion (axis, angle):
    // 1. rotate the direction vector
    // 2. update the world matrix using quaternion
    // 3. recompute the world inverse matrix

    using namespace DirectX;

    Transform& comp = *pTransform_;
    const index idx = comp.ids.get_idx(id);

    if (comp.ids[idx] != id)
    {
        sprintf(g_String, "there is no transform data for entt by id: %ld", id);
        LogErr(g_String);
        return false;
    }

    // rotate the direction vector using input quaternion
    XMVECTOR& dir = comp.directions[idx];
    const XMVECTOR invQuat = XMQuaternionInverse(quat);

    // rotated_direction = inv_quat * orig_direction * quat; 
    const XMVECTOR tmpVec = DirectX::XMQuaternionMultiply(invQuat, dir);
    dir = DirectX::XMQuaternionMultiply(tmpVec, quat);

    // rotate the world
    const XMMATRIX R = XMMatrixRotationQuaternion(quat);
    XMMATRIX oldWorld = comp.worlds[idx];
    XMVECTOR translation = oldWorld.r[3];

    oldWorld.r[3] = { 0,0,0,1 };           // translate it to the world's origin
    comp.worlds[idx] = oldWorld * R;       // rotate world 
    comp.worlds[idx].r[3] = translation;   // translate world to original position

    RecomputeInvWorldMatrixByIdx(idx);

    return true;
}


// =================================================================================
// Get/Set transformation
// =================================================================================

void TransformSystem::TransformWorld(const EntityID id, const XMMATRIX& transformation)
{
    const index idx = GetIdx(id);
    const XMVECTOR pos = GetPositionVec(id);
    const XMVECTOR dir = XMVector3Normalize(GetDirectionVec(id));

    const XMVECTOR newPos = XMVector3Transform(pos, transformation);
    const XMVECTOR newDir = XMVector3Transform(dir, transformation);

    SetPositionVec(id, newPos);
    SetDirection(id, newDir);

    pTransform_->worlds[idx] = DirectX::XMMatrixMultiply(pTransform_->worlds[idx], transformation);
    RecomputeInvWorldMatrixByIdx(idx);
}

///////////////////////////////////////////////////////////

DirectX::XMMATRIX TransformSystem::GetWorldMatrixOfEntt(const EntityID id)
{
    // return a world matrix of entt by ID or return a matrix of NANs if there is no such entt by ID
    return pTransform_->worlds[GetIdx(id)];
}

///////////////////////////////////////////////////////////

const DirectX::XMMATRIX& TransformSystem::GetInverseWorld(const EntityID id)
{
    // return an inverse world matrix of entt by ID or return a matrix of NANs if there is no such entt by ID
    return pTransform_->invWorlds[GetIdx(id)];
}

///////////////////////////////////////////////////////////

void TransformSystem::GetWorlds(
    const EntityID* ids,
    const size numEntts,
    cvector<DirectX::XMMATRIX>& outWorlds)
{
    CAssert::True(ids != nullptr, "input ptr to entities IDs arr == nullptr");
    CAssert::True(numEntts > 0,   "input number of entities must be > 0");

    // get data idx by each ID and then get world matrices by these idxs
    pTransform_->ids.get_idxs(ids, numEntts, s_Idxs);
    pTransform_->worlds.get_data_by_idxs(s_Idxs, outWorlds);
}

///////////////////////////////////////////////////////////

void TransformSystem::GetInverseWorlds(
    const EntityID* ids,
    const size numEntts,
    cvector<DirectX::XMMATRIX>& outInvWorlds)
{
    if (!ids || (numEntts < 0))
        LogErr("input args are invalid");

    // get data idx by each ID and then get inverse world matrices by these idxs
    pTransform_->ids.get_idxs(ids, numEntts, s_Idxs);
    pTransform_->invWorlds.get_data_by_idxs(s_Idxs, outInvWorlds);
}


// =================================================================================
//                            PRIVATE HELPERS
// =================================================================================
void TransformSystem::AddRecordsToTransformComponent(
    const EntityID* ids,
    const XMFLOAT3* positions,
    const XMVECTOR* directions,
    const float* uniformScales,
    const size numElems)
{
    // store transformation data of input entities into the Transform component

    Transform& comp = *pTransform_;

    bool canAddComponent = !comp.ids.binary_search(ids, numElems);
    CAssert::True(canAddComponent, "can't add component: there is already a record with some entity id");

    cvector<index> idxs;
    comp.ids.get_insert_idxs(ids, numElems, idxs);

    // execute indices correction
    for (index i = 0; i < idxs.size(); ++i)
        idxs[i] += i;

    // store ids
    for (index i = 0; i < numElems; ++i)
        comp.ids.insert_before(idxs[i], ids[i]);

    // store positions
    for (index i = 0; i < numElems; ++i)
    {
        const XMFLOAT3& pos = positions[i];
        comp.posAndUniformScale.insert_before(idxs[i], { pos.x, pos.y, pos.z, 1.0f });
    }

    // we store uniform scale values in the w-component of float4
    for (index i = 0; i < numElems; ++i)
        comp.posAndUniformScale[idxs[i]].w = uniformScales[i];

    // normalize all the input directions and store them into the component
    cvector<XMVECTOR> normDirections(numElems);

    for (int i = 0; i < numElems; ++i)
        normDirections[i] = DirectX::XMVector3Normalize(directions[i]);

    for (index i = 0; i < numElems; ++i)
        comp.directions.insert_before(idxs[i], normDirections[i]);

    // ----------------------------------------------------

    // compute a world matrix and store it
    // and also compute an inverse world matrix and store it as well
    for (index i = 0; i < numElems; ++i)
    {
        const XMFLOAT3 pos = positions[i];
        const float scale  = uniformScales[i];

        // compute a world matrix and store it
        // and also compute an inverse world matrix and store it as well
        const XMMATRIX S = XMMatrixScaling(scale, scale, scale);
        //const XMMATRIX R = XMMatrixRotation(normDirections[i]);
        const XMMATRIX T = XMMatrixTranslation(pos.x, pos.y, pos.z);
        const XMMATRIX world = S * T;

        comp.worlds.insert_before(idxs[i], world);
        comp.invWorlds.insert_before(idxs[i], XMMatrixInverse(nullptr, world));
    }
}

///////////////////////////////////////////////////////////

index TransformSystem::GetIdx(const EntityID id) const
{
    // return valid idx if there is an entity by such ID;
    // or return 0 if there is no such entity;
    const index idx = pTransform_->ids.get_idx(id);

    if (pTransform_->ids[idx] != id)
    {
        sprintf(g_String, "there is no transform data for entt by id: %ld", id);
        LogErr(g_String);
        return 0;
    }

    return idx;
}

} // namespace ECS
