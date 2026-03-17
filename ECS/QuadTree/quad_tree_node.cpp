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
#include "../Common/pch.h"
#include "quad_tree_node.h"
#include "scene_object.h"
#include <geometry/frustum.h>
#include <geometry/rect3d_functions.h>
#include <bit_flags_functions.h>


//---------------------------------------------------------
// convert AABB from world space into "quad tree space"
//---------------------------------------------------------
void QuadTreeRect::Convert(const Rect3d& worldRect, const Vec3& offset, const Vec3& scale)
{
    Rect3d convertedRect(worldRect);

    // reposition and scale world coordinates to quad tree coordinates
    convertedRect += offset;
    convertedRect *= scale;

    // reduce by a tiny amount to handle tiled data
    convertedRect.x1 -= 0.01f;
    convertedRect.y1 -= 0.01f;
    convertedRect.z1 -= 0.01f;

    convertedRect.x1 = Max(convertedRect.x1, convertedRect.x0);
    convertedRect.y1 = Max(convertedRect.y1, convertedRect.y0);
    convertedRect.z1 = Max(convertedRect.z1, convertedRect.z0);

    // convert to integer values, throwing away the fractional part
    x0 = RealToInt32_chop(convertedRect.x0);
    x1 = RealToInt32_chop(convertedRect.x1);
    y0 = RealToInt32_chop(convertedRect.y0);
    y1 = RealToInt32_chop(convertedRect.y1);
    z0 = RealToInt32_chop(convertedRect.z0);
    z1 = RealToInt32_chop(convertedRect.z1);

    // we must be positive
    x0 = Clamp(x0, 0, 254);
    y0 = Clamp(y0, 0, 30);
    z0 = Clamp(z0, 0, 254);

    // and must be at least one unit large
    x1 = Clamp(x1, x0 + 1, 255);
    y1 = Clamp(y1, y0 + 1, 31);
    z1 = Clamp(z1, z0 + 1, 255);
}

//---------------------------------------------------------
// Desc:  setup parent and children to this node
//---------------------------------------------------------
void QuadTreeNode::Setup(
    QuadTreeNode* pParent,
    QuadTreeNode* pChild0,
    QuadTreeNode* pChild1,
    QuadTreeNode* pChild2,
    QuadTreeNode* pChild3)
{
    assert(pParentNode_ == nullptr && "parent node is already set");

    pParentNode_ = pParent;
    pChildNode_[0] = pChild0;
    pChildNode_[1] = pChild1;
    pChildNode_[2] = pChild2;
    pChildNode_[3] = pChild3;
}

//---------------------------------------------------------
// Desc:  add a new member to this node or update its location
//        within the quad tree
// Args:  - rect:  world bounding of input member converted to "quad tree space"
//---------------------------------------------------------
uint32 QuadTreeNode::AddOrUpdateMember(SceneObject* pMember, const QuadTreeRect& rect)
{
    assert(pMember);

    const u32Flags yMask = YMASK(rect.y0, rect.y1);

    // is this node not already a member?
    if (pMember->GetQuadTreeNode() != this)
    {
        // remove the member from it's previous quad tree node (if any)
        if (pMember->GetQuadTreeNode())
            pMember->GetQuadTreeNode()->RemoveMember(pMember);

        // account for the new addition
        if (!pFirstMember_)
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
            pParentNode_->DescendantMemberAdded(yMask);
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
// Desc:  detach input member from the quad tree node
//        and update yMask of this node
//---------------------------------------------------------
void QuadTreeNode::RemoveMember(SceneObject* pMember)
{
    // make sure this is one of ours
    assert(pMember->GetQuadTreeNode() == this && "error removing quad tree member");

    SceneObject* pPrev = pMember->GetPrevTreeLink();
    SceneObject* pNext = pMember->GetNextTreeLink();

    // remove this member from it's chain
    if (pNext)
        pNext->SetPrevTreeLink(pPrev);

    if (pPrev)
        pPrev->SetNextTreeLink(pNext);

    // if this was our first member, advance our pointer to the next member
    if (pFirstMember_ == pMember)
        pFirstMember_ = pNext;

    // clear the former members links
    pMember->SetPrevTreeLink(nullptr);
    pMember->SetNextTreeLink(nullptr);

    // rebuild our yMask
    RebuildLocalYMask();

    // notify our parent
    if (pParentNode_)
        pParentNode_->DescendantMemberRemoved();
}

//---------------------------------------------------------
// Desc:  rebuild yMask only for the members of current node
//---------------------------------------------------------
void QuadTreeNode::RebuildLocalYMask()
{
    yLocalMask_.Clear();

    // for each local member
    SceneObject* pObj = pFirstMember_;
    while (pObj)
    {
        yLocalMask_.SetFlags(pObj->GetYMask());
        pObj = pObj->GetNextTreeLink();
    }

    // the combined yMask must now be updated
    RebuildYMask();
}

//---------------------------------------------------------
// Desc:  rebuild combined yMask (a mask for local members of node
//        and all the descendant nodes members as well)
//---------------------------------------------------------
void QuadTreeNode::RebuildYMask()
{
    // reset our overall y-mask to the mask
    // defined by our local members only
    yMask_ = yLocalMask_;

    // sum up the masks of our children
    for (int i = 0; i < 4; ++i)
    {
        if (pChildNode_[i])
        {
            yMask_.SetFlags(pChildNode_[i]->GetYMask());
        }
    }
}

//---------------------------------------------------------
// Desc:  is called from some child node to notify its parent (current node)
//        that we have added some new member (object)
//---------------------------------------------------------
void QuadTreeNode::DescendantMemberAdded(const u32Flags yMask)
{
    // update our yMask
    yMask_.SetFlags(yMask);

    // notify our parent of the addition as well
    if (pParentNode_)
        pParentNode_->DescendantMemberAdded(yMask);
}

//---------------------------------------------------------
// Desc:  is called from some child node to notify its parent (current node)
//        that we have removed some member (object)
//---------------------------------------------------------
void QuadTreeNode::DescendantMemberRemoved()
{
    // update our yMask
    RebuildYMask();

    // notify our parent of the removal
    if (pParentNode_)
        pParentNode_->DescendantMemberRemoved();
}

//---------------------------------------------------------
// Desc:  calling this func assumes that the 2D search rectangle contains this 
//        node completely, so all we need is to test against the y-range
//        specified as input param and the optional frustum
//---------------------------------------------------------
void QuadTreeNode::Search(
    SceneObject** pResultList,
    SceneObject** pResultListTail,
    const u32Flags& yMask,
    const float yMin,
    const float yMax,
    const Frustum* pOptionalFrustum)
{
    if (!yLocalMask_.TestAny(yMask))
        return;


    // if we have a frustum we will do frustum tests as well
    if (pOptionalFrustum)
    {
        for (SceneObject* pObj = pFirstMember_; pObj; pObj = pObj->GetNextTreeLink())
        {
            // yMask test
            if (!pObj->GetYMask().TestAny(yMask))
                continue;

            // frustum/rect test
           // if (!pOptionalFrustum->TestRect(pObj->GetWorldBox()))
           //     continue;

            if (!pOptionalFrustum->TestSphere(pObj->GetWorldSphere()))
                continue;

            if (*pResultListTail)
            {
                // append to results list
                pObj->AttachToSearchResult(*pResultListTail, nullptr);
                *pResultListTail = pObj;
            }
            else
            {
                // this is the first result in the list
                pObj->ClearSearchResults();
                *pResultList = pObj;
                *pResultListTail = pObj;
            }
        }
    }

    // we don't have any frustum for the current search so test only against Y-mask
    else
    {
        for (SceneObject* pObj = pFirstMember_; pObj; pObj = pObj->GetNextTreeLink())
        {
            // yMask test
            if (!pObj->GetYMask().TestAny(yMask))
                continue;

            if (*pResultListTail)
            {
                // append to results list
                pObj->AttachToSearchResult(*pResultListTail, nullptr);
                *pResultListTail = pObj;
            }
            else
            {
                // this is the first result in the list
                pObj->ClearSearchResults();
                *pResultList = pObj;
                *pResultListTail = pObj;
            }
        }
    }
}

//---------------------------------------------------------
// Desc:  calling this func assumes that the 2D search rectangle intersects this node,
//        so we need to test against the zMask bit patterns as well as the search
//        area (3D rectangle -- AABB) for our local members
//---------------------------------------------------------
void QuadTreeNode::Search(
    SceneObject** pResultList,
    SceneObject** pResultListTail,
    const u32Flags& yMask,
    const Rect3d& worldRect,
    const Frustum* pOptionalFrustum)
{
    if (!yLocalMask_.TestAny(yMask))
        return;

    // if we have a frustum we will do frustum tests as well
    if (pOptionalFrustum)
    {
        for (SceneObject* pObj = pFirstMember_; pObj; pObj = pObj->GetNextTreeLink())
        {
            if (!pObj->GetYMask().TestAny(yMask))
                continue;

            if (!IntersectRect3d(worldRect, pObj->GetWorldBox()))
                continue;

            //if (!pOptionalFrustum->TestRect(pObj->GetWorldBox()))
            //    continue;

            if (!pOptionalFrustum->TestSphere(pObj->GetWorldSphere()))
                continue;

            if (*pResultListTail)
            {
                // append to results list
                pObj->AttachToSearchResult(*pResultListTail, nullptr);
                *pResultListTail = pObj;
            }
            else
            {
                // this is the first result in list
                pObj->ClearSearchResults();
                *pResultList = pObj;
                *pResultListTail = pObj;
            }
        }
    }

    // we don't have any frustum for the current search so test only against Y-mask
    else
    {
        for (SceneObject* pObj = pFirstMember_; pObj; pObj = pObj->GetNextTreeLink())
        {
            // yMask test
            if (!pObj->GetYMask().TestAny(yMask))
                continue;

            if (!IntersectRect3d(worldRect, pObj->GetWorldBox()))
                continue;

            if (*pResultListTail)
            {
                // append to results list
                pObj->AttachToSearchResult(*pResultListTail, nullptr);
                *pResultListTail = pObj;
            }
            else
            {
                // this is the first result in the list
                pObj->ClearSearchResults();
                *pResultList = pObj;
                *pResultListTail = pObj;
            }
        }
    }
}
