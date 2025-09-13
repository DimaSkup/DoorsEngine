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
    Material&  AddMaterial(const char* materialName);
    MaterialID AddMaterial(const Material& material);
    MaterialID AddMaterial(Material&& material);

    bool SetMatColorData(
        const MaterialID id,
        const Vec4& ambient,
        const Vec4& diffuse,
        const Vec4& specular,
        const Vec4& reflect);

    // getters
    Material&  GetMatById    (const MaterialID id);
    Material&  GetMatByName  (const char* name);
    void       GetMaterialsByIds  (const MaterialID* ids, const size numMaterials, cvector<Material>& outMaterials);

    MaterialID GetMatIdByName(const char* name) const;
    MaterialID GetMatIdByIdx (const index idx) const;

    inline size GetNumAllMaterials() const { return materials_.size(); }

    void GetFillModesNames         (cvector<std::string>& outNames) const;
    void GetCullModesNames         (cvector<std::string>& outNames) const;
    void GetBlendingStatesNames    (cvector<std::string>& outNames) const;
    void GetDepthStencilStatesNames(cvector<std::string>& outNames) const;

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
