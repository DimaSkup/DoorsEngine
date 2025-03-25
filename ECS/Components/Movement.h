// *********************************************************************************
// Filename:     Movement.h
// Description:  an ECS component which contains movement data of entities;
// 
// Created:
// *********************************************************************************
#pragma once


#include "../Common/Types.h"
#include "../Common/cvector.h"


namespace ECS
{

struct Movement
{
	cvector<EntityID> ids_;                     // entities IDs
	cvector<XMFLOAT4> translationAndUniScales_; // translation (x,y,z); uniform scale (w)
	cvector<XMVECTOR> rotationQuats_;           // rotation quatertion {0, pitch, yaw, roll}

    ComponentType type_ = ComponentType::MoveComponent;
};

}
