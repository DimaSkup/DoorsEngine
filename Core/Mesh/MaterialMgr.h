// =================================================================================
// Filename:    MaterialMgr.h
// Description: a manager/storage for all the loaded materials;
//              each unique material has its own ID so we are able
//              to receive material data by its ID;
//
// Created:     25.03.2025 by DimaSkup
// =================================================================================
#pragma once

#include <CoreCommon/Types.h>
#include <CoreCommon/cvector.h>
#include "Material.h"

namespace Core
{

class MaterialMgr
{
public:
    MaterialMgr();

    inline static MaterialMgr* Get() { return pInstance_; }

    // adders/setters
    MaterialID AddMaterial(Material&& material);

    // getters
    Material& GetMaterialByID(const MaterialID id);
    void GetMaterialsByIDs(const MaterialID* ids, const size numMaterials, cvector<Material>& outMaterials) const;
    MaterialID GetMaterialIdByName(const std::string name);

private:
    cvector<MaterialID> ids_;
    cvector<Material>   materials_;

    static MaterialMgr* pInstance_;
    static MaterialID   lastMaterialID_;
};

} // namespace Core
