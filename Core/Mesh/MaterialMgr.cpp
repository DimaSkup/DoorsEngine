// =================================================================================
// Filename:   MaterialMgr.cpp
//
// Created:    25.03.2025 by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include "MaterialMgr.h"


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
    LogDbg(LOG, "creation of the material manager");

    if (pInstance_ == nullptr)
    {
        pInstance_ = this;

        constexpr size newCapacity = 128;
        ids_.reserve(newCapacity);
        materials_.reserve(newCapacity);
    }
    else
    {
        LogErr(LOG, "can't create new instance: there is already an instance of the MaterialMgr");
        return;
    }
}

//---------------------------------------------------------
// Desc:   add a new EMPTY material
// Args:   - matName:  each material must have its unique name
// Ret:    a ref to the added material
//---------------------------------------------------------
Material& MaterialMgr::AddMaterial(const char* matName)
{
    if (!matName || matName[0] == '\0')
    {
        LogErr(LOG, "can't add a new empty material: input name is empty");
        return materials_[INVALID_MATERIAL_ID];
    }

    // generate id
    const MaterialID id = lastMaterialID_;
    ++lastMaterialID_;

    ids_.push_back(id);
    materials_.push_back(Material(matName));

    Material& mat = materials_.back();
    mat.id = id;

    return mat;
}

//---------------------------------------------------------
// Desc:   add a new material into the manager, generate an ID
// Args:   - material:   setup material
// Ret:    id of added material
//---------------------------------------------------------
MaterialID MaterialMgr::AddMaterial(const Material& material)
{
    const MaterialID id = lastMaterialID_;
    ++lastMaterialID_;

    ids_.push_back(id);
    materials_.push_back(material);

    return id;
}

//---------------------------------------------------------
// Desc:   add a new material into the manager, generate an ID
// Args:   - material:   setup material
// Ret:    id of added material
//---------------------------------------------------------
MaterialID MaterialMgr::AddMaterial(Material&& material)
{
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
        LogErr(LOG, "there is no material by ID: %ld", id);
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

Material& MaterialMgr::GetMaterialById(const MaterialID id)
{
    // check if such model exist if so we get its index,
    // or in another case we return material by idx == 0

    const index idx = ids_.get_idx(id);
    const bool exist = (ids_[idx] == id);   // check if we got a valid index

    return materials_[idx * exist];
}

//---------------------------------------------------------
// Desc:   find a material by input name
// Ret:    a ref to material
//---------------------------------------------------------
Material& MaterialMgr::GetMaterialByName(const char* matName)
{
    // check input args
    if (!matName || matName[0] == '\0')
    {
        LogErr(LOG, "can't get material by name: input name is empty");
        return materials_[INVALID_MATERIAL_ID];
    }

    // find material
    for (Material& mat : materials_)
    {
        if (strcmp(mat.name, matName) == 0)
            return mat;
    }

    // if we didn't find any material by input name
    return materials_[INVALID_MATERIAL_ID];
}

///////////////////////////////////////////////////////////

void MaterialMgr::GetMaterialsByIds(
    const MaterialID* ids,
    const size numMaterials,
    cvector<Material>& outMaterials)
{
    try
    {
        CAssert::True(ids != nullptr,   "can't get materials: input ptr to materials IDs arr == nullptr");
        CAssert::True(numMaterials > 0, "can't get materials: input number of materials must be > 0");

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
        outMaterials.resize(numMaterials, Material("invalid"));

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
