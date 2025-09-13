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

// since we already have an "invalid" material by ID == 0 we start from 1
MaterialID   MaterialMgr::lastMaterialID_ = 1;         


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

        // create and setup an "invalid" material
        ids_.push_back(INVALID_MATERIAL_ID);
        materials_.push_back(Material("invalid"));

        Material& invalidMat = materials_.back();
        invalidMat.SetTexture(TEX_TYPE_DIFFUSE, INVALID_TEXTURE_ID);
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

    //Material& mat = GetMatByName(matName);

    // if input name isn't unique just return already existed material by this name
    //if (mat.id != INVALID_MATERIAL_ID)
    //{
    //    return mat;
    //}

    // generate a new id
    const MaterialID id = lastMaterialID_;
    ++lastMaterialID_;

    ids_.push_back(id);
    materials_.push_back(Material(matName));

    Material& newMat = materials_.back();
    newMat.id = id;

    return newMat;
}

//---------------------------------------------------------
// Desc:   add a new material into the manager
// Args:   - material:   material identifier
// Ret:    id of added material
//---------------------------------------------------------
MaterialID MaterialMgr::AddMaterial(const Material& material)
{
    // check if input material has unique name
    if (GetMatIdByName(material.name) != INVALID_MATERIAL_ID)
    {
        LogErr(LOG, "can't add a new empty material: input name must be unique: %s", material.name);
        return INVALID_MATERIAL_ID;
    }

    // generate id
    const MaterialID id = lastMaterialID_;
    ++lastMaterialID_;

    ids_.push_back(id);
    materials_.push_back(material);

    Material& mat = materials_.back();
    mat.id = id;

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

    Material& mat = materials_.back();
    mat.id = id;

    return id;
}

//---------------------------------------------------------
// Desc:   setup color properties for material by ID
//---------------------------------------------------------
bool MaterialMgr::SetMatColorData(
    const MaterialID id,
    const Vec4& ambient,
    const Vec4& diffuse,
    const Vec4& specular,
    const Vec4& reflect)
{
    Material& mat = GetMatById(id);

    if (mat.id == INVALID_MATERIAL_ID)
        return false;

    mat.ambient  = ambient;
    mat.diffuse  = diffuse;
    mat.specular = specular;
    mat.reflect  = reflect;

    return true;
}

//---------------------------------------------------------
// Desc:   get a material by input id
// Ret:    a ref to material
//---------------------------------------------------------
Material& MaterialMgr::GetMatById(const MaterialID id)
{
    const index idx = ids_.get_idx(id);

    // if such material exist we return it by idx or
    // return an invalid material by idx == 0
    return materials_[idx * (ids_[idx] == id)];
}

//---------------------------------------------------------
// Desc:   find a material by input name
// Ret:    a ref to material
//---------------------------------------------------------
Material& MaterialMgr::GetMatByName(const char* matName)
{
    // check input args
    if (!matName || matName[0] == '\0')
    {
        LogErr(LOG, "can't get material by name: input name is empty");
        return materials_[INVALID_MATERIAL_ID];
    }

    // find a material by name
    for (Material& mat : materials_)
    {
        if (strcmp(mat.name, matName) == 0)
            return mat;
    }

    // if we didn't find any material by input name
    LogErr(LOG, "there is no material by name: %s", matName);
    return materials_[INVALID_MATERIAL_ID];
}

//---------------------------------------------------------
// Desc:   get an array of material by input identifiers
// Args:   - ids:      arr of identifiers
//         - numMats:  how many materials we want to get
//         - outMats:  output array of materials
//---------------------------------------------------------
void MaterialMgr::GetMaterialsByIds(
    const MaterialID* ids,
    const size numMats,
    cvector<Material>& outMats)
{
    // check input args
    if (numMats <= 0)
    {
        LogErr(LOG, "can't get materials: input number of materials must be > 0");
        return;
    }

    if (!ids)
    {
        LogErr(LOG, "can't get materials: input ptr to materials IDs arr == nullptr");

        // since we have some input number of materials
        // we just return arr of "invalid" materials
        outMats.resize(numMats, Material());
        return;
    }


    // get idxs to materials data by its ids
    ids_.get_idxs(ids, numMats, idxs_);

    // get materials by idxs
    outMats.resize(numMats);

    for (index i = 0; i < numMats; ++i)
        outMats[i] = materials_[idxs_[i]];
}

//---------------------------------------------------------
// Desc:   find a material identifier by input name
// Ret:    a material ID or 0 if there is no such material
//---------------------------------------------------------
MaterialID MaterialMgr::GetMatIdByName(const char* name) const
{
    // TODO: optimize me, shithead!
    if (!name || name[0] == '\0')
    {
        LogErr("input name is empty");
        return INVALID_MATERIAL_ID;
    }

    for (int i = 0; const Material& mat : materials_)
    {
        if (strcmp(mat.name, name) == 0)
            return ids_[i];

        ++i;
    }

    // if we didn't found any material by name, we just return an invalid ID (0)
    return INVALID_MATERIAL_ID;
}

//---------------------------------------------------------
// Desc:   find a material identifier by input index
// Ret:    a material identifier (is 0 if input idx is invalid)
//---------------------------------------------------------
MaterialID MaterialMgr::GetMatIdByIdx(const index idx) const
{
    const bool isValid = ((idx > 0) && (idx < ids_.size()));
    return ids_[idx * isValid];
}

//---------------------------------------------------------
// Desc:   get array of fill modes names
//---------------------------------------------------------
void MaterialMgr::GetFillModesNames(cvector<std::string>& outNames) const
{
    // for details look at enum eMaterialProp in Material.h
    outNames = { "Solid", "Wireframe" };
}

//---------------------------------------------------------
// Desc:   get array of fill modes names
//---------------------------------------------------------
void MaterialMgr::GetCullModesNames(cvector<std::string>& outNames) const
{
    // for details look at enum eMaterialProp in Material.h
    outNames = { "Back", "Front", "None" };
}

//---------------------------------------------------------
// Desc:   get array of blending states names
// Args:   - bsNames:  output arr of names
//---------------------------------------------------------
void MaterialMgr::GetBlendingStatesNames(cvector<std::string>& outNames) const
{
    // for details look at enum eMaterialProp in Material.h
    outNames =
    {
        "No render target writes",
        "Blend disable",
        "Blend enable",
        "Add",
        "Subtract",
        "Multiply",
        "Transparent",
        "Alpha to coverage",
    };
}

//---------------------------------------------------------
// Desc:   get array of depth-stencil states names
//---------------------------------------------------------
void MaterialMgr::GetDepthStencilStatesNames(cvector<std::string>& outNames) const
{
    // for details look at enum eMaterialProp in Material.h
    outNames =
    {
        "Depth enabled",
        "Depth disabled",
        "Mark mirror",
        "Draw reflection",
        "No double blend",
        "Sky dome",
    };
}


} // namespace Core
