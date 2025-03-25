// *********************************************************************************
// Filename:     Transform.h
// Description:  an ECS component which contains transformation data of entities;
// 
// Created:
// *********************************************************************************
#pragma once


#include "../Common/Types.h"
#include "../Common/cvector.h"

namespace ECS
{

__declspec(align(16)) struct Transform
{
    cvector<XMMATRIX> worlds_;
    cvector<XMMATRIX> invWorlds_;           // inverse world matrices
    cvector<XMFLOAT4> posAndUniformScale_;  // pos (x,y,z); uniform scale (w)
    cvector<XMVECTOR> dirQuats_;            // normalized direction quaternion
    cvector<EntityID> ids_;
	
	ComponentType type_ = ComponentType::TransformComponent;
};

}
