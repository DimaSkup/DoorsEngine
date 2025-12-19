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
        LogErr(LOG, "input ptr to the hierarchy component == nullptr");
        return;
    }

    if (!pTransformSys)
    {
        LogErr(LOG, "input ptr to the transform system == nullptr");
        return;
    }

    // create "invalid" data which is used when we query for not existent entity ID
    //pHierarchyComponent->data[0] = HierarchyNode();
}

//---------------------------------------------------------
// Desc:  add a child for the entity by ID;
// Args:  - id:       identifier of entity which will have a new child
//        - childId:  identifier of entity chich will be a child 
// Ret:   true if we managed to did it
//---------------------------------------------------------
bool HierarchySystem::AddChild(const EntityID id, const EntityID childID)
{
    Hierarchy& comp = *pHierarchy_;

    // if child already has some another parent
    if (comp.data[childID].parentID != 0)
    {
        LogErr(
            LOG,
            "can't add child (id: %" PRIu32 " for parent (id: %" PRIu32 ") because the child "
            "is already an element of another hierarchy and has another parent",
            childID, id);
        return false;
    }

    const XMFLOAT3 parentPos = pTransformSys_->GetPosition(id);
    const XMFLOAT3 childPos  = pTransformSys_->GetPosition(childID);

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

//---------------------------------------------------------
// Desc:   update a position of child relatively to its parent
//---------------------------------------------------------
void HierarchySystem::UpdateRelativePos(const EntityID childID)
{
    Hierarchy& comp = *pHierarchy_;

    if (comp.data.contains(childID))
    {
        HierarchyNode& node = comp.data[childID];

        // if we have a parent of this child
        if (node.parentID)
        {
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
    }
}

//---------------------------------------------------------
// Desc:   set a position of child relatively to its parent
// Args:   - childId:    identifier of a child entity to update
//         - newRelPos:  new relative position
//---------------------------------------------------------
void HierarchySystem::SetRelativePos(const EntityID childID, const XMFLOAT3& newRelPos)
{
    Hierarchy& comp = *pHierarchy_;

    if (comp.data.contains(childID))
    {
        HierarchyNode& node = comp.data[childID];

        // if we have a parent of this child
        if (node.parentID)
            node.relativePos = newRelPos;
    }
}

//---------------------------------------------------------

void HierarchySystem::SetRelativePos(const EntityID childId, const XMVECTOR& newRelPos)
{
    Hierarchy& comp = *pHierarchy_;

    if (comp.data.contains(childId))
    {
        HierarchyNode& node = comp.data[childId];

        // if we have a parent of this child
        if (node.parentID)
        {
            XMStoreFloat3(&node.relativePos, newRelPos);
        }
    }
}

//---------------------------------------------------------
// Desc:  set a new parent to child entity;
//        if necessary we detach this child from its previous parent
//---------------------------------------------------------
void HierarchySystem::SetParent(const EntityID childID, const EntityID parentID)
{

    Hierarchy& comp = *pHierarchy_;

    // if there is already a record with this child entity 
    if (comp.data.contains(childID))
    {
        // detach this child from its previous parent
        EntityID prevParentID = comp.data[childID].parentID;
        comp.data[prevParentID].children.erase(childID);
    }
}

//---------------------------------------------------------
// Desc:   get a position relatively to parent
//---------------------------------------------------------
XMFLOAT3 HierarchySystem::GetRelativePos(const EntityID childID) const
{
    const Hierarchy& comp = *pHierarchy_;

    if (comp.data.contains(childID))
    {
        return comp.data.at(childID).relativePos;
    }
    else
    {
        LogErr(LOG, "there is no hierarchy data for entity: %" PRIu32, childID);
        return { 0,0,0 };
    }
}

//---------------------------------------------------------

void HierarchySystem::GetRelativePositions(
    const EntityID* ids,
    const size numEntts,
    XMFLOAT3* outPositions) const
{
    if (!ids || !outPositions)
    {
        LogErr(LOG, "some of input ptrs == nullptr");
        return;
    }


    const Hierarchy& comp = *pHierarchy_;

    for (index i = 0; i < numEntts; ++i)
    {
        const EntityID childId = ids[i];

        if (comp.data.contains(childId))
            outPositions[i] = comp.data.at(childId).relativePos;

        else
        {
            LogErr(LOG, "there is no hierarchy data for entity: %" PRIu32, childId);
            outPositions[i] = { 0,0,0 };
        }
    }
}

} // namespace ECS
