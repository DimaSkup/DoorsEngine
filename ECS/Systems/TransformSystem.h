// **********************************************************************************
// Filename:      TranformSystem.h
// Description:   Entity-Component-System (ECS) system for handling 
//                transform data of entities
// 
// Created:       20.05.24
// **********************************************************************************
#pragma once
#include "../Common/ECSTypes.h"
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

    void GetPositions    (const EntityID* ids, const size numEntts, cvector<XMFLOAT3>& outPositions);
    void GetDirections   (const EntityID* ids, const size numEntts, cvector<XMVECTOR>& outDirections);
    void GetUniformScales(const EntityID* ids, const size numEntts, cvector<float>& outScales);

    XMFLOAT3  GetPosition      (const EntityID id) const;
    XMFLOAT4  GetPositionFloat4(const EntityID id) const;
    XMVECTOR  GetPositionVec   (const EntityID id) const;

    const XMVECTOR& GetDirectionVec(const EntityID id) const;
    const XMFLOAT3  GetDirection   (const EntityID id) const;
    const float     GetScale       (const EntityID id) const;
    const XMVECTOR  GetRotationQuat(const EntityID id) const;

    // get position and direction for multiple entities / single entity
    void GetPositionsAndDirections(
        const EntityID* ids,
        const size numEntts,
        cvector<XMFLOAT3>& outPositions,
        cvector<XMFLOAT3>& outDirections);

    void GetPosAndDir(const EntityID id, XMVECTOR& outPos, XMVECTOR& outDir) const;


    // SET position/direction/uniform_scale
    bool AdjustPositions(const EntityID* ids, const size numEntts, const XMFLOAT3& adjustBy);
    bool AdjustPosition (const EntityID id, const XMFLOAT3& offset);
    bool SetPositionVec (const EntityID id, const XMVECTOR& pos);
    bool SetPosition    (const EntityID id, float x, float y, float z);
    bool SetPosition    (const EntityID id, const XMFLOAT3& pos);
    bool SetDirection   (const EntityID id, const XMVECTOR& dir);
    bool SetScale       (const EntityID id, const float scale);

    bool SetPositions(const EntityID* ids, const size numEntts, const XMFLOAT3* positions);

    bool RotateLocalSpacesByQuat(const EntityID* ids, const size numEntts, const XMVECTOR& quat);
    bool RotateLocalSpaceByQuat (const EntityID id, const XMVECTOR& rotQuat);

    // ---------------------------------------------

    void TransformWorld(const EntityID id, const XMMATRIX& transformation);

    const DirectX::XMMATRIX& GetWorld       (const EntityID id) const;
    const DirectX::XMMATRIX& GetInvWorld(const EntityID id) const;

    void GetWorlds(
        const EntityID* ids,
        const size numEntts,
        cvector<DirectX::XMMATRIX>& outWorlds);

    void GetInverseWorlds(
        const EntityID* ids,
        const size numEntts,
        cvector<DirectX::XMMATRIX>& outInvWorlds);


private:
    bool AddRecordsHelper(
        const EntityID* ids,
        const XMFLOAT3* positions,
        const XMVECTOR* directions,
        const float* uniformScales,
        const size numElems);

    index GetIdx(const EntityID id) const;

    void     RecalcInvWorldMatrixByIdx(const index idx);
    XMMATRIX GetMatTranslation        (const XMFLOAT4& pos) const;
    XMMATRIX GetMatRotation           (const XMVECTOR& rotQuat) const;
    XMMATRIX GetMatScaling            (const float scale) const;

private:
    Transform* pTransform_ = nullptr;   // a ptr to the Transform component
};

//==================================================================================
// inline functions
//==================================================================================

//-----------------------------------------------------
// Desc:  recompute inverse world matrix based on world by array idx;
// NOTE:  expects the world matrix to be computed already!!!
//-----------------------------------------------------
inline void TransformSystem::RecalcInvWorldMatrixByIdx(const index idx)
{
    pTransform_->invWorlds[idx] = DirectX::XMMatrixInverse(nullptr, pTransform_->worlds[idx]);
}

//-----------------------------------------------------
// get matrices
//-----------------------------------------------------
inline XMMATRIX TransformSystem::GetMatTranslation(const XMFLOAT4& pos) const
{
    return DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
}

inline XMMATRIX TransformSystem::GetMatRotation(const XMVECTOR& direction) const
{
    return DirectX::XMMatrixRotationQuaternion(direction);
}

inline XMMATRIX TransformSystem::GetMatScaling(const float uScale) const
{
    return DirectX::XMMatrixScaling(uScale, uScale, uScale);
}

}
