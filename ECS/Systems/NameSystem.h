// **********************************************************************************
// Filename:      NameSystem.h
// Description:   Entity-Component-System (ECS) system for control entities names;
// 
// Created:       12.06.24
// **********************************************************************************
#pragma once

#include <Types.h>
#include "../Components/Name.h"

namespace ECS
{

class NameSystem
{
public:
    NameSystem(Name* pNameComponent);
    ~NameSystem() {}


    bool AddRecord(const EntityID id, const char* name);

    bool AddRecords(
        const EntityID* ids,
        const std::string* names,
        const size numEntts);

    EntityID    GetIdByName(const char* name) const;
    const char* GetNameById(const EntityID id) const;

    void GetIdsByNames(const char** names, const size numNames, cvector<EntityID>& outIds) const;
    void GetIdsByNames(const char** names, const size numNames, EntityID* outIdsArr)       const;

    bool IsUnique(const char* name) const;

private:
    inline bool IsIdxValid(const index idx) const
    {
        return (idx >= 0 && idx < pNameComponent_->ids_.size());
    }

private:
    Name* pNameComponent_ = nullptr;
};

}
