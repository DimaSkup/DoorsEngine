// =================================================================================
// Filename:       Camera.h
// Description     an ECS component for storing data of multiple cameras
// 
// Created:        14.01.25   by DimaSkup
// =================================================================================
#pragma once

#include "../Common/Types.h"
#include <vector>

namespace ECS
{

struct Camera
{
	Camera()
	{
		// push empty invalid data
		ids_.push_back(INVALID_ENTITY_ID);
		views_.push_back(DirectX::XMMatrixIdentity());
		projs_.push_back(DirectX::XMMatrixIdentity());
		invViews_.push_back(DirectX::XMMatrixIdentity());
	}

	ComponentType type_ = ComponentType::CameraComponent;

	std::vector<EntityID> ids_;          // id of entt which has a camera
	std::vector<XMMATRIX> views_;         // view matrix of the camera
	std::vector<XMMATRIX> projs_;         // projection matrix of the camera

	std::vector<XMMATRIX> invViews_;      // current inverse view matrix
};

}