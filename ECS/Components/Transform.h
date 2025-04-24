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
    cvector<EntityID> ids;
    cvector<XMMATRIX> worlds;
    cvector<XMMATRIX> invWorlds;           // inverse world matrices
    cvector<XMFLOAT4> posAndUniformScale;  // pos (x,y,z); uniform scale (w)
    cvector<XMVECTOR> directions;          // normalized direction vector
	
	eComponentType type = eComponentType::TransformComponent;
};

}
