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
    const char* GetNameById(const EntityID& id) const;

private:
    // check if input name is unique
    inline bool IsUnique(const char* name) const
    {
        return (GetIdByName(name) == INVALID_ENTITY_ID);
    }

private:
    Name* pNameComponent_ = nullptr;

};

}
