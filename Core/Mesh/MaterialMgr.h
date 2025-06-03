// =================================================================================
// Filename:    MaterialMgr.h
// Description: a manager/storage for all the loaded materials;
//              each unique material has its own ID so we are able
//              to receive material data by its ID;
//
// Created:     25.03.2025 by DimaSkup
// =================================================================================
#pragma once

#include <Types.h>
#include <cvector.h>
#include "Material.h"

namespace Core
{

class MaterialMgr
{
public:
    MaterialMgr();

    // adders/setters
    MaterialID AddMaterial(Material&& material);

    bool SetMaterialColorData(
        const MaterialID id,
        const Float4& ambient,
        const Float4& diffuse,
        const Float4& specular,
        const Float4& reflect);

    // getters
    Material&  GetMaterialByID    (const MaterialID id);
    void       GetMaterialsByIDs  (const MaterialID* ids, const size numMaterials, cvector<Material>& outMaterials);
    MaterialID GetMaterialIdByName(const char* name) const;
    MaterialID GetMaterialIdByIdx (const index idx) const;

    inline size GetNumAllMaterials() const { return materials_.size(); }


private:
    cvector<MaterialID> ids_;
    cvector<Material>   materials_;

    cvector<index>      idxs_;

    static MaterialMgr* pInstance_;
    static MaterialID   lastMaterialID_;
};


// =================================================================================
// Declare a global instance of the material manager
// =================================================================================
extern MaterialMgr g_MaterialMgr;

} // namespace Core
