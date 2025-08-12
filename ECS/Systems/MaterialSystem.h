// **********************************************************************************
// Filename:      MaterialSystem.h
// Description:   Entity-Component-System (ECS) system for control 
//                textures data of entities
// 
// Created:       28.06.24
// **********************************************************************************
#pragma once

#include <Types.h>
#include "../Components/Material.h"
#include "../Systems/NameSystem.h"
#include <fstream>

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

private:
    Material*   pMaterialComponent_ = nullptr;
    NameSystem* pNameSystem_        = nullptr;
};

}
