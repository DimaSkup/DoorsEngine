// **********************************************************************************
// Filename:      MaterialSystem.cpp
// Description:   implementation of the MaterialSystem's functional
// 
// Created:       28.06.24
// **********************************************************************************
#include "MaterialSystem.h"

#include "../Common/Assert.h"
#include "../Common/log.h"

#pragma warning (disable : 4996)


namespace ECS
{

MaterialSystem::MaterialSystem(
    Material* pMaterialComponent,
    NameSystem* pNameSys)
    :
    pMaterialComponent_(pMaterialComponent),
    pNameSystem_(pNameSys)
{
    Assert::True(pMaterialComponent != nullptr, "input ptr to the Material component == nullptr");
    Assert::True(pNameSys != nullptr,           "input ptr to the Name system == nullptr");

    // setup default (invalid) material which has ID == 0 for invalid entity
    const cvector<MaterialID> materialsIDs(1, INVALID_MATERIAL_ID);
    const bool                isMeshBasedMaterials = true;
    MaterialData              matData(materialsIDs.data(), materialsIDs.size());

    pMaterialComponent->enttsIDs.push_back(INVALID_ENTITY_ID);
    pMaterialComponent->data.push_back(std::move(matData));
    pMaterialComponent->flagsMeshBasedMaterials.push_back(isMeshBasedMaterials);
}

///////////////////////////////////////////////////////////

void MaterialSystem::Serialize(std::ofstream& fout, u32& offset)
{
    Assert::True(false, "TODO: implement it!");
}

///////////////////////////////////////////////////////////

void MaterialSystem::Deserialize(std::ifstream& fin, const u32 offset)
{
    Assert::True(false, "TODO: implement it!");
}

///////////////////////////////////////////////////////////

void MaterialSystem::AddRecord(
    const EntityID enttID,
    const MaterialID* materialsIDs,
    const size numSubmeshes,
    const bool areMaterialsMeshBased)
{
    // add own textures set to each input entity;
    // 
    // in: materialsIDs          -- arr of material IDs (each material ID will be related to a single submesh of the entity)
    //     numSubmeshes          -- how many meshes does input entity have
    //     areMaterialsMeshBased -- defines if all the materials IDs are the same as materials IDs of the related model

    Assert::True(enttID != INVALID_ENTITY_ID, "invalid entity");
    Assert::True(materialsIDs != nullptr,     "input ptr to materials IDs arr == nullptr");
    Assert::True(numSubmeshes > 0,            "input number of submeshes must be > 0");


    Material& comp = *pMaterialComponent_;

    // if there is already a record with such entt ID
    if (comp.enttsIDs.binary_search(enttID))
    {
        char buf[64];
        sprintf(buf, "can't add record: there is already an entity by ID: %ud", enttID);
        LogErr(buf);
        return;
    }

    // add a record (here we execute sorted insertion into the data arrays)
    const index idx = comp.enttsIDs.get_insert_idx(enttID);

    comp.enttsIDs.insert_before(idx, enttID);
    comp.data.insert_before(idx, MaterialData(materialsIDs, numSubmeshes));
    comp.flagsMeshBasedMaterials.insert_before(idx, areMaterialsMeshBased);
}

///////////////////////////////////////////////////////////

void MaterialSystem::SetMaterial(
    const EntityID enttID,
    const SubmeshID enttSubmeshID,
    const MaterialID matID)
{
    // set a material (matID) for subset/mesh (enttSubmeshID) of entity (enttID)

    Material& comp = *pMaterialComponent_;
    const index idx      = comp.enttsIDs.get_idx(enttID);
    const bool exist     = (comp.enttsIDs[idx] == enttID);

    if (exist)
    {
        comp.data[idx].materialsIDs[0] = matID;
        comp.flagsMeshBasedMaterials[idx] = false;
    }
    else
    {
        char buf[64];
        sprintf(buf, "there is no entity by ID: %d", enttID);
        LogErr(buf);
    }
}

///////////////////////////////////////////////////////////

const MaterialData& MaterialSystem::GetDataByEnttID(const EntityID id) const
{
    // get data (arr of material IDs) for entity by input ID

    const Material& comp = *pMaterialComponent_;
    const index idx = comp.enttsIDs.get_idx(id);
    const bool exist = (comp.enttsIDs[idx] == id);

    // if there no data by input ID we return "invalid" data (by idx == 0)
    return comp.data[idx * exist];
}

///////////////////////////////////////////////////////////

void MaterialSystem::GetDataByEnttsIDs(
    const EntityID* ids,
    const size numEntts,
    cvector<MaterialData>& outMaterialsDataPerEntt)
{
    // get an array of materials data per each input entity id;
    // (array per entity because each entity can have multiple meshes with one material)

    Assert::True(ids != nullptr, "input ptr to entities IDs arr == nullptr");
    Assert::True(numEntts > 0,   "input number of entities must be > 0");

    const Material& comp = *pMaterialComponent_;
    cvector<index> idxs;

    comp.enttsIDs.get_idxs(ids, numEntts, idxs);

#if DEBUG || _DEBUG
    CheckEnttsHaveMaterialComponent(ids, idxs.data(), numEntts);
#endif

    outMaterialsDataPerEntt.resize(numEntts);

    for (int i = 0; const index idx : idxs)
        outMaterialsDataPerEntt[i++] = comp.data[idx];
}

///////////////////////////////////////////////////////////

void MaterialSystem::GetMaterialsFlagsByEntts(
    const EntityID* ids,
    const size numEntts,
    cvector<bool>& outFlags) const
{
    // get flags which defines if materials of particular entity by ID are based on
    // materials of the related original model; if so we will render such entity
    // using INSTANCING;

    Assert::True(ids != nullptr, "input ptr to entities IDs arr == nullptr");
    Assert::True(numEntts > 0,   "input number of entities must be > 0");

    Material& comp = *pMaterialComponent_;
    cvector<index> idxs;

    comp.enttsIDs.get_idxs(ids, numEntts, idxs);

    // check if we have valid entities IDs
#if DEBUG || _DEBUG
    CheckEnttsHaveMaterialComponent(ids, idxs.data(), numEntts);
#endif

    outFlags.resize(numEntts);

    for (int i = 0; const index idx : idxs)
        outFlags[i++] = comp.flagsMeshBasedMaterials[idx];
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
            LogErr(GenerateErrMsgNotHaveComponent(ids[i]));
        }
    }
}

///////////////////////////////////////////////////////////

const char* MaterialSystem::GenerateErrMsgNotHaveComponent(const EntityID id) const
{
    // a helper to generate a message about the entity doesn't have this component
    const EntityName& name = pNameSystem_->GetNameById(id);
    sprintf(g_String, "entity (ID: %ud; name: %s) doesn't have a material component!", id, name.c_str());

    return g_String;
}

} // namespace ECS
