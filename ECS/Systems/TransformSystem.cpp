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
    pTransform_->dirQuats.push_back(XMVECTOR{ NAN, NAN, NAN, NAN });

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

#if 0
void TransformSystem::Initialize(Transform* pTransform)
{
    Assert::NotNullptr(pTransform, "ptr to the Transform component == nullptr");
    pTransform_ = pTransform;

    // add invalid data; this data is returned when we ask for wrong entity
    pTransform_->ids_.push_back(INVALID_ENTITY_ID);
    pTransform_->posAndUniformScale.push_back(XMFLOAT4{ NAN, NAN, NAN, NAN });
    pTransform_->dirQuats.push_back(XMVECTOR{ NAN, NAN, NAN, NAN });

    const cvector<float> nanArray(16, NAN);
    XMMATRIX nanMatrix(nanArray.data());

    pTransform_->worlds.push_back(nanMatrix);
    pTransform_->invWorlds.push_back(nanMatrix); // inverse world matrix
}
#endif

///////////////////////////////////////////////////////////

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

void TransformSystem::GetPositionsByIDs(
    const EntityID* ids,
    const size numEntts,
    cvector<XMFLOAT3>& outPositions) const
{
    // get positions of entities by input IDs array;
    // 
    // NOTE: it is supposed that ids.size() == outPositions.size()

    Assert::True(ids != nullptr, "input ptr to entities IDs arr == nullptr");
    Assert::True(numEntts > 0,   "input number of entities must be > 0");

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

void TransformSystem::GetDirectionsQuatsByIDs(
    const EntityID* ids,
    const size numEntts,
    cvector<XMVECTOR>& outDirQuats) const
{
    if (!ids)
    {
        LogErr("input ptr to entities IDs arr == nullptr");
        return;
    }

    Transform& comp = *pTransform_;
    cvector<index> idxs;

    comp.ids.get_idxs(ids, numEntts, idxs);

    // get direction quaternions by idxs
    outDirQuats.resize(numEntts);

    for (int i = 0; const index idx : idxs)
        outDirQuats[i++] = comp.dirQuats[idx];
}

///////////////////////////////////////////////////////////

void TransformSystem::GetUniformScalesByIDs(
    const EntityID* ids,
    const size numEntts,
    cvector<float>& outScales) const
{
    Assert::True(ids != nullptr, "input ptr to entities IDs arr == nullptr");
    Assert::True(numEntts > 0,   "input number of entities must be > 0");

    Transform& comp = *pTransform_;
    cvector<index> idxs;

    comp.ids.get_idxs(ids, numEntts, idxs);

    // get uniform scales by idxs
    outScales.resize(numEntts);

    for (int i = 0; const index idx : idxs)
        outScales[i++] = comp.posAndUniformScale[idx].w;   // uniform scale values (float) is packed into float4 in the w-component
}

///////////////////////////////////////////////////////////

void TransformSystem::GetPositionsAndDirectionsByIDs(
    const EntityID* ids,
    const size numEntts,
    cvector<XMFLOAT3>& outPositions,
    cvector<XMFLOAT3>& outDirections)
{
    Assert::True(ids != nullptr, "input ptr to entities IDs arr == nullptr");
    Assert::True(numEntts > 0,   "input number of entities must be > 0");

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
        DirectX::XMStoreFloat3(&outDirections[i], comp.dirQuats[idx]);  // we use quaterion rotation axis as direction of light
        ++i;
    }
        
}

///////////////////////////////////////////////////////////

const XMFLOAT3 TransformSystem::GetPositionByID(const EntityID id) const
{
    const XMFLOAT4& pos = pTransform_->posAndUniformScale[GetIdxByID(id)];
    return XMFLOAT3(pos.x, pos.y, pos.z);
}

///////////////////////////////////////////////////////////

const XMVECTOR TransformSystem::GetDirectionQuatByID(const EntityID id) const
{
    return pTransform_->dirQuats[GetIdxByID(id)];
}

///////////////////////////////////////////////////////////

const float TransformSystem::GetUniformScaleByID(const EntityID id) const
{
    return pTransform_->posAndUniformScale[GetIdxByID(id)].w;
}


// =================================================================================
// SET position/direction/uniform_scale
// =================================================================================

bool TransformSystem::SetPositionByID(const EntityID id, const XMFLOAT3& pos)
{
    const index idx = GetIdxByID(id);

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

bool TransformSystem::SetDirectionQuatByID(const EntityID id, const XMVECTOR& dirQuat)
{
    // update direction of entity using input direction quaternion

    const index idx = GetIdxByID(id);

    // if there is no transformation data for entity by ID
    if (idx == 0)
        return false;

    pTransform_->dirQuats[idx] = XMQuaternionNormalize(dirQuat);

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

    Transform& comp = *pTransform_;
    const index idx = comp.ids.get_idx(id);

    if (idx == -1)
    {
        char buf[64];
        sprintf(buf, "there is no transformation for entity by ID: %ud", id);
        LogErr(buf);
        return false;
    }

    const XMMATRIX R = XMMatrixRotationQuaternion(quat);
    XMMATRIX W = comp.worlds[idx];
    XMVECTOR translation = W.r[3];
    W.r[3] = { 0,0,0,1 };
    comp.worlds[idx] = W * R;
    comp.worlds[idx].r[3] = translation;
    RecomputeInvWorldMatrixByIdx(idx);

    XMVECTOR scale;
    XMVECTOR rotQuat;
    //XMVECTOR translation;
    XMMatrixDecompose(&scale, &rotQuat, &translation, comp.worlds[idx]);

    // store translation in (x,y,z) of float4
    XMStoreFloat4(&comp.posAndUniformScale[idx], translation);

    // store uniform scale in w-component of float4
    comp.posAndUniformScale[idx].w = XMVectorGetX(scale);

    // store direction quaternion
    comp.dirQuats[idx] = rotQuat;

    return true;

#if 0
    XMVECTOR& origQuat     = comp.dirQuats[idx];
    const XMVECTOR invQuat = XMQuaternionInverse(quat);

    // rotated_vec = inv_quat * vec * quat;
    XMVECTOR newVec = DirectX::XMQuaternionMultiply(invQuat, origQuat);
    origQuat        = DirectX::XMQuaternionMultiply(newVec, quat);

    return true;
#endif
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
    const XMFLOAT4& data = pTransform_->posAndUniformScale[idx];

    dirQuat = pTransform_->dirQuats[idx];
    pos = { data.x, data.y, data.z };
    uniformScale = data.w;
}

///////////////////////////////////////////////////////////

DirectX::XMMATRIX TransformSystem::GetWorldMatrixOfEntt(const EntityID id)
{
    // return a world matrix of entt by ID or return a matrix of NANs if there is no such entt by ID
    return pTransform_->worlds[GetIdxByID(id)];
}

///////////////////////////////////////////////////////////

const DirectX::XMMATRIX& TransformSystem::GetInverseWorldMatrixOfEntt(const EntityID id)
{
    // return an inverse world matrix of entt by ID or return a matrix of NANs if there is no such entt by ID
    return pTransform_->invWorlds[GetIdxByID(id)];
}

///////////////////////////////////////////////////////////

void TransformSystem::GetWorldMatricesOfEntts(
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

void TransformSystem::GetInverseWorldMatricesOfEntts(
    const EntityID* ids,
    const int numEntts,
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
    const XMVECTOR& newDirectionQuat,
    const float newScale)
{
    Transform& transformComp = *pTransform_;

    const index idx = GetIdxByID(id);

    if (idx == -1)
    {
        char buf[64];
        sprintf(buf, "there is no entity by ID: %ud", id);
        LogErr(buf);
        return;
    }

    XMFLOAT4& posAndScale = transformComp.posAndUniformScale[idx];

    DirectX::XMStoreFloat4(&posAndScale, newPosition);  // xyz - translation
    posAndScale.w = newScale;                           // w   - contains a uniform scale value
    transformComp.dirQuats[idx] = newDirectionQuat;

    // recompute world matrix and inverse world matrix for this entity
    RecomputeWorldMatrixByIdx(idx);
    RecomputeInvWorldMatrixByIdx(idx);
}

///////////////////////////////////////////////////////////

void TransformSystem::SetTransformDataByIdxs(
    const cvector<index>& idxs,
    const cvector<XMVECTOR>& newPositions,
    const cvector<XMVECTOR>& newDirQuats,
    const cvector<float>& newUniformScales)
{
    const size numElems = idxs.size();
    Assert::NotZero(numElems, "data idxs arr is empty");
    Assert::True(numElems == newPositions.size(), "arr of idxs and arr of positions are not equal");
    Assert::True(numElems == newDirQuats.size(), "arr of idxs and arr of directions are not equal");
    Assert::True(numElems == newUniformScales.size(), "arr of idxs and arr of scales are not equal");

    Transform& comp = *pTransform_;

    // set new positions by idxs
    for (index posIdx = 0; index idx : idxs)
        DirectX::XMStoreFloat4(&comp.posAndUniformScale[idx], newPositions[posIdx]);

    // set new uniform scales by idxs
    for (index scaleIdx = 0; index idx : idxs)
        comp.posAndUniformScale[idx].w = newUniformScales[scaleIdx++];

    // the Transform component stores only normalized direction quaternions so just do it
    for (index quatIdx = 0; index idx : idxs)
        comp.dirQuats[idx] = DirectX::XMQuaternionNormalize(newDirQuats[quatIdx]);
}

///////////////////////////////////////////////////////////

void TransformSystem::SetWorldMatricesByIdxs(
    const cvector<index>& dataIdxs,
    const cvector<XMMATRIX>& newWorldMatrices)
{
    // store world matrices by input data idxs

    Assert::True(pTransform_->worlds.size() >= std::ssize(newWorldMatrices), "count of new matrices can't be bigger than the number of matrices in the component");

    for (index newMatIdx = 0; const index idx : dataIdxs)
        pTransform_->worlds[idx] = newWorldMatrices[newMatIdx++];
}


// =================================================================================
//                            PRIVATE HELPERS
// =================================================================================
void TransformSystem::AddRecordsToTransformComponent(
    const EntityID* ids,
    const XMFLOAT3* positions,
    const XMVECTOR* dirQuats,      // direction quaternions
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


    // normalize all the input direction quaternions
    cvector<XMVECTOR> normDirQuats(numElems);

    for (int i = 0; i < numElems; ++i)
        normDirQuats[i] = DirectX::XMQuaternionNormalize(dirQuats[i]);

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

    // store direction quaternions
    for (index i = 0; i < numElems; ++i)
        comp.dirQuats.insert_before(idxs[i], normDirQuats[i]);

    // compute a world matrix and store it
    // and also compute an inverse world matrix and store it as well
    for (index i = 0; i < numElems; ++i)
    {
        const XMFLOAT3 pos     = positions[i];
        const XMVECTOR dirQuat = normDirQuats[i];
        const float scale      = uniformScales[i];

        // compute a world matrix and store it
        // and also compute an inverse world matrix and store it as well
        const XMMATRIX S = XMMatrixScaling(scale, scale, scale);
        const XMMATRIX R = XMMatrixRotationQuaternion(dirQuat);
        const XMMATRIX T = XMMatrixTranslation(pos.x, pos.y, pos.z);
        const XMMATRIX world = S * R * T;

        comp.worlds.insert_before(idxs[i], world);
        comp.invWorlds.insert_before(idxs[i], XMMatrixInverse(nullptr, world));
    }
}

///////////////////////////////////////////////////////////

index TransformSystem::GetIdxByID(const EntityID id) const
{
    // return valid idx if there is an entity by such ID;
    // or return 0 if there is no such entity;
    const index idx = pTransform_->ids.get_idx(id);

    if (pTransform_->ids[idx] != id)
    {
        char buf[64];
        sprintf(buf, "there is no transform data for entt by id: %ud", id);
        LogErr(buf);
        return 0;
    }

    return idx;
}

} // namespace ECS
