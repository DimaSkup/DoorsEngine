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

    // get a position relatively to parent
    XMFLOAT3 GetRelativePos(const EntityID childID) const;

    void UpdateRelativePos(const EntityID childID)
    {
        Hierarchy& comp = *pHierarchy_;

        if (comp.data.contains(childID))
        {
            HierarchyNode& node = comp.data.at(childID);

            XMFLOAT3 posParent = pTransformSys_->GetPosition(node.parentID);
            XMFLOAT3 posChild  = pTransformSys_->GetPosition(childID);

            // compute new relative position
            node.relativePos =
            {
                posChild.x - posParent.x,
                posChild.y - posParent.y,
                posChild.z - posParent.z
            };
        }
        else
        {
            sprintf(g_String, "there is no hierarchy data for entt: %d", childID);
            LogErr(g_String);
        }
    }

    ///////////////////////////////////////////////////////

    inline bool HasChildren(const EntityID id) const
    {
        // return a flag to define if enitity has any children
        return !GetChildrenSet(id).empty();
    }

    ///////////////////////////////////////////////////////

    inline void GetChildrenArr(const EntityID id, cvector<EntityID>& outChildren) const
    {
        // output: an array of children IDs

        const std::set<EntityID>& children = GetChildrenSet(id);
        outChildren.resize(children.size());

        for (int i = 0; const EntityID childID : children)
            outChildren[i++] = childID;
    }

    ///////////////////////////////////////////////////////

    inline const std::set<EntityID>& GetChildrenSet(const EntityID id) const
    {
        // return all the children of input entity
        Hierarchy& comp  = *pHierarchy_;
        const bool exist = comp.data.contains(id);

        return comp.data[id * exist].children;      // if not exist we return empty set of children (of entity which ID == 0)
    }

private:
    Hierarchy*       pHierarchy_    = nullptr;
    TransformSystem* pTransformSys_ = nullptr;
};

} // namespace ECS
