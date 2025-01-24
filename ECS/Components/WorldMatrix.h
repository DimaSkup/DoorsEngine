// *********************************************************************************
// Filename:     WorldMatrix.h
// Description:  an ECS component which contains world matrices of entities;
// 
// Created:      08.06.24
// *********************************************************************************
#pragma once

#include "../Common/Types.h"
#include <vector>

namespace ECS
{

struct WorldMatrix
{
	ComponentType type_ = ComponentType::WorldMatrixComponent;

	std::vector<EntityID> ids_;
	std::vector<XMMATRIX> worlds_;
	std::vector<XMMATRIX> invWorlds_;      // inverse world matrices
};

}
