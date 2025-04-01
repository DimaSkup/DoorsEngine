// =================================================================================
// Filename:      TranformSystem.cpp
// Description:   implementation of the TransformSystem's functional
// 
// Created:       20.05.24
// =================================================================================
#include "TransformSystem.h"

#include "../Common/Assert.h"
#include "../Common/MathHelper.h"
#include "SaveLoad/TransformSysSerDeser.h"

#include <stdexcept>
#include <algorithm>

using namespace DirectX;


namespace ECS
{

TransformSystem::TransformSystem()
{
}

TransformSystem::~TransformSystem()
{
}

// =================================================================================
// PUBLIC METHODS
// =================================================================================

void TransformSystem::Initialize(Transform* pTransform)
{
    Assert::NotNullptr(pTransform, "ptr to the Transform component == nullptr");
    pTransform_ = pTransform;

    // add invalid data; this data is returned when we ask for wrong entity
    pTransform_->ids_.push_back(INVALID_ENTITY_ID);
    pTransform_->posAndUniformScale_.push_back(XMFLOAT4{ NAN, NAN, NAN, NAN });
    pTransform_->dirQuats_.push_back(XMVECTOR{ NAN, NAN, NAN, NAN });

    const cvector<float> nanArray(16, NAN);
    XMMATRIX nanMatrix(nanArray.data());

    pTransform_->worlds_.push_back(nanMatrix);
    pTransform_->invWorlds_.push_back(nanMatrix); // inverse world matrix
}

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

///////////////////////////////////////////////////////////

void TransformSystem::Serialize(std::ofstream& fout, u32& offset)
{
}

///////////////////////////////////////////////////////////

void TransformSystem::Deserialize(std::ifstream& fin, const u32 offset)
{
}


// =================================================================================
// GET position/rotation/uniform_scale
// =================================================================================

void TransformSystem::GetPositionsByIDs(
    const EntityID* ids,
    XMFLOAT3* outPositions,
    const size numEntts) const
{
    // get positions of entities by input IDs array;
    // 
    // NOTE: it is supposed that ids.size() == outPositions.size()

    Assert::True((ids) && (outPositions) && (numEntts > 0), "invalid input args");

    Transform& comp = *pTransform_;
    cvector<index> idxs;

    comp.ids_.get_idxs(ids, numEntts, idxs);

    for (int i = 0; const index idx : idxs)
    {
        XMFLOAT4& data = comp.posAndUniformScale_[idx];   // pos (float3) + scale (float)
        outPositions[i++] = { data.x, data.y, data.z };
    }
}

///////////////////////////////////////////////////////////

const XMFLOAT3 TransformSystem::GetPositionByID(const EntityID id) const
{
    const XMFLOAT4& pos = pTransform_->posAndUniformScale_[GetIdxByID(id)];
    return XMFLOAT3(pos.x, pos.y, pos.z);
}

///////////////////////////////////////////////////////////

const XMVECTOR TransformSystem::GetRotationQuatByID(const EntityID id) const
{
    return pTransform_->dirQuats_[GetIdxByID(id)];
}

///////////////////////////////////////////////////////////

const XMFLOAT3 TransformSystem::GetRotationPitchYawRollByID(const EntityID id) const
{
    const index idx = GetIdxByID(id);

    if (idx == 0)
    {
        // if there is no transformation data for entity by ID
        return XMFLOAT3{ NAN, NAN, NAN };
    }
    else
    {
        // convert quaternion into Euler angles (in order: pitch, yaw, roll)
        return MathHelper::QuatToEulerAngles(pTransform_->dirQuats_[idx]);
    }
}

///////////////////////////////////////////////////////////

const float TransformSystem::GetUniformScaleByID(const EntityID id) const
{
    return pTransform_->posAndUniformScale_[GetIdxByID(id)].w;
}


// =================================================================================
// SET position/rotation/uniform_scale
// =================================================================================

bool TransformSystem::SetPositionByID(const EntityID id, const XMFLOAT3& pos)
{
    const index idx = GetIdxByID(id);

    // if there is no transformation data for entity by ID
    if (idx == 0)
        return false;

    Transform& comp = *pTransform_;
    XMFLOAT4& data = comp.posAndUniformScale_[idx];

    // position is stored in x,y,z components (w-component stores the uniform scale)
    data.x = pos.x;
    data.y = pos.y;
    data.z = pos.z;

    // recompute world matrix and inverse world matrix for this entity
    RecomputeWorldMatrixByIdx(idx);
    RecomputeInvWorldMatrixByIdx(idx);

    return true;
}

///////////////////////////////////////////////////////////

bool TransformSystem::SetRotationQuatByID(const EntityID id, const XMVECTOR& dirQuat)
{
    // update rotation of entity using input rotation quaternion

    const index idx = GetIdxByID(id);

    // if there is no transformation data for entity by ID
    if (idx == 0)
        return false;

    pTransform_->dirQuats_[idx] = XMQuaternionNormalize(dirQuat);

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
    pTransform_->posAndUniformScale_[idx].w = uniformScale;

    // recompute world matrix and inverse world matrix for this entity
    RecomputeWorldMatrixByIdx(idx);
    RecomputeInvWorldMatrixByIdx(idx);

    return true;
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
    const XMFLOAT4& data = pTransform_->posAndUniformScale_[idx];

    dirQuat = pTransform_->dirQuats_[idx];
    pos = { data.x, data.y, data.z };
    uniformScale = data.w;
}

///////////////////////////////////////////////////////////

DirectX::XMMATRIX TransformSystem::GetWorldMatrixOfEntt(const EntityID id)
{
    // return a world matrix of entt by ID or return a matrix of NANs if there is no such entt by ID
    return pTransform_->worlds_[GetIdxByID(id)];
}

///////////////////////////////////////////////////////////

const DirectX::XMMATRIX& TransformSystem::GetInverseWorldMatrixOfEntt(const EntityID id)
{
    // return an inverse world matrix of entt by ID or return a matrix of NANs if there is no such entt by ID
    return pTransform_->invWorlds_[GetIdxByID(id)];
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
    comp.ids_.get_idxs(ids, numEntts, idxs);
    comp.worlds_.get_data_by_idxs(idxs, outWorlds);
}

///////////////////////////////////////////////////////////

void TransformSystem::GetInverseWorldMatricesOfEntts(
    const EntityID* ids,
    DirectX::XMMATRIX* outInvWorlds,
    const int numEntts)
{
    // NOTE: size of arrays enttsIDs and outInvWorlds must be equal !!!
    Assert::True((ids != nullptr) && (outInvWorlds != nullptr) && (numEntts > 0), "invalid input data");

    const Transform& comp = *pTransform_;
    cvector<index> idxs(numEntts);
    cvector<XMMATRIX> invWorlds(numEntts);

    // get data idx by each ID and then get inverse world matrices by these idxs
    comp.ids_.get_idxs(ids, numEntts, idxs);
    comp.invWorlds_.get_data_by_idxs(idxs, invWorlds);
    memcpy(outInvWorlds, invWorlds.data(), invWorlds.size() * sizeof(XMMATRIX));
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
    const XMVECTOR& newRotation,
    const float newScale)
{
    Transform& transformComp = *pTransform_;
    //WorldMatrix& worldComp   = *pWorldMat_;

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

void TransformSystem::SetTransformDataByDataIdxs(
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
        DirectX::XMStoreFloat4(&comp.posAndUniformScale_[idx], newPositions[posIdx]);

    // set new uniform scales by idxs
    for (index scaleIdx = 0; index idx : idxs)
        comp.posAndUniformScale_[idx].w = newUniformScales[scaleIdx++];

    // the Transform component stores only normalized direction quaternions so just do it
    for (index quatIdx = 0; index idx : idxs)
        comp.dirQuats_[idx] = DirectX::XMQuaternionNormalize(newDirQuats[quatIdx]);
}

///////////////////////////////////////////////////////////

void TransformSystem::SetWorldMatricesByDataIdxs(
    const cvector<index>& dataIdxs,
    const cvector<XMMATRIX>& newWorldMatrices)
{
    // store world matrices by input data idxs

    Assert::True(pTransform_->worlds_.size() >= std::ssize(newWorldMatrices), "count of new matrices can't be bigger than the number of matrices in the component");

    for (index newMatIdx = 0; const index idx : dataIdxs)
        pTransform_->worlds_[idx] = newWorldMatrices[newMatIdx++];
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

    bool canAddComponent = !comp.ids_.binary_search(ids, numElems);
    Assert::True(canAddComponent, "can't add component: there is already a record with some entity id");

    cvector<index> idxs;
    comp.ids_.get_insert_idxs(ids, numElems, idxs);


    // normalize all the input direction quaternions
    cvector<XMVECTOR> normDirQuats(numElems);

    for (int i = 0; i < numElems; ++i)
        normDirQuats[i] = DirectX::XMQuaternionNormalize(dirQuats[i]);

    // store ids
    for (index i = 0; i < numElems; ++i)
        comp.ids_.insert_before(idxs[i] + i, ids[i]);

    // store positions
    for (index i = 0; i < numElems; ++i)
    {
        const XMFLOAT3& pos = positions[i];
        comp.posAndUniformScale_.insert_before(idxs[i] + i, { pos.x, pos.y, pos.z, 1.0f });
    }

    // we store uniform scale values in the w-component of float4
    for (index i = 0; i < numElems; ++i)
        comp.posAndUniformScale_[idxs[i] + i].w = uniformScales[i];

    // store direction quaternions
    for (index i = 0; i < numElems; ++i)
        comp.dirQuats_.insert_before(idxs[i] + i, normDirQuats[i]);

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

        comp.worlds_.insert_before(idxs[i] + i, world);
        comp.invWorlds_.insert_before(idxs[i] + i, XMMatrixInverse(nullptr, world));
    }
}

} // namespace ECS
