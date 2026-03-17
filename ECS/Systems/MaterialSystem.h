// **********************************************************************************
// Filename:      MaterialSystem.h
// Description:   Entity-Component-System (ECS) system for control 
//                textures data of entities
// 
// Created:       28.06.24
// **********************************************************************************
#pragma once

#include <log.h>
#include "../Components/Material.h"
#include "../Systems/NameSystem.h"

namespace ECS
{

class MaterialSystem
{
public:
    MaterialSystem(Material* pMaterialComponent, NameSystem* pNameSys);
    ~MaterialSystem() {};

    void AddRecord(
        const EntityID enttID,
        const MaterialID* materialsIDs,
        const size numSubmeshes);

    void SetMaterial(
        const EntityID enttID,
        const SubmeshID enttSubmeshID,
        const MaterialID matID);


    const MaterialData& GetDataByEnttId(const EntityID enttID) const;

    void GetDataByEnttsIds(
        const EntityID* ids,
        const size numEntts,
        cvector<MaterialData>& outMaterialsDataPerEntt);

private:
    void CheckEnttsHaveMaterialComponent(
        const EntityID* ids,
        const index* idxs,
        const size numEntts) const;

    index GetIdx(const EntityID id) const;

private:
    Material*   pMaterialComponent_ = nullptr;
    NameSystem* pNameSystem_        = nullptr;
};

//---------------------------------------------------------
// Desc:  get VALID index to data by entity id
//        if there is no data we return 0
//---------------------------------------------------------
inline index MaterialSystem::GetIdx(const EntityID id) const
{
    const Material& comp = *pMaterialComponent_;
    const index idx = comp.enttsIds.get_idx(id);

    if (comp.enttsIds.is_valid_index(idx))
        return idx;

    LogErr(LOG, "there is no entity by ID: %d", (int)id);
    return 0;
}

//---------------------------------------------------------
// Desc:   get arr of material Ids for entity by input ID
//---------------------------------------------------------
inline const MaterialData& MaterialSystem::GetDataByEnttId(const EntityID id) const
{
    return pMaterialComponent_->data[GetIdx(id)];
}


}
