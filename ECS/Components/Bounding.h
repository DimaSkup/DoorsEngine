// =================================================================================
// Filename:     Bounding.h
// Description:  an ECS component stores Bounding data for entities
// 
// Created:      26.09.24
// =================================================================================
#pragma once

#include <Types.h>
#include <cvector.h>
#include <DirectXCollision.h>

namespace ECS
{

//
// HELPER DATA TYPES
//
enum BoundingType
{
    BOUND_SPHERE,
    BOUND_BOX,
};

// ----------------------------------------------

struct BoundingData
{
    BoundingData()
    {
    }

    BoundingData(
        const BoundingType inType,
        const DirectX::BoundingSphere& inSphere,
        const DirectX::BoundingBox& inAABB)
        :
        mainType(inType),
        sphere(inSphere),
        aabb(inAABB)
    {
    }

    // of the whole entity
    BoundingType mainType = BOUND_BOX;

    // around the whole entity
    DirectX::BoundingSphere sphere = { XMFLOAT3(0,0,0), 1 };
    DirectX::BoundingBox    aabb   = { sphere.Center, XMFLOAT3(1.732f, 1.732f, 1.732f) };   // sqrt(3)
}; 


// =================================================================================
// COMPONENT
// =================================================================================
struct Bounding
{
    // Bounding Box:
    // center  - center of the box / sphere; 
    // extents - Distance from the center to each side OR radius of the sphere

    cvector<EntityID>     ids;
    cvector<BoundingData> data;
};


} // namespace ECS
