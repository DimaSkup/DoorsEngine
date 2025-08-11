// *********************************************************************************
// Filename:     Material.h
// Description:  an ECS component which contains material data of entities;
//               each entity can have multiple subsets (meshes)
//               and each subset can have its own unique material;
//               so we create an array of materials IDs for each entity by ID;
// 
// Created:      28.06.24
// *********************************************************************************
#pragma once

#include <Types.h>
#include <cvector.h>

namespace ECS
{

//---------------------------------------------------------
// Desc:   set of materials per entity
//         (where each subset/mesh of the model has its own material)
//---------------------------------------------------------
struct MaterialData
{
    MaterialData() {}

    MaterialData(const MaterialID* ids, const size numMaterials)
    {
        assert((ids != nullptr) && (numMaterials > 0) && "invalid input data");

        materialsIds.resize(numMaterials);
        std::copy(ids, ids + numMaterials, materialsIds.begin());
    }

    cvector<MaterialID> materialsIds;
};

//---------------------------------------------------------
// ECS component
//---------------------------------------------------------
struct Material
{
    cvector<EntityID>     enttsIDs;
    cvector<MaterialData> data;
};

}
