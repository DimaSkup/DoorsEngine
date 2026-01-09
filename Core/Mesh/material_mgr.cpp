// =================================================================================
// Filename:   MaterialMgr.cpp
//
// Created:    25.03.2025 by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include "material_mgr.h"


namespace Core
{

// init a global instance of the material manager
MaterialMgr g_MaterialMgr;

// setup some static fields
MaterialMgr* MaterialMgr::pInstance_      = nullptr;

// since we already have an "invalid" material by ID == 0 we start from 1
MaterialID   MaterialMgr::lastMaterialID_ = 1;

#define DEFAULT_SHADER_ID 3



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
// Desc:   just destructor
//---------------------------------------------------------
MaterialMgr::~MaterialMgr()
{
    LogMsg(LOG, "is destroyed");
}

//---------------------------------------------------------
// Desc:  add and setup a material which serve us as default/invalid
//
//        for instance: we try to get a material by wrong ID/name
//                      so in this case we return the default/invalid material
//---------------------------------------------------------
bool MaterialMgr::Init()
{
    ids_.push_back(INVALID_MATERIAL_ID);
    materials_.push_back(Material("invalid"));

    // currently there must be only one material
    if ((ids_.size() != 1) || (materials_.size() != 1))
    {
        LogErr(LOG, "something went wrong: there is more than one material");
        return false;
    }

    Material& invalidMat = materials_.back();
    invalidMat.shaderId = INVALID_SHADER_ID;
    invalidMat.renderStates = MAT_PROP_DEFAULT;
    invalidMat.SetTexture(TEX_TYPE_DIFFUSE, INVALID_TEX_ID);

    return true;
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

    // generate a new id
    const MaterialID id = lastMaterialID_;
    ++lastMaterialID_;

    ids_.push_back(id);
    materials_.push_back(Material(matName));

    Material& newMat = materials_.back();
    newMat.id = id;
    newMat.shaderId = DEFAULT_SHADER_ID;

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

    if (mat.shaderId == INVALID_SHADER_ID)
        mat.shaderId = DEFAULT_SHADER_ID;

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

    if (mat.shaderId == INVALID_SHADER_ID)
        mat.shaderId = DEFAULT_SHADER_ID;

    return id;
}

//---------------------------------------------------------
// Desc:  bind a texture to material at a particular texture slot by texType
//---------------------------------------------------------
bool MaterialMgr::SetMatTexture(
    const MaterialID matId,
    const TexID texId,
    const uint texType)
{
    Material& mat = GetMatById(matId);

    if (mat.id == INVALID_MATERIAL_ID)
        return false;

    //if (texId == INVALID_TEX_ID)
    //    return false;

    if (texType >= NUM_TEXTURE_TYPES)
        return false;

    mat.SetTexture((eTexType)texType, texId);
    return true;
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

    // check that idx is valid
    if (idx < 0 && idx >= ids_.size())
        return materials_[0];

    return materials_[idx];
}

//---------------------------------------------------------
// Desc:   find a material by input name
// Ret:    a ref to material
//---------------------------------------------------------
Material& MaterialMgr::GetMatByName(const char* name)
{
    if (StrHelper::IsEmpty(name))
    {
        LogErr(LOG, "input name is empty");
        return materials_[INVALID_MATERIAL_ID];
    }

    // find a material by name
    for (Material& mat : materials_)
    {
        if (strcmp(mat.name, name) == 0)
            return mat;
    }

    LogErr(LOG, "there is no material by name: %s", name);
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
    if (numMats <= 0)
    {
        LogErr(LOG, "input number of materials must be > 0");
        return;
    }
    if (!ids)
    {
        // out: arr of empty materials
        outMats.resize(numMats, Material());
        LogErr(LOG, "input ptr to materials IDs arr == nullptr");
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
    if (StrHelper::IsEmpty(name))
    {
        LogErr(LOG, "input name is empty");
        return INVALID_MATERIAL_ID;
    }

    for (index i = 0; i < materials_.size(); ++i)
    {
        if (strcmp(materials_[i].name, name) == 0)
            return ids_[i];
    }

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


} // namespace Core
