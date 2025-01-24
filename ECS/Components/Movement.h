// *********************************************************************************
// Filename:     Movement.h
// Description:  an ECS component which contains movement data of entities;
// 
// Created:
// *********************************************************************************
#pragma once


#include "../Common/Types.h"
#include <vector>


namespace ECS
{

struct Movement
{
	ComponentType type_ = ComponentType::MoveComponent;
	
	std::vector<EntityID> ids_;                     // entities IDs
	std::vector<XMFLOAT4> translationAndUniScales_; // translation (x,y,z); uniform scale (w)
	std::vector<XMVECTOR> rotationQuats_;           // rotation quatertion {0, pitch, yaw, roll}
};

}
