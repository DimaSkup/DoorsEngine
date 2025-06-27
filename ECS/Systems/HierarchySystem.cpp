#include "../Common/pch.h"
#include "HierarchySystem.h"

#pragma warning (disable : 4996)

namespace ECS
{

HierarchySystem::HierarchySystem(
    Hierarchy* pHierarchyComponent,
    TransformSystem* pTransformSys)
    :
    pHierarchy_(pHierarchyComponent),
    pTransformSys_(pTransformSys)
{
    if (!pHierarchyComponent)
    {
        LogErr("input ptr to the hierarchy component == nullptr");
        return;
    }

    if (!pTransformSys)
    {
        LogErr("input ptr to the transform system == nullptr");
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

    const XMFLOAT3 parentPos = pTransformSys_->GetPosition(id);
    const XMFLOAT3 childPos = pTransformSys_->GetPosition(childID);

    // compute position of child relatively to its parent (offset from parent to child)
    const XMFLOAT3 relPos =
    {
        childPos.x - parentPos.x,
        childPos.y - parentPos.y,
        childPos.z - parentPos.z,
    };

    // set a new parent and relative position
    comp.data[childID].parentID = id;
    comp.data[childID].relativePos = relPos;

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
}

///////////////////////////////////////////////////////////

XMFLOAT3 HierarchySystem::GetRelativePos(const EntityID childID) const
{
    // get a position relatively to parent
    const Hierarchy& comp = *pHierarchy_;

    if (comp.data.contains(childID))
    {
        return comp.data.at(childID).relativePos;
    }
    else
    {
        sprintf(g_String, "there is no hierarchy data for entity: %d", childID);
        LogErr(g_String);
        return { 0,0,0 };
    }
}

} // namespace ECS
