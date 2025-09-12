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


    //-----------------------------------------------------
    // Desc:   check if input name is unique
    //-----------------------------------------------------
    inline bool IsUnique(const char* name) const
    {
        if (!name || name[0] == '\0')
        {
            LogErr(LOG, "input name is empty");
            return false;
        }

        // if there is no such a name its idx == -1 (so it is a unique name)
        return pNameComponent_->names_.find(name) == -1;
    }

private:
    Name* pNameComponent_ = nullptr;

};

}
