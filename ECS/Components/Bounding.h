// =================================================================================
// Filename:     Bounding.h
// Description:  an ECS component stores Bounding data for entities
// 
// Created:      26.09.24
// =================================================================================
#pragma once

#include "../Common/Types.h"
#include "../Common/cvector.h"
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
		const size numData,
		const BoundingType* types,             // AABB type per mesh
		const DirectX::BoundingBox* AABBs) :
		numData(numData),
		types(types, types + numData)
	{
		obbs.resize(numData);

		// convert each input AABB into OBB
		for (index i = 0; i < numData; ++i)
			DirectX::BoundingOrientedBox::CreateFromBoundingBox(obbs[i], AABBs[i]);
	}

	size numData = 0;
    cvector<BoundingType> types;                   // types: AABB/sphere
	cvector<DirectX::BoundingOrientedBox> obbs;    // center, extents, rotation
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

    ComponentType componentType_ = ComponentType::BoundingComponent;
};


} // namespace ECS
