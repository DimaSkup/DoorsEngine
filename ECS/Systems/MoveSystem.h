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
#include "../Components/WorldMatrix.h"

// systems
#include "TransformSystem.h"

namespace ECS
{

class MoveSystem final
{
public:
	MoveSystem(
		Transform* pTransformComponent,
		WorldMatrix* pWorldMatComponent_,
		Movement* pMoveComponent);
	~MoveSystem() {}

	void Serialize(std::ofstream& fout, u32& offset);
	void Deserialize(std::ifstream& fin, const u32 offset);

	void UpdateAllMoves(const float deltaTime, TransformSystem& transformSys);

	void AddRecords(
		const std::vector<EntityID>& enttsIDs,
		const std::vector<XMFLOAT3>& translations,
		const std::vector<XMVECTOR>& rotationQuats,
		const std::vector<float>& uniformScaleChanges);

	void RemoveRecords(const std::vector<EntityID>& enttsIDs);

	inline void GetEnttsIDsFromMoveComponent(std::vector<EntityID>& outEnttsIDs) { outEnttsIDs = pMoveComponent_->ids_; }

private:
	void CheckCanAddRecords(const std::vector<EntityID>& ids);

private:
	Transform*   pTransformComponent_ = nullptr;
	WorldMatrix* pWorldMatComponent_ = nullptr;
	Movement*    pMoveComponent_ = nullptr;
};

}