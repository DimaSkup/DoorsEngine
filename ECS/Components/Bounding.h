// =================================================================================
// Filename:     Bounding.h
// Description:  an ECS component stores Bounding data for entities
// 
// Created:      26.09.24
// =================================================================================
#pragma once

#include <types.h>
#include <cvector.h>
#include <DirectXCollision.h>

namespace ECS
{

// =================================================================================
// HELPER DATA TYPES
// =================================================================================
struct BoundData
{
    BoundData()
    {
    }

    BoundData(
        const DirectX::BoundingBox& _localBox,
        const DirectX::BoundingBox& _worldBox,
        const DirectX::BoundingSphere& _localSphere,
        const DirectX::BoundingSphere& _worldSphere)
        :
        localBox(_localBox),
        worldBox(_worldBox),
        localSphere(_localSphere),
        worldSphere(_worldSphere)
    {
    }

    DirectX::BoundingBox    localBox;
    DirectX::BoundingBox    worldBox;
    DirectX::BoundingSphere localSphere;
    DirectX::BoundingSphere worldSphere;
}; 

// =================================================================================
// ECS COMPONENT
// =================================================================================
struct Bounding
{
    Bounding()
    {
        // add "invalid" bounding shape
        ids.push_back(INVALID_ENTT_ID);
        data.push_back(BoundData());
    }

    cvector<EntityID>  ids;
    cvector<BoundData> data;
};


} // namespace ECS
