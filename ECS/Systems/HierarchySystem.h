// =================================================================================
// Filename:     HierarchySystem.h
// Description:  ECS system to handle entities hierarchy data:
//               entity can have only one "parent" and have multiple "children"
//
// Created:      24.04.2025 by DimaSkup
// =================================================================================
#pragma once

#include "../Components/Hierarchy.h"


namespace ECS
{

class HierarchySystem
{
public:
    HierarchySystem(Hierarchy* pHierarchyComponent);

    bool AddChild(const EntityID id, const EntityID childID);
    void SetParent(const EntityID childID, const EntityID parentID);

    ///////////////////////////////////////////////////////

    inline bool HasChildren(const EntityID id) const
    {
        // return a flag to define if enitity has any children
        return !GetChildren(id).empty();
    }

    ///////////////////////////////////////////////////////

    inline void GetChildrenArr(const EntityID id, cvector<EntityID>& outChildren) const
    {
        // output: an array of children IDs

        const std::set<EntityID>& children = GetChildren(id);
        outChildren.resize(children.size());

        for (int i = 0; const EntityID childID : children)
            outChildren[i++] = childID;
    }

    ///////////////////////////////////////////////////////

    inline const std::set<EntityID>& GetChildren(const EntityID id) const
    {
        // return all the children of input entity
        Hierarchy& comp  = *pHierarchy_;
        const bool exist = comp.data.contains(id);

        return comp.data[id * exist].children;      // if not exist we return empty set of children (of entity which ID == 0)
    }

private:
    Hierarchy* pHierarchy_ = nullptr;
};

} // namespace ECS
