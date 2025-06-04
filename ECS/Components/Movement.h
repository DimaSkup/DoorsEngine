// *********************************************************************************
// Filename:     Movement.h
// Description:  an ECS component which contains movement data of entities;
// 
// Created:
// *********************************************************************************
#pragma once

#include <Types.h>
#include <cvector.h>


namespace ECS
{

struct Movement
{
	cvector<EntityID> ids_;                              // entities IDs
	cvector<DirectX::XMFLOAT4> translationAndUniScales_; // translation (x,y,z); uniform scale (w)
	cvector<DirectX::XMVECTOR> rotationQuats_;           // rotation quatertion {0, pitch, yaw, roll}
};

}
