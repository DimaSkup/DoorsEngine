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

// static array for internal purposes
static cvector<index> s_Idxs;


//---------------------------------------------------------
// constructor
//---------------------------------------------------------
MaterialSystem::MaterialSystem(Material* pMaterialComponent, NameSystem* pNameSys)
    :
    pMaterialComponent_(pMaterialComponent),
    pNameSystem_(pNameSys)
{
    assert(pMaterialComponent);
    assert(pNameSys);

    // setup default (invalid) material which has ID == 0 for invalid entity
    const cvector<MaterialID> materialsIDs(1, INVALID_MAT_ID);
    const bool                isMeshBasedMaterials = true;
    MaterialData              matData(materialsIDs.data(), materialsIDs.size());

    pMaterialComponent->enttsIds.push_back(INVALID_ENTT_ID);
    pMaterialComponent->data.push_back(std::move(matData));
}

//---------------------------------------------------------
// Desc:   add own materials set to each input entity
// Args:   - materialsIDs: arr of material IDs (each material ID will be related
//                         to a single submesh of the entity)
//         - numSubmeshes: how many meshes does input entity have
//---------------------------------------------------------
void MaterialSystem::AddRecord(
    const EntityID enttId,
    const MaterialID* materialsIds,
    const size numSubmeshes)
{
    assert(enttId != INVALID_ENTT_ID);
    assert(materialsIds);
    assert(numSubmeshes);

    Material& comp = *pMaterialComponent_;

    if (comp.enttsIds.binary_search(enttId))
    {
        LogErr(LOG, "there is already an entity by ID: %" PRIu32, enttId);
        return;
    }

    // add a record (here we execute sorted insertion into the data arrays)
    const index idx = comp.enttsIds.get_insert_idx(enttId);

    comp.enttsIds.insert_before(idx, enttId);
    comp.data.insert_before(idx, MaterialData(materialsIds, numSubmeshes));
}

//---------------------------------------------------------
// Desc:   set a material (matID) for subset/mesh (enttSubmeshId) of entity (enttID)
//---------------------------------------------------------
void MaterialSystem::SetMaterial(
    const EntityID enttId,
    const SubmeshID enttSubmeshId,
    const MaterialID matId)
{
    Material& comp = *pMaterialComponent_;
    const cvector<EntityID>& ids = comp.enttsIds;

    const index idx = GetIdx(enttId);
    if (idx == 0)
        return;

    // if input submesh id is invalid (is too big)
    if ((vsize)enttSubmeshId >= comp.data[idx].materialsIds.size())
    {
        LogErr(LOG, "input submesh id is invalid: %d", (int)enttSubmeshId);
        return;
    }

    // set new material id for a submesh
    comp.data[idx].materialsIds[enttSubmeshId] = matId;
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

    comp.enttsIds.get_idxs(ids, numEntts, s_Idxs);

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
        if (comp.enttsIds[idxs[i]] != ids[i])
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
