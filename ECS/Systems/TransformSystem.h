// **********************************************************************************
// Filename:      TranformSystem.h
// Description:   Entity-Component-System (ECS) system for handling 
//                transform data of entities
// 
// Created:       20.05.24
// **********************************************************************************
#pragma once

#include "../Components/Transform.h"


namespace ECS
{

class TransformSystem
{
public:
    TransformSystem(Transform* pTransform);
    ~TransformSystem();


    void AddRecords(
        const EntityID* ids,
        const XMFLOAT3* positions,
        const XMVECTOR* directions,      
        const float* uniformScales,
        const size numElems);

    void RemoveRecords(const cvector<EntityID>& enttsIDs);

    // -------------------------------------------------------

    // GET position/direction/uniform_scale

    void GetPositions    (const EntityID* ids, const size numEntts, cvector<XMFLOAT3>& outPositions)  const;
    void GetDirections   (const EntityID* ids, const size numEntts, cvector<XMVECTOR>& outDirections) const;
    void GetUniformScales(const EntityID* ids, const size numEntts, cvector<float>& outScales)        const;

    const XMFLOAT3  GetPosition     (const EntityID id) const;
    const XMVECTOR  GetPositionVec  (const EntityID id) const;
    const XMVECTOR& GetDirectionVec (const EntityID id) const;
    const XMFLOAT3  GetDirection    (const EntityID id) const;
    const float     GetUniformScale (const EntityID id) const;

    // get position and direction for multiple entities / single entity
    void GetPositionsAndDirections(
        const EntityID* ids,
        const size numEntts,
        cvector<XMFLOAT3>& outPositions,
        cvector<XMFLOAT3>& outDirections) const;

    void GetPosAndDir (
        const EntityID id,
        XMVECTOR& outPos,
        XMVECTOR& outDir) const;


    // SET position/direction/uniform_scale
    bool SetPositionVec(const EntityID id, const XMVECTOR& pos);
    bool SetPosition   (const EntityID id, const XMFLOAT3& pos);
    bool SetDirection  (const EntityID id, const XMVECTOR& direction);
    bool SetUniScale   (const EntityID id, const float uniformScale);

    bool RotateWorldByQuat(const EntityID id, const XMVECTOR& quat);

    // ---------------------------------------------

    DirectX::XMMATRIX GetWorldMatrixOfEntt(const EntityID id);

    void GetWorlds(
        const EntityID* ids,
        const size numEntts,
        cvector<DirectX::XMMATRIX>& outWorlds);

    // ----------------------------------------------------

    const DirectX::XMMATRIX& GetInverseWorld(const EntityID id);

    void GetInverseWorlds(
        const EntityID* ids,
        const size numEntts,
        cvector<DirectX::XMMATRIX>& outInvWorlds);


private:
    void AddRecordsToTransformComponent(
        const EntityID* ids,
        const XMFLOAT3* positions,
        const XMVECTOR* dirQuats,      // direction quaternions
        const float* uniformScales,
        const size numElems);

    // ---------------------------------------------

    index GetIdx(const EntityID id) const;

    inline void RecomputeWorldMatrixByIdx(const index idx)
    {
        // recompute world matrix for the entity by array idx
        pTransform_->worlds[idx] =
            GetMatScaling(pTransform_->posAndUniformScale[idx].w) *
            GetMatRotation(pTransform_->directions[idx]) *
            GetMatTranslation(pTransform_->posAndUniformScale[idx]);
    }

    inline void RecomputeInvWorldMatrixByIdx(const index idx)
    {
        // recompute inverse world matrix based on world by array idx;
        // NOTE: expects the world matrix to be computed already!!!
        pTransform_->invWorlds[idx] = DirectX::XMMatrixInverse(nullptr, pTransform_->worlds[idx]);
    }

    inline XMMATRIX GetMatTranslation(const XMFLOAT4& pos)    const { return DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z); }
    inline XMMATRIX GetMatRotation(const XMVECTOR& direction) const { return DirectX::XMMatrixRotationQuaternion(direction); }
    inline XMMATRIX GetMatScaling(const float uScale)         const { return DirectX::XMMatrixScaling(uScale, uScale, uScale); }

private:
    Transform* pTransform_ = nullptr;   // a ptr to the Transform component
};

}
