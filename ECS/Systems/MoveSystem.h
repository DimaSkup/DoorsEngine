// ********************************************************************************
// Filename:      MoveSystem.h
// Description:   Entity-Component-System (ECS) system
//                for controlling of the entities movement;
// 
// Created:       .05.24
// ********************************************************************************
#pragma once

// components
#include "../Components/Movement.h"
#include "../Components/Transform.h"

// systems
#include "TransformSystem.h"

namespace ECS
{

class MoveSystem final
{
public:
	MoveSystem(
		Transform* pTransformComponent,
		Movement* pMoveComponent);
	~MoveSystem() {}

	void Serialize(std::ofstream& fout, u32& offset);
	void Deserialize(std::ifstream& fin, const u32 offset);

	void UpdateAllMoves(const float deltaTime, TransformSystem& transformSys);

    void AddRecords(
        const EntityID* ids,
        const XMFLOAT3* translations,
        const XMVECTOR* rotationQuats,
        const float* uniformScaleFactors,
        const size numEntts);

	void RemoveRecords(const cvector<EntityID>& enttsIDs);

	inline void GetEnttsIDsFromMoveComponent(cvector<EntityID>& outEnttsIDs) { outEnttsIDs = pMoveComponent_->ids_; }

private:
	Transform*   pTransformComponent_ = nullptr;
	Movement*    pMoveComponent_ = nullptr;
};

}
