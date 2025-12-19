// **********************************************************************************
// Filename:      MaterialSystem.cpp
// Description:   implementation of the MaterialSystem's functional
// 
// Created:       28.06.24
// **********************************************************************************
#include "../Common/pch.h"
#include "MaterialSystem.h"

#pragma warning (disable : 4996)


namespace ECS
{

static cvector<index> s_Idxs;

MaterialSystem::MaterialSystem(Material* pMaterialComponent, NameSystem* pNameSys)
    :
    pMaterialComponent_(pMaterialComponent),
    pNameSystem_(pNameSys)
{
    CAssert::True(pMaterialComponent != nullptr, "input ptr to the Material component == nullptr");
    CAssert::True(pNameSys != nullptr,           "input ptr to the Name system == nullptr");

    // setup default (invalid) material which has ID == 0 for invalid entity
    const cvector<MaterialID> materialsIDs(1, INVALID_MATERIAL_ID);
    const bool                isMeshBasedMaterials = true;
    MaterialData              matData(materialsIDs.data(), materialsIDs.size());

    pMaterialComponent->enttsIDs.push_back(INVALID_ENTITY_ID);
    pMaterialComponent->data.push_back(std::move(matData));
}

//---------------------------------------------------------
// Desc:   add own textures set to each input entity
// Args:   - materialsIDs: arr of material IDs (each material ID will be related
//                         to a single submesh of the entity)
//         - numSubmeshes: how many meshes does input entity have
//---------------------------------------------------------
void MaterialSystem::AddRecord(
    const EntityID enttID,
    const MaterialID* materialsIDs,
    const size numSubmeshes)
{

    CAssert::True(enttID != INVALID_ENTITY_ID, "invalid entity");
    CAssert::True(materialsIDs != nullptr,     "input ptr to materials IDs arr == nullptr");
    CAssert::True(numSubmeshes > 0,            "input number of submeshes must be > 0");


    Material& comp = *pMaterialComponent_;

    // if there is already a record with such entt ID
    if (comp.enttsIDs.binary_search(enttID))
    {
        LogErr(LOG, "can't add record: there is already an entity by ID: %ld", enttID);
        return;
    }

    // add a record (here we execute sorted insertion into the data arrays)
    const index idx = comp.enttsIDs.get_insert_idx(enttID);

    comp.enttsIDs.insert_before(idx, enttID);
    comp.data.insert_before(idx, MaterialData(materialsIDs, numSubmeshes));
}

//---------------------------------------------------------
// Desc:   set a material (matID) for subset/mesh (enttSubmeshId) of entity (enttID)
//---------------------------------------------------------
void MaterialSystem::SetMaterial(
    const EntityID enttID,
    const SubmeshID enttSubmeshId,
    const MaterialID matID)
{
    Material& comp = *pMaterialComponent_;
    const index idx = comp.enttsIDs.get_idx(enttID);
    cvector<MaterialID>& matsIds = comp.data[idx].materialsIds;

    // if such entt doesn't have a material component (we have no such record)
    if (comp.enttsIDs[idx] != enttID)
    {
        LogErr(LOG, "there is no entity by ID: %d", (int)enttID);
        return;
    }
 
    // if input submesh id is invalid (is too big)
    if ((vsize)enttSubmeshId >= matsIds.size())
    {
        LogErr(LOG, "input submesh id is invalid: %d", (int)enttSubmeshId);
        return;
    }

    // set new material id for a submesh
    matsIds[enttSubmeshId] = matID;
}

//---------------------------------------------------------
// Desc:   get arr of material Ids for entity by input ID
//---------------------------------------------------------
const MaterialData& MaterialSystem::GetDataByEnttId(const EntityID id) const
{
    const Material& comp = *pMaterialComponent_;
    const index idx      = comp.enttsIDs.get_idx(id);
    const bool exist     = (comp.enttsIDs[idx] == id);

    // if there no data by input ID we return "invalid" data (by idx == 0)
    return comp.data[idx * exist];
}

//---------------------------------------------------------
// Desc:   get an array of materials data per each input entity id;
//         (array per entity because each entity can have multiple meshes with one material)
// Args:   - ids:                 arr of entities identifiers
//         - numEntts:            how many entities in arr
//         - outMatsDataPerEntt:  arr of material data per each entity
//---------------------------------------------------------
void MaterialSystem::GetDataByEnttsIds(
    const EntityID* ids,
    const size numEntts,
    cvector<MaterialData>& outMatsDataPerEntt)
{
    CAssert::True(ids != nullptr, "input ptr to entities IDs arr == nullptr");
    CAssert::True(numEntts > 0,   "input number of entities must be > 0");

    const Material& comp = *pMaterialComponent_;

    comp.enttsIDs.get_idxs(ids, numEntts, s_Idxs);

#if DEBUG || _DEBUG
    CheckEnttsHaveMaterialComponent(ids, s_Idxs.data(), numEntts);
#endif

    outMatsDataPerEntt.resize(numEntts);

    for (int i = 0; const index idx : s_Idxs)
    {
        outMatsDataPerEntt[i++] = comp.data[idx];
    }
}

// =================================================================================
// Private helper methods
// =================================================================================
void MaterialSystem::CheckEnttsHaveMaterialComponent(
    const EntityID* ids,
    const index* idxs,              // expected idxs of entity IDs in the Material component
    const size numEntts) const
{
    // print an error message into console if there is no entity
    // by particular ID in the Material component

    const Material& comp = *pMaterialComponent_;

    for (index i = 0; i < numEntts; ++i)
    {
        // if we have not the same ID
        if (comp.enttsIDs[idxs[i]] != ids[i])
        {
            LogErr(
                LOG,
                "entity (ID: %d; name: %s) doesn't have a material component!",
                (int)ids[i],
                pNameSystem_->GetNameById(ids[i]));
        }
    }
}

} // namespace ECS
