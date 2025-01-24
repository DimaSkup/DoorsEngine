// *********************************************************************************
// Filename:     Transform.h
// Description:  an ECS component which contains transformation data of entities;
// 
// Created:
// *********************************************************************************
#pragma once


#include "../Common/Types.h"
#include <vector>

namespace ECS
{

struct Transform
{
	Transform()
	{
		// reserve ahead some memory
		const u32 newCapacity = 128;

		ids_.reserve(newCapacity);
		posAndUniformScale_.reserve(newCapacity);
		dirQuats_.reserve(newCapacity);
	}

	ComponentType type_ = ComponentType::TransformComponent;

	std::vector<EntityID> ids_; 
	std::vector<XMFLOAT4> posAndUniformScale_;  // pos (x,y,z); uniform scale (w)
	std::vector<XMVECTOR> dirQuats_;            // normalized direction quaternion

};

}
