// =================================================================================
// Filename:     Hierarchy.h
// Description:  ECS component to hold entities hierarchy data:
//               entity can have only one "parent" and have multiple "children"
//
// Created:      24.04.2025 by DimaSkup
// =================================================================================
#pragma once

#include "../Common/Types.h"
#include <map>
#include <set>

namespace ECS
{

struct HierarchyNode
{
    HierarchyNode() {}

    EntityID parentID = 0;
    std::set<EntityID> children{};
};

///////////////////////////////////////////////////////////

// ECS component
struct Hierarchy
{
    Hierarchy()
    {
        data.emplace(0, HierarchyNode());
    }

    std::map<EntityID, HierarchyNode> data;
};

} // namespace ECS
