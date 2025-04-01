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

#include "../Common/Types.h"
#include "../Common/cvector.h"

namespace ECS
{

struct MaterialData
{
    MaterialData() {}

    MaterialData(const MaterialID* ids, const size numMaterials)
    {
        materialsIDs.resize(numMaterials);
        std::copy(ids, ids + numMaterials, materialsIDs.begin());
    }

    cvector<MaterialID> materialsIDs;
};

// --------------------------------------------------------

// ECS component
struct Material
{
    cvector<EntityID>     enttsIDs;
    cvector<MaterialData> data;

    // a flag to define if all the materials (MaterialData) which are related to entity
    // are based on related model (means related to entity)
    cvector<bool>         flagsMeshBasedMaterials;  

    eComponentType        type = MaterialComponent;
};

}
