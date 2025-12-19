/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: quad_tree_node.cpp
    Desc:     implementation of quad tree nodes functional

              This code I mainly rewrote from sources to the amazing book
              "Real Time 3D Terrain Engines Using C++ and DirectX"
              by Greg Snook

    Created:  15.09.2025 by DimaSkup
\**********************************************************************************/
#include <CoreCommon/pch.h>
#include "quad_tree_node.h"
#include "scene_object.h"
#include <bit_flags_functions.h>
#include <geometry/intersection_tests.h>

namespace Core
{

//---------------------------------------------------------
// Desc:   setup a parent and children for this quad tree node
//---------------------------------------------------------
void QuadTreeNode::Setup(
    QuadTreeNode* pParent,
    QuadTreeNode* pChild0,
    QuadTreeNode* pChild1,
    QuadTreeNode* pChild2,
    QuadTreeNode* pChild3)
{
    //assert(pParent                                  && "input ptr to parent node == nullptr");
    //assert(pChild0 && pChild1 && pChild2 && pChild3 && "some of input ptrs to child node == nullptr");

    if (pParentNode_ != 0)
        LogDbg(LOG, "Parent node is already set");

    pParentNode_ = pParent;
    pChildNodes_[0] = pChild0;
    pChildNodes_[1] = pChild1;
    pChildNodes_[2] = pChild2;
    pChildNodes_[3] = pChild3;
}

//---------------------------------------------------------
// Desc: 
//---------------------------------------------------------
uint32 QuadTreeNode::AddOrUpdateMember(SceneObject* pMember, const QuadTreeRect& rect)
{
    u32Flags yMask = YMASK(rect.y0, rect.y1);

    QuadTreeNode* pNode = pMember->GetQuadTreeNode();

    // is this node not already a member?
    if (pNode != this)
    {
        // remove the member from it's previous quad tree node (if any)
        if (pNode)
            pNode->RemoveMember(pMember);

        // account for the new addition
        if (pFirstMember_ == nullptr)
        {
            pFirstMember_ = pMember;
        }
        else
        {
            // prepend this member to our list
            pMember->SetPrevTreeLink(nullptr);
            pMember->SetNextTreeLink(pFirstMember_);
            pFirstMember_->SetPrevTreeLink(pMember);
            pFirstMember_ = pMember;
        }

        // update our yMask
        yMask_.SetFlags(yMask);
        yLocalMask_.SetFlags(yMask);

        // notify our parent of the addition
        if (pParentNode_)
            pParentNode_->DescendantMemberAdded(yMask_);
    }
    else
    {
        // refresh our yMask for all members
        RebuildLocalYMask();
    }

    pMember->SetQuadTreeData(this, yMask);

    // update the member's yMask
    return yMask;
}

//---------------------------------------------------------

void QuadTreeNode::RemoveMember(SceneObject* pMember)
{
    if (!pMember)
    {
        LogErr(LOG, "input ptr to scene object member == nullptr");
        return;
    }

    // make sure this is one of ours
    if (pMember->GetQuadTreeNode() != this)
    {
        LogErr(LOG, "error removing quad tree member");
        return;
    }

    // remove this member from it's chain
    if (pMember->GetNextTreeLink())
    {
        pMember->GetNextTreeLink()->SetPrevTreeLink(pMember->GetPrevTreeLink());
    }
    if (pMember->GetPrevTreeLink())
    {
        pMember->GetPrevTreeLink()->SetNextTreeLink(pMember->GetNextTreeLink());
    }

    // if this was our first member, advance our ptr to the next member
    if (pFirstMember_ == pMember)
        pFirstMember_ = pMember->GetNextTreeLink();

    // clear the former members links
    pMember->SetPrevTreeLink(nullptr);
    pMember->SetNextTreeLink(nullptr);

    // rebuild our y mask
    RebuildLocalYMask();

    // notify our parent
    if (pParentNode_)
        pParentNode_->DescendantMemberRemoved();
}

//---------------------------------------------------------
// Desc:   completely rebuild Y mask (bitfield) for this quad tree node
//---------------------------------------------------------
void QuadTreeNode::RebuildLocalYMask()
{
    yLocalMask_.Clear();

    // go through each member and combine its Y-mask with the tree node's mask
    SceneObject* pObj = pFirstMember_;
    while (pObj)
    {
        yLocalMask_.SetFlags(pObj->GetYMask());
        pObj = pObj->GetNextTreeLink();
    }

    // the combines yMask must now be updated
    RebuildYMask();
}

//---------------------------------------------------------
// Desc:   apply y mask of each children to the y mask of the current tree node
//---------------------------------------------------------
void QuadTreeNode::RebuildYMask()
{
    // reset our overall y mask to the mask
    // defined by our local members only
    yMask_ = yLocalMask_;

    // sum up the masks of our children
    for (int i = 0; i < 4; ++i)
    {
        if (pChildNodes_[i])
        {
            yMask_.SetFlags(pChildNodes_[i]->GetYMask());
        }
    }
}

//---------------------------------------------------------
// Desc:   after adding a new member (scene object) into the quad tree's node
//         we have to update Y mask of this and all the parent nodes
//---------------------------------------------------------
void QuadTreeNode::DescendantMemberAdded(const u32Flags& yMask)
{
    // update our yMask
    yMask_.SetFlags(yMask);

    // notify our parent of the addition
    if (pParentNode_)
    {
        pParentNode_->DescendantMemberAdded(yMask);
    }
}

//---------------------------------------------------------
// Desc:   after removing a member (scene object) from the quad tree's node
//         we have to update Y mask of this and all the parent nodes 
//---------------------------------------------------------
void QuadTreeNode::DescendantMemberRemoved()
{
    // update our yMask
    RebuildYMask();

    // notify our parent of the removal
    if (pParentNode_)
    {
        pParentNode_->DescendantMemberRemoved();
    }
}

//---------------------------------------------------------
// Desc:   calling this func assumes that the 2D search rectangle
//         contains this node COMPLETELY, so all we need to test against
//         is the Y range specified and input frustum
//---------------------------------------------------------
void QuadTreeNode::TestLocalMembersForSearchResults(
    EntityID* visibleSceneObjects,
    int& numVisibleSceneObjects,
    const u32Flags yMask,
    const float yMin,
    const float yMax,
    const Frustum* pFrustum)
{
    assert(pFrustum != nullptr);

    SceneObject* pObj = nullptr;

    // if this node is "vertically" seen
    if (yLocalMask_.TestAny(yMask))
    {
        // go through each member (scene object) of this quad tree node
        for (pObj = pFirstMember_; pObj; pObj = pObj->GetNextTreeLink())
        {
            // if the object is "vertically" seen
            if (pObj->GetYMask().TestAny(yMask))
            {
                if (pFrustum->TestRect(pObj->GetWorldBounds()))
                {
                    // add this scene object into arr of visible entities
                    visibleSceneObjects[numVisibleSceneObjects++] = pObj->enttId_;
                }
            }
        }
    }
}

//---------------------------------------------------------
// Desc:   calling this func assumes that the 2D search rectangle
//         INTERSECTS this node, so we need to test against is the yMask
//         bit patterns as well a the search area for our local members
//---------------------------------------------------------
void QuadTreeNode::TestLocalMembersForSearchResults(
    EntityID* visibleSceneObjects,
    int& numVisibleSceneObjects,
    const u32Flags yMask,
    const Rect3d& trueRect,
    const Frustum* pFrustum)
{
    assert(pFrustum != nullptr);

    SceneObject* pObj = nullptr;
    Rect3d result;

    // if this node is "vertically" seen
    if (yLocalMask_.TestAny(yMask))
    {
        // go through each member (scene object) of this quad tree node
        for (pObj = pFirstMember_; pObj; pObj = pObj->GetNextTreeLink())
        {
            // if this object is "vertically" seen
            if (pObj->GetYMask().TestAny(yMask))
            {
                // if we have any intersection btw input rect and object's rect
                if (IntersectRect3d(trueRect, pObj->GetWorldBounds(), result))
                {
                    if (pFrustum->TestRect(pObj->GetWorldBounds()))
                    {
                        // add this scene object into arr of visible entities
                        visibleSceneObjects[numVisibleSceneObjects++] = pObj->enttId_;
                    }
                }
            }
        }
    }
}

} // namespace
