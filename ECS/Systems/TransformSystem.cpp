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
    Assert::NotNullptr(pTransform, "ptr to the Transform component == nullptr");
    pTransform_ = pTransform;

    // add invalid data; this data is returned when we ask for wrong entity
    pTransform_->ids.push_back(INVALID_ENTITY_ID);
    pTransform_->posAndUniformScale.push_back(XMFLOAT4{ NAN, NAN, NAN, NAN });
    pTransform_->directions.push_back(XMVECTOR{ NAN, NAN, NAN, NAN });

    const cvector<float> nanArray(16, NAN);
    XMMATRIX nanMatrix(nanArray.data());

    pTransform_->worlds.push_back(nanMatrix);
    pTransform_->invWorlds.push_back(nanMatrix); // inverse world matrix
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
    const XMVECTOR* dirQuats,      // direction quaternions
    const float* uniformScales,
    const size numElems)
{
    Assert::True(ids && positions && dirQuats && uniformScales && (numElems > 0), "invalid input args");

    AddRecordsToTransformComponent(ids, positions, dirQuats, uniformScales, numElems);
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

const XMFLOAT3 TransformSystem::GetPosition(const EntityID id) const
{
    const XMFLOAT4& pos = pTransform_->posAndUniformScale[GetIdx(id)];
    return XMFLOAT3(pos.x, pos.y, pos.z);
}

///////////////////////////////////////////////////////////

const XMVECTOR TransformSystem::GetPositionVec(const EntityID id) const
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
bool TransformSystem::SetPositionVec(const EntityID id, const XMVECTOR& pos)
{
    XMFLOAT3 p;
    XMStoreFloat3(&p, pos);
    return SetPosition(id, p);
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
    RecomputeWorldMatrixByIdx(idx);
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
    RecomputeWorldMatrixByIdx(idx);
    RecomputeInvWorldMatrixByIdx(idx);

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

bool TransformSystem::RotateWorldByQuat(const EntityID id, const XMVECTOR& quat)
{
    using namespace DirectX;

    const index idx = GetIdx(id);

    if (idx == 0)
        return false;


    Transform& comp = *pTransform_;

    // rotate the world
    const XMMATRIX R = XMMatrixRotationQuaternion(quat);
    XMMATRIX W = comp.worlds[idx];
    XMVECTOR translation = W.r[3];
    W.r[3] = { 0,0,0,1 };
    comp.worlds[idx] = W * R;
    comp.worlds[idx].r[3] = translation;
    RecomputeInvWorldMatrixByIdx(idx);

    // rotate the direction vector using input quaternion
    XMVECTOR& dir = comp.directions[idx];
    const XMVECTOR invQuat = XMQuaternionInverse(quat);

    // rotated_direction = inv_quat * orig_direction * quat; 
    XMVECTOR newVec = DirectX::XMQuaternionMultiply(invQuat, dir);
    dir = DirectX::XMQuaternionMultiply(newVec, quat);

    return true;
}


// =================================================================================
// Get/Set transformation
// =================================================================================
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
    // NOTE: size of arrays ids and outWorlds must be equal !!!
    Assert::True(ids != nullptr, "input ptr to entities IDs arr == nullptr");
    Assert::True(numEntts > 0,   "input number of entities must be > 0");

    const Transform& comp = *pTransform_;
    cvector<index> idxs(numEntts);
    cvector<DirectX::XMMATRIX> worlds(numEntts);

    // get data idx by each ID and then get world matrices by these idxs
    comp.ids.get_idxs(ids, numEntts, idxs);
    comp.worlds.get_data_by_idxs(idxs, outWorlds);
}

///////////////////////////////////////////////////////////

void TransformSystem::GetInverseWorlds(
    const EntityID* ids,
    const size numEntts,
    cvector<DirectX::XMMATRIX>& outInvWorlds)
{
    // NOTE: size of arrays enttsIDs and outInvWorlds must be equal !!!

    if (!ids || (numEntts < 0))
        LogErr("input args are invalid");

    const Transform& comp = *pTransform_;
    cvector<index> idxs(numEntts);
    cvector<XMMATRIX> invWorlds(numEntts);

    // get data idx by each ID and then get inverse world matrices by these idxs
    comp.ids.get_idxs(ids, numEntts, idxs);
    comp.invWorlds.get_data_by_idxs(idxs, outInvWorlds);
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
    Assert::True(canAddComponent, "can't add component: there is already a record with some entity id");

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
        const XMMATRIX R = XMMatrixRotationQuaternion(normDirections[i]);
        const XMMATRIX T = XMMatrixTranslation(pos.x, pos.y, pos.z);
        const XMMATRIX world = S * R * T;

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
        sprintf(g_String, "there is no transform data for entt by id: %ud", id);
        LogErr(g_String);
        return 0;
    }

    return idx;
}

} // namespace ECS
