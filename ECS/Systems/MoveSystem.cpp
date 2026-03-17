// ================================================================================
// Filename:      MoveSystem.h
// Description:   implementation of the MoveSystem's functional
// 
// Created:       23.05.24
// ================================================================================
#include "../Common/pch.h"
#include "MoveSystem.h"


namespace ECS
{
 

MoveSystem::MoveSystem(Transform* pTransformComponent, Movement* pMoveComponent)
{
    CAssert::NotNullptr(pTransformComponent, "ptr to the Transform component == nullptr");
    CAssert::NotNullptr(pMoveComponent, "ptr to the Movement component == nullptr");

    pTransformComponent_ = pTransformComponent;
    pMoveComponent_ = pMoveComponent;
}


// ================================================================================
//                PUBLIC SERIALIZATION / DESERIALIZATION API
// ================================================================================
void MoveSystem::Serialize(std::ofstream& fout, uint32& offset)
{
}

///////////////////////////////////////////////////////////

void MoveSystem::Deserialize(std::ifstream& fin, const uint32 offset)
{
}


// ================================================================================
//                              PUBLIC UPDATING API
// ================================================================================
void MoveSystem::UpdateAllMoves(
    const float deltaTime,
    TransformSystem& transformSys)
{
    const cvector<EntityID>& enttsToMove = pMoveComponent_->ids_;

    // if we don't have any entities to move we just go out
    if (enttsToMove.size() == 0)
        return;
}


// ================================================================================
//                      PUBLIC CREATION / DELETING API
// ================================================================================
void MoveSystem::AddRecords(
    const EntityID* ids,
    const XMFLOAT3* translations,
    const XMVECTOR* rotationQuats,
    const float* uniformScaleFactors,
    const size numEntts)
{
    Movement& comp = *pMoveComponent_;
    cvector<XMFLOAT4> packedTrScales(numEntts);
    cvector<XMVECTOR> normRotQuats(numEntts);
    const XMFLOAT3* tr = translations;


    // pack translation and uniformScale into a single XMFLOAT4
    for (index i = 0; i < numEntts; ++i)
    {
        const XMFLOAT3& t = translations[i];
        packedTrScales[i] = { t.x, t.y, t.z, 1.0f };
    }

    for (index i = 0; i < numEntts; ++i)
        packedTrScales[i].w = uniformScaleFactors[i];

    // normalize each input rotation quaternion
    for (index i = 0; i < numEntts; ++i)
        normRotQuats[i] = DirectX::XMQuaternionNormalize(rotationQuats[i]);


    cvector<index> idxs;
    comp.ids_.get_insert_idxs(ids, numEntts, idxs);


    // allocate ahead more memory
    const size newCapacity = comp.ids_.size() + numEntts;
    comp.ids_.reserve(newCapacity);
    comp.translationAndUniScales_.reserve(newCapacity);
    comp.rotationQuats_.reserve(newCapacity);


    // execute sorted insertion into the data arrays
    for (index i = 0; i < numEntts; ++i)
        comp.ids_.insert_before(idxs[i] + i, ids[i]);

    for (index i = 0; i < numEntts; ++i)
        comp.translationAndUniScales_.insert_before(idxs[i] + i, packedTrScales[i]);

    for (index i = 0; i < numEntts; ++i)
        comp.rotationQuats_.insert_before(idxs[i] + i, normRotQuats[i]);
}

///////////////////////////////////////////////////////////

void MoveSystem::RemoveRecords(const cvector<EntityID>& enttsIDs)
{
    throw EngineException("TODO: IMPLEMENT IT!");
}

}
