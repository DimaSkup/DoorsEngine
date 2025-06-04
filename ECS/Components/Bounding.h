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
	SPHERE,
	BOUND_BOX,
};

// ----------------------------------------------

struct BoundingData
{
	// contains bounding data for each subset (mesh / submesh) of the entity
	// NOTE: idx == 0 means subset_0
	//       idx == 1 means subset_1
	//       etc.

    BoundingData() {};

	BoundingData(
        DirectX::BoundingSphere sphere,        // around the whole entity
		const size numData,
		const BoundingType* types,             // AABB type per mesh
		const DirectX::BoundingBox* AABBs) :
        boundSphere(sphere),
		types(types, types + numData)
	{
		obbs.resize(numData);

		// convert each input AABB into OBB
		for (index i = 0; i < numData; ++i)
			DirectX::BoundingOrientedBox::CreateFromBoundingBox(obbs[i], AABBs[i]);
	}

    DirectX::BoundingSphere boundSphere;           // around the whole entity


    cvector<BoundingType> types;                   // type per mesh: AABB/sphere
	cvector<DirectX::BoundingOrientedBox> obbs;    // per mesh: center, extents, rotation
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
