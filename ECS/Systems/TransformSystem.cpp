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

// static arrays for internal purposes
static cvector<index> s_Idxs;


//---------------------------------------------------------
// Desc:  constructor and destructor
//---------------------------------------------------------
TransformSystem::TransformSystem(Transform* pTransform)
{
    CAssert::NotNullptr(pTransform, "ptr to the Transform component == nullptr");
    pTransform_ = pTransform;
}

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

    AddRecordsHelper(ids, positions, directions, uniformScales, numElems);
}

///////////////////////////////////////////////////////////

void TransformSystem::RemoveRecords(const cvector<EntityID>& enttsIDs)
{
    assert("TODO: IMPLEMENT IT!" && 0);
}

// =================================================================================
// GET position/direction/uniform_scale
// =================================================================================

//---------------------------------------------------------
// Desc:  get positions of entities by input IDs
//---------------------------------------------------------
void TransformSystem::GetPositions(
    const EntityID* ids,
    const size numEntts,
    cvector<XMFLOAT3>& outPositions)
{
    if (!ids)
    {
        LogErr(LOG, "input ptr to entities IDs arr == nullptr");
        outPositions.resize(0);
        return;
    }

    Transform& comp = *pTransform_;
    comp.ids.get_idxs(ids, numEntts, s_Idxs);

    // get positions by idxs
    outPositions.resize(numEntts);

    for (int i = 0; const index idx : s_Idxs)
    {
        XMFLOAT4& pos = comp.posAndUniformScale[idx];
        outPositions[i++] = { pos.x, pos.y, pos.z };
    }
}

//---------------------------------------------------------
// Desc:  get arr of direction of entities by input IDs
//---------------------------------------------------------
void TransformSystem::GetDirections(
    const EntityID* ids,
    const size numEntts,
    cvector<XMVECTOR>& outDirections)
{
    if (!ids)
    {
        LogErr(LOG, "input ptr to entities IDs arr == nullptr");
        outDirections.resize(0);
        return;
    }

    Transform& comp = *pTransform_;
    comp.ids.get_idxs(ids, numEntts, s_Idxs);

    // get directions by idxs
    outDirections.resize(numEntts);

    for (int i = 0; const index idx : s_Idxs)
        outDirections[i++] = comp.directions[idx];
}

///////////////////////////////////////////////////////////

void TransformSystem::GetUniformScales(
    const EntityID* ids,
    const size numEntts,
    cvector<float>& outScales)
{
    if (!ids)
    {
        LogErr(LOG, "input ptr to entities IDs arr == nullptr");
        outScales.resize(0);
        return;
    }

    Transform& comp = *pTransform_;
    comp.ids.get_idxs(ids, numEntts, s_Idxs);

    // get uniform scales by idxs
    outScales.resize(numEntts);

    for (int i = 0; const index idx : s_Idxs)
        outScales[i++] = comp.posAndUniformScale[idx].w;   // uniform scale values (float) is packed into float4 in the w-component
}

//---------------------------------------------------------
// Desc:  get positions and directions as float3 of entities by input ids
//---------------------------------------------------------
void TransformSystem::GetPositionsAndDirections(
    const EntityID* ids,
    const size numEntts,
    cvector<XMFLOAT3>& outPositions,
    cvector<XMFLOAT3>& outDirections)
{
    if (!ids)
    {
        LogErr(LOG, "input ptr to entities IDs arr == nullptr");
        outPositions.resize(0);
        outDirections.resize(0);
        return;
    }

    outPositions.resize(numEntts);
    outDirections.resize(numEntts);

    Transform& comp = *pTransform_;
    comp.ids.get_idxs(ids, numEntts, s_Idxs);

    // get positions and directions by idxs
    for (int i = 0; const index idx : s_Idxs)
    {
        const XMFLOAT4& pos = comp.posAndUniformScale[idx];
        outPositions[i++] = { pos.x, pos.y, pos.z };
    }

    for (int i = 0; const index idx : s_Idxs)
    {
        DirectX::XMStoreFloat3(&outDirections[i], comp.directions[idx]);
        ++i;
    }       
}

//---------------------------------------------------------
// Desc:  get position and direction of entity by ID
//---------------------------------------------------------
void TransformSystem::GetPosAndDir(
    const EntityID id,
    XMVECTOR& outPos,
    XMVECTOR& outDir) const
{
    const index idx = GetIdx(id);
    Transform& comp = *pTransform_;
    XMFLOAT4& pos = comp.posAndUniformScale[idx];

    outPos = { pos.x, pos.y, pos.z, 1.0f };
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

const XMVECTOR TransformSystem::GetRotationQuat(const EntityID id) const
{
    XMVECTOR S, R, T;
    const Transform& comp = *pTransform_;
    const index idx = GetIdx(id);
    XMMatrixDecompose(&S, &R, &T, comp.worlds[idx]);

    return R;
}

///////////////////////////////////////////////////////////

const float TransformSystem::GetScale(const EntityID id) const
{
    return pTransform_->posAndUniformScale[GetIdx(id)].w;
}



// =================================================================================
// SET position/direction/uniform_scale
// =================================================================================

//---------------------------------------------------------
// Desc:  check if input entities have the Transform component
//---------------------------------------------------------
bool CheckEnttsHaveTransform(const EntityID* ids, const size numEntts, const index* idxs, Transform& comp)
{
    const size numAllEntts = comp.ids.size();
    bool idxsValid = true;

    // check if we have any invalid index to data
    for (index i = 0; i < numEntts; ++i)
    {
        idxsValid &= (idxs[i] > 0 && idxs[i] < numAllEntts);
    }

    // if we have any invalid index...
    if (idxsValid == false)
    {
        LogErr(LOG, "the following entities don't have Transform component:");

        for (index i = 0; i < numEntts; ++i)
        {
            if (idxs[i] <= 0 || idxs[i] >= numAllEntts)
                printf("\t[%d] id: %" PRIu32 "\n", (int)i, ids[i]);
        }

        return false;
    }

    // all the entities have Transform component
    return true;
}

//---------------------------------------------------------
// Desc:  update position for each input entity by ID
//---------------------------------------------------------
bool TransformSystem::SetPositions(
    const EntityID* ids,
    const size numEntts,
    const XMFLOAT3* positions)
{
    if (!ids || numEntts == 0 || !positions)
    {
        LogErr(LOG, "input args are invalid");
        return false;
    }

    // find idxs by ids
    Transform& comp = *pTransform_;
    comp.ids.get_idxs(ids, numEntts, s_Idxs);

#if DEBUG || _DEBUG
    if (!CheckEnttsHaveTransform(ids, numEntts, s_Idxs.data(), comp))
        return false;
#endif

    // update positions and world matrices
    for (index i = 0; i < numEntts; ++i)
    {
        const index idx = s_Idxs[i];
        XMFLOAT4& data = comp.posAndUniformScale[idx];
        const XMFLOAT3& pos = positions[i];

        // position is stored in x,y,z components (w-component stores the uniform scale so we don't change it)
        data.x = pos.x;
        data.y = pos.y;
        data.z = pos.z;

        // update world matrix
        comp.worlds[idx].r[3] = XMVECTOR{ pos.x, pos.y, pos.z, 1.0f };
    }

    // update inverse world matrices
    for (const index idx : s_Idxs)
        RecomputeInvWorldMatrixByIdx(idx);

    return true;
}

//---------------------------------------------------------
// Desc:  update position for entity by ID
// Ret:   true if we managed to do it
//---------------------------------------------------------
bool TransformSystem::SetPosition(const EntityID id, const XMFLOAT3& pos)
{
    return SetPositions(&id, 1, &pos);
}

//---------------------------------------------------------
// Desc:  adjust position of each input entity by the same offset
//---------------------------------------------------------
bool TransformSystem::AdjustPositions(
    const EntityID* ids,
    const size numEntts,
    const XMVECTOR& adjustBy)
{
    if (!ids || numEntts == 0)
    {
        LogErr(LOG, "input args are invalid");
        return false;
    }

    // get idxs by ids
    Transform& comp = *pTransform_;
    comp.ids.get_idxs(ids, numEntts, s_Idxs);

#if DEBUG || _DEBUG
    if (!CheckEnttsHaveTransform(ids, numEntts, s_Idxs.data(), comp))
        return false;
#endif

    XMFLOAT3 offset;
    XMStoreFloat3(&offset, adjustBy);

    // update positions by idxs
    for (const index idx : s_Idxs)
    {
        XMFLOAT4& pos = comp.posAndUniformScale[idx];
        pos.x += offset.x;
        pos.y += offset.y;
        pos.z += offset.z;
    }

    // update worlds by idxs
    for (const index idx : s_Idxs)
        comp.worlds[idx].r[3] += adjustBy;

    // update inverse worlds by idxs
    for (const index idx : s_Idxs)
        RecomputeInvWorldMatrixByIdx(idx);

    return true;
}

//---------------------------------------------------------
// adjust position of input entity by ID
//---------------------------------------------------------
bool TransformSystem::AdjustPosition(const EntityID id, const XMVECTOR& adjustBy)
{
    return AdjustPositions(&id, 1, adjustBy);
}

//---------------------------------------------------------
// Desc:  update position of entity by ID with input position vector
//---------------------------------------------------------
bool TransformSystem::SetPositionVec(const EntityID id, const XMVECTOR& posVec)
{
    XMFLOAT3 pos;
    XMStoreFloat3(&pos, posVec);

    return SetPositions(&id, 1, &pos);
}

//---------------------------------------------------------
// update direction of entity using input direction
//---------------------------------------------------------
bool TransformSystem::SetDirection(const EntityID id, const XMVECTOR& direction)
{
    const index idx = GetIdx(id);

    // if there is no transformation data for entity by ID
    if (idx == 0)
        return false;

    pTransform_->directions[idx] = XMVector3Normalize(direction);
    return true;
}

//---------------------------------------------------------
// Desc:  set uniform scale for the entity by ID
//---------------------------------------------------------
bool TransformSystem::SetUniScale(const EntityID id, const float scale)
{
    const index idx = GetIdx(id);

    // if there is no transformation data for entity by ID
    if (idx == 0)
        return false;

    Transform& comp = *pTransform_;

    // we store uniform scale in the w-component
    comp.posAndUniformScale[idx].w = scale;

    // recompute world matrix and inverse world matrix for this entity
    XMVECTOR S, R, T;
    XMMatrixDecompose(&S, &R, &T, comp.worlds[idx]);

    S = { scale, scale, scale };

    comp.worlds[idx] =
        XMMatrixScalingFromVector(S) *
        XMMatrixRotationQuaternion(R) *
        XMMatrixTranslationFromVector(T);

    //RecomputeWorldMatrixByIdx(idx);
    RecomputeInvWorldMatrixByIdx(idx);

    return true;
}

//---------------------------------------------------------
// Desc:  helpers for work with DX quaternions
//---------------------------------------------------------
inline XMVECTOR QuatMul(const XMVECTOR& q1, const XMVECTOR& q2)
{
    return DirectX::XMQuaternionMultiply(q1, q2);
}

//---------------------------------------------------------
// Desc:  transform input vector using quaternion
// Args:  - Q:         quaternion
//        - invQ:      inversed quaternion
//        - inOutVec:  in/out vector to transform
//---------------------------------------------------------
inline void TransformVecWithQuat(
    const XMVECTOR& Q,
    const XMVECTOR& invQ,
    XMVECTOR& inOutVec)
{
    // rotated_vector = inv_quat * orig_vec * quat; 
    inOutVec = XMQuaternionMultiply(XMQuaternionMultiply(invQ, inOutVec), Q);
}

//---------------------------------------------------------
// rotate each input entity around itself using input rotation quat(axis, angle)
// 1. rotate the direction vector
// 2. update the world matrix using quaternion
// 3. recompute the world inverse matrix
//---------------------------------------------------------
bool TransformSystem::RotateLocalSpacesByQuat(
    const EntityID* ids,
    const size numEntts,
    const XMVECTOR& quat)
{
    if (!ids)
    {
        LogErr(LOG, "input ptr to the entities IDs arr == nullptr");
        return false;
    }

    Transform& comp = *pTransform_;
    comp.ids.get_idxs(ids, numEntts, s_Idxs);

    const XMVECTOR invQuat = XMQuaternionInverse(quat);

    for (const index idx : s_Idxs)
        TransformVecWithQuat(quat, invQuat, comp.directions[idx]);

    // rotate the worlds
    const XMMATRIX R = XMMatrixRotationQuaternion(quat);

    for (const index idx : s_Idxs)
    {
        XMMATRIX& world   = comp.worlds[idx];
        const XMVECTOR tr = world.r[3];         // store translation

        world.r[3] = { 0,0,0,1 };               // now we have no translation
        world *= R;                             // rotate world (so entt rotates around itself)
        world.r[3] = tr;                        // move to original position
    }

    // compute inverse matrices of updated worlds
    for (const index idx : s_Idxs)
        RecomputeInvWorldMatrixByIdx(idx);

    return true;
}

//---------------------------------------------------------
// rotate entity around itself using input rotation quaternion (axis, angle):
// 1. rotate the direction vector
// 2. update the world matrix using quaternion
// 3. recompute the world inverse matrix
//---------------------------------------------------------
bool TransformSystem::RotateLocalSpaceByQuat(const EntityID id, const XMVECTOR& quat)
{
    Transform& comp = *pTransform_;
    const index idx = GetIdx(id);

    if (idx == 0)
        return false;

    // rotate the dir vector using input quat
    TransformVecWithQuat(quat, XMQuaternionInverse(quat), comp.directions[idx]);

    // move world to <0,0,0>, rotate, and then move back to original pos
    const XMMATRIX R = XMMatrixRotationQuaternion(quat);
    XMMATRIX& world = comp.worlds[idx];
    const XMVECTOR tr = world.r[3];         // store translation

    world.r[3] = { 0,0,0,1 };               // now we have no translation
    world *= R;                             // rotate world (so entt rotates around itself)
    world.r[3] = tr;                        // move to original position

    RecomputeInvWorldMatrixByIdx(idx);

    return true;
}

// =================================================================================
// Get/Set transformation
// =================================================================================

void TransformSystem::TransformWorld(const EntityID id, const XMMATRIX& transformation)
{
    const index idx = GetIdx(id);

    if (idx == 0)
        return;

    const XMVECTOR pos = GetPositionVec(id);
    const XMVECTOR dir = XMVector3Normalize(GetDirectionVec(id));

    SetPositionVec(id, XMVector3Transform(pos, transformation));
    SetDirection  (id, XMVector3Transform(dir, transformation));

    XMMATRIX& W = pTransform_->worlds[idx];
    W = DirectX::XMMatrixMultiply(W, transformation);
    RecomputeInvWorldMatrixByIdx(idx);
}

//---------------------------------------------------------
// Desc:  return a world matrix of entt by ID or
//        return a matrix of NANs if there is no such entt by ID
//---------------------------------------------------------
const DirectX::XMMATRIX& TransformSystem::GetWorld(const EntityID id) const
{
    return pTransform_->worlds[GetIdx(id)];
}

//---------------------------------------------------------
// Desc:  return an inverse world matrix of entt by ID or
//        return a matrix of NANs if there is no such entt by ID
//---------------------------------------------------------
const DirectX::XMMATRIX& TransformSystem::GetInverseWorld(const EntityID id) const
{
    return pTransform_->invWorlds[GetIdx(id)];
}

//---------------------------------------------------------
// Desc:  get arr of world matrices by input entities IDs
//---------------------------------------------------------
void TransformSystem::GetWorlds(
    const EntityID* ids,
    const size numEntts,
    cvector<DirectX::XMMATRIX>& outWorlds)
{
    if (!ids || (numEntts <= 0))
    {
        LogErr(LOG, "input args are invalid");
        outWorlds.resize(0);
        return;
    }

    pTransform_->ids.get_idxs(ids, numEntts, s_Idxs);
    pTransform_->worlds.get_data_by_idxs(s_Idxs, outWorlds);
}

//---------------------------------------------------------
// Desc:  get arr of inverse world matrices by input entities IDs
//---------------------------------------------------------
void TransformSystem::GetInverseWorlds(
    const EntityID* ids,
    const size numEntts,
    cvector<DirectX::XMMATRIX>& outInvWorlds)
{
    if (!ids || (numEntts <= 0))
    {
        LogErr(LOG, "input args are invalid");
        outInvWorlds.resize(0);
        return;
    }

    pTransform_->ids.get_idxs(ids, numEntts, s_Idxs);
    pTransform_->invWorlds.get_data_by_idxs(s_Idxs, outInvWorlds);
}


// =================================================================================
//                            PRIVATE HELPERS
// =================================================================================

void DumpIds(const EntityID* ids, const size numEntts)
{
    printf("\tdump ids:\n");

    for (int i = 0; i < (int)numEntts; ++i)
    {
        printf("[%d] id: %" PRIu32 "\n", i, ids[i]);
    }
    printf("\n");
}

//---------------------------------------------------------
// Desc:  store transformation data of input entities into the Transform component
//---------------------------------------------------------
bool TransformSystem::AddRecordsHelper(
    const EntityID* ids,
    const XMFLOAT3* positions,
    const XMVECTOR* directions,
    const float* uniformScales,
    const size numEntts)
{
    Transform& comp = *pTransform_;

    if (comp.ids.binary_search(ids, numEntts))
    {
        LogErr(LOG, "there is already a record with some input ID");
        DumpIds(ids, numEntts);
        return false;
    }


    cvector<index>& idxs = s_Idxs;
    comp.ids.get_insert_idxs(ids, numEntts, idxs);

    // execute indices correction (so we will insert data into proper positions)
    for (index i = 0; i < idxs.size(); ++i)
        idxs[i] += i;

    // store ids
    for (index i = 0; const index idx : idxs)
        comp.ids.insert_before(idx, ids[i++]);

    // store positions + uniform scales:
    // x,y,z - pos; w - scale
    for (index i = 0; i < numEntts; ++i)
    {
        const XMFLOAT3& p = positions[i];
        const float     s = uniformScales[i];
        comp.posAndUniformScale.insert_before(idxs[i], { p.x, p.y, p.z, s });
    }

    // normalize all the input directions and store them into the component
    for (index i = 0; i < numEntts; ++i)
    {
        const XMVECTOR normalizedDir = XMVector3Normalize(directions[i]);
        comp.directions.insert_before(idxs[i], normalizedDir);
    }
        

    // ----------------------------------------------------

    // compute a world matrix and store it
    // and also compute an inverse world matrix and store it as well
    for (index i = 0; i < numEntts; ++i)
    {
        const XMFLOAT3& p = positions[i];
        const float     s = uniformScales[i];

        const XMMATRIX S = XMMatrixScaling(s, s, s);
        const XMMATRIX T = XMMatrixTranslation(p.x, p.y, p.z);
        const XMMATRIX W = S * T;

        comp.worlds.insert_before(idxs[i], W);
        comp.invWorlds.insert_before(idxs[i], XMMatrixInverse(nullptr, W));
    }

    return true;
}

//---------------------------------------------------------
// return valid idx if there is an entity by such ID;
// or return 0 if there is no such entity (index to "invalid" data)
//---------------------------------------------------------
index TransformSystem::GetIdx(const EntityID id) const
{
    const Transform& comp = *pTransform_;
    const index       idx = comp.ids.get_idx(id);

    if (idx <= 0 || idx >= comp.ids.size())
    {
        LogErr(LOG, "there is no transform data for entt by id: %" PRIu32, id);
        return 0;
    }

    return idx;
}

} // namespace ECS
