#include "../Common/pch.h"
#include "HierarchySystem.h"

#pragma warning (disable : 4996)

namespace ECS
{

HierarchySystem::HierarchySystem(Hierarchy* pHierarchyComponent) :
    pHierarchy_(pHierarchyComponent)
{
    if (!pHierarchyComponent)
    {
        LogErr("input ptr to the hierarchy component == nullptr");
        return;
    }

    // create "invalid" data which is used when we query for not existent entity ID
    //pHierarchyComponent->data[0] = HierarchyNode();
}

///////////////////////////////////////////////////////

bool HierarchySystem::AddChild(const EntityID id, const EntityID childID)
{
    // add a child for the entity by ID;
    // return true if we managed to did it

    Hierarchy& comp = *pHierarchy_;

    // if child already has some another parent
    if (comp.data[childID].parentID != 0)
    {
        sprintf(
            g_String,
            "can't add child (id: %ld) for parent (id: %ld) because the child "
            "is already an element of another hierarchy and has another parent",
            childID, id);

        LogErr(g_String);
        return false;
    }

    comp.data[childID].parentID = id;

    // add a child entity for the parent entity by ID
    comp.data[id].children.insert(childID);

    return true;
}

///////////////////////////////////////////////////////

void HierarchySystem::SetParent(const EntityID childID, const EntityID parentID)
{
    // set a new parent to child entity;
    // if necessary we detach this child from its previous parent

    Hierarchy& comp = *pHierarchy_;

    // if there is already a record with this child entity 
    if (comp.data.contains(childID))
    {
        // detach this child from its previous parent
        EntityID prevParentID = comp.data[childID].parentID;
        comp.data[prevParentID].children.erase(childID);
    }

    // set a new parent
    comp.data[childID].parentID = parentID;
}

} // namespace ECS
