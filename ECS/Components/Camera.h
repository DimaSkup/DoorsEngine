// =================================================================================
// Filename:       Camera.h
// Description     an ECS component for storing data of UVN cameras data
// 
// Created:        14.01.25   by DimaSkup
// =================================================================================
#pragma once

#include "../Common/ECSTypes.h"
#include <Types.h>
#include <map>


namespace ECS
{

// camera's data container
struct CameraData
{
    // cache frustum properties
    float fovY          = -1.0f;
    float aspectRatio   = -1.0f;      // viewport width / viewport height
    float nearZ         = -1.0f;
    float farZ          = -1.0f;

    float nearWndHeight = -1.0f;
    float farWndHeight  = -1.0f;

    XMMATRIX baseView = DirectX::XMMatrixIdentity();
    XMMATRIX view     = DirectX::XMMatrixIdentity();
    XMMATRIX invView  = DirectX::XMMatrixIdentity();
    XMMATRIX proj     = DirectX::XMMatrixIdentity();   // projection matrix
    XMMATRIX ortho    = DirectX::XMMatrixIdentity();   // orthographic matrix

    // camera coordinate system with coordinates relative to world space
#if 0
    XMVECTOR position;                // is stored in the Transform component
    XMVECTOR direction;               // is stored in the Transform component
#endif

    XMVECTOR right      { 1,0,0 };    // world space right vector of the camera
    //XMVECTOR up         { 0,1,0 };    // camera's up vector
    XMVECTOR target{ 0,1,0 };    // world space look_at point (is used when we concentrate camera on some point)

    // defines if camera is fixed at some particular look_at point
    bool isFixedLook_ = false;
};

///////////////////////////////////////////////////////////

// ECS component
struct Camera
{
    Camera()
    {
        data.emplace(0, CameraData());
    }

    std::map<EntityID, CameraData> data;
};

}
