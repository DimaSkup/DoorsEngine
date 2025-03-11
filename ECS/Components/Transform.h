// *********************************************************************************
// Filename:     Transform.h
// Description:  an ECS component which contains transformation data of entities;
// 
// Created:
// *********************************************************************************
#pragma once


#include "../Common/Types.h"
#include "../Common/vector.h"

namespace ECS
{

__declspec(align(16)) struct Transform
{
	Transform()
	{
		// reserve ahead some memory
		//const u32 newCapacity = 128;

		//ids_.reserve(newCapacity);
		//posAndUniformScale_.reserve(newCapacity);
		//dirQuats_.reserve(newCapacity);
	}

	vector<XMMATRIX> worlds_;
	vector<XMMATRIX> invWorlds_;           // inverse world matrices
	vector<XMFLOAT4> posAndUniformScale_;  // pos (x,y,z); uniform scale (w)
	vector<XMVECTOR> dirQuats_;            // normalized direction quaternion
	vector<EntityID> ids_;
	

	ComponentType type_ = ComponentType::TransformComponent;
};

}
