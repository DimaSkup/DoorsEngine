// =================================================================================
// Filename:   MaterialMgr.cpp
//
// Created:    25.03.2025 by DimaSkup
// =================================================================================
#include "MaterialMgr.h"
#include <CoreCommon/log.h>
#include <CoreCommon/Assert.h>


namespace Core
{

// init a global instance of the material manager
MaterialMgr g_MaterialMgr;

// setup some static fields
MaterialMgr* MaterialMgr::pInstance_      = nullptr;
MaterialID   MaterialMgr::lastMaterialID_ = 0;


///////////////////////////////////////////////////////////

MaterialMgr::MaterialMgr()
{
    LogDbg("creation of the material manager");

    if (pInstance_ == nullptr)
    {
        pInstance_ = this;

        constexpr size newCapacity = 128;
        ids_.reserve(newCapacity);
        materials_.reserve(newCapacity);
    }
    else
    {
        LogErr("can't create new instance: there is already an instance of the MaterialMgr");
        return;
    }
}

///////////////////////////////////////////////////////////

MaterialID MaterialMgr::AddMaterial(Material&& material)
{
    // add a new material into the manager, generate an ID for it and return this ID

    const MaterialID id = lastMaterialID_;
    ++lastMaterialID_;

    ids_.push_back(id);
    materials_.push_back(std::move(material));

    return id;
}

///////////////////////////////////////////////////////////

bool MaterialMgr::SetMaterialColorData(
    const MaterialID id,
    const Float4& ambient,
    const Float4& diffuse,
    const Float4& specular,
    const Float4& reflect)
{
    // setup color properties of the material by ID

    const index idx = ids_.get_idx(id);
    const bool exist = (ids_[idx] == id);   // check if we got a valid index

    if (!exist)
    {
        sprintf(g_String, "there is no material by ID: %ld", id);
        LogErr(g_String);
        return false;
    }

    Material& mat = materials_[idx];
    mat.ambient   = ambient;
    mat.diffuse   = diffuse;
    mat.specular  = specular;
    mat.reflect   = reflect;

    return true;
}

///////////////////////////////////////////////////////////

Material& MaterialMgr::GetMaterialByID(const MaterialID id)
{
    // check if such model exist if so we get its index,
    // or in another case we return material by idx == 0

    const index idx = ids_.get_idx(id);
    const bool exist = (ids_[idx] == id);   // check if we got a valid index

    return materials_[idx * exist];
}

///////////////////////////////////////////////////////////

void MaterialMgr::GetMaterialsByIDs(
    const MaterialID* ids,
    const size numMaterials,
    cvector<Material>& outMaterials)
{
    try
    {
        Assert::True(ids != nullptr,   "can't get materials: input ptr to materials IDs arr == nullptr");
        Assert::True(numMaterials > 0, "can't get materials: input number of materials must be > 0");

        // get idxs to materials data by its ids
        ids_.get_idxs(ids, numMaterials, idxs_);

        // get materials by idxs
        outMaterials.resize(numMaterials);

        for (index i = 0; i < numMaterials; ++i)
            outMaterials[i] = materials_[idxs_[i]];

    }
    catch (EngineException& e)
    {
        // in any case if we have some input number of materials we fill the output arr with "invalid" materials
        outMaterials.resize(numMaterials, Material());

        LogErr(e);
    }
}

///////////////////////////////////////////////////////////

MaterialID MaterialMgr::GetMaterialIdByName(const char* name) const
{
    // TODO: optimize me, shithead!

    if (!name || name[0] == '\0')
    {
        LogErr("input name is empty");
        return INVALID_MATERIAL_ID;
    }

    for (int i = 0; const Material & mat : materials_)
    {
        if (strcmp(mat.name, name) == 0)
            return ids_[i];

        ++i;
    }

    // if we didn't found any material by name, we just return an invalid ID (0)
    return INVALID_MATERIAL_ID;
}

///////////////////////////////////////////////////////////

MaterialID MaterialMgr::GetMaterialIdByIdx(const index idx) const
{
    // if we have valid idx we return an ID by it or return ID by idx 0 (INVALID_MATERIAL_ID == 0)
    const bool isValid = ((idx > 0) && (idx < ids_.size()));
    return ids_[idx * isValid];
}

} // namespace Core
