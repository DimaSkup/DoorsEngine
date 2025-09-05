// =================================================================================
// Filename:     HierarchySystem.h
// Description:  ECS system to handle entities hierarchy data:
//               entity can have only one "parent" and have multiple "children"
//
// Created:      24.04.2025 by DimaSkup
// =================================================================================
#pragma once

#include "../Components/Hierarchy.h"
#include "../Systems/TransformSystem.h"

#pragma warning (disable : 4996)

namespace ECS
{

class HierarchySystem
{
public:
    HierarchySystem(Hierarchy* pHierarchyComponent, TransformSystem* pTransformSys);

    bool AddChild(const EntityID id, const EntityID childID);
    void SetParent(const EntityID childID, const EntityID parentID);

    void     UpdateRelativePos(const EntityID childID);
    XMFLOAT3 GetRelativePos(const EntityID childID) const;

    //---------------------------------------------------------
    // Desc:  return a flag to define if enitity has any children
    //---------------------------------------------------------
    inline bool HasChildren(const EntityID id) const
    {
        return !GetChildrenSet(id).empty();
    }

    //---------------------------------------------------------
    // output:  an array of children Ids
    //---------------------------------------------------------
    inline void GetChildrenArr(const EntityID id, cvector<EntityID>& outChildren) const
    {
        const std::set<EntityID>& children = GetChildrenSet(id);
        outChildren.resize(children.size());

        for (int i = 0; const EntityID childID : children)
            outChildren[i++] = childID;
    }

    //---------------------------------------------------------
    // Desc:  return all the children of input entity
    //---------------------------------------------------------
    inline const std::set<EntityID>& GetChildrenSet(const EntityID id) const
    {
        Hierarchy& comp  = *pHierarchy_;
        const bool exist = comp.data.contains(id);

        return comp.data[id * exist].children;      // if not exist we return empty set of children (of entity which ID == 0)
    }

private:
    Hierarchy*       pHierarchy_    = nullptr;
    TransformSystem* pTransformSys_ = nullptr;
};

} // namespace ECS
