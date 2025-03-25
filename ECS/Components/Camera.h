// =================================================================================
// Filename:       Camera.h
// Description     an ECS component for storing data of multiple cameras
// 
// Created:        14.01.25   by DimaSkup
// =================================================================================
#pragma once

#include "../Common/Types.h"
#include "../Common/cvector.h"


namespace ECS
{

struct Camera
{
    Camera()
    {
        // push empty invalid data
        ids.push_back(INVALID_ENTITY_ID);
        views.push_back(DirectX::XMMatrixIdentity());
        projs.push_back(DirectX::XMMatrixIdentity());
        invViews.push_back(DirectX::XMMatrixIdentity());
    }


    cvector<EntityID> ids;           // id of entt which has a camera
    cvector<XMMATRIX> views;         // view matrix of the camera
    cvector<XMMATRIX> projs;         // projection matrix of the camera
    cvector<XMMATRIX> invViews;      // current inverse view matrix

    ComponentType type = ComponentType::CameraComponent;

};

}
