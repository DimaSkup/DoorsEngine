// **********************************************************************************
// Filename:      MaterialSystem.h
// Description:   Entity-Component-System (ECS) system for control 
//                textures data of entities
// 
// Created:       28.06.24
// **********************************************************************************
#pragma once

#include "../Common/Types.h"
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

    void Serialize(std::ofstream& fout, u32& offset);
    void Deserialize(std::ifstream& fin, const u32 offset);

    void AddRecord(
        const EntityID enttID,
        const MaterialID* materialsIDs,
        const size numSubmeshes,
        const bool areMaterialsMeshBased);

    void SetMaterial(
        const EntityID enttID,
        const SubmeshID enttSubmeshID,
        const MaterialID matID);


    const MaterialData& GetDataByEnttID(const EntityID enttID) const;

    void GetDataByEnttsIDs(
        const EntityID* ids,
        const size numEntts,
        cvector<MaterialData>& outMaterialsDataPerEntt);

    void GetMaterialsFlagsByEntts(
        const EntityID* ids,
        const size numEntts,
        cvector<bool>& outFlags) const;

private:
    void CheckEnttsHaveMaterialComponent(
        const EntityID* ids,
        const index* idxs,
        const size numEntts) const;

    const char* GenerateErrMsgNotHaveComponent(const EntityID id) const;

private:
    Material*   pMaterialComponent_ = nullptr;
    NameSystem* pNameSystem_        = nullptr;
};

}
