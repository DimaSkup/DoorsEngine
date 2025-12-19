/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: quad_tree_node.h
    Desc:     implementation of quad tree nodes

              This code I mainly rewrote from sources to the amazing book
              "Real Time 3D Terrain Engines Using C++ and DirectX"
              by Greg Snook

    Created:  15.09.2025 by DimaSkup
\**********************************************************************************/
#pragma once

#include "quad_tree_rect.h"
#include <Types.h>
#include <math/math_helpers.h>
#include <geometry/rect_3d.h>
#include <geometry/rect_3d_functions.h>
#include <geometry/frustum.h>
#include <bit_flags.h>
#include <log.h>
#include <math.h>


namespace Core
{

class SceneObject;
class QuadTree;

//==================================================================================
//   QuadTreeNode represents a single node in our quad tree. Even though
//   the QuadTree is a 2D spatial organization, each member of the node is
//   given a 32bit number (called a yMask) representing the areas of Y
//   that they occupy in the world. The world Y extent is divided into 32 areas,
//   each one represented by a bit in the yMask. Therefore, when testing with
//   a 3D rectangle, the node can set a yMask bit pattern representing the Y areas
//   being searched and cull it's members by And'ing the search yMask with
//   the members yMask to see if there is any overlap.
//==================================================================================
inline uint32 YMASK(const uint8 yMin, const uint8 yMax)
{
    const uint32 low  = (1 << yMin);
    const uint32 high = (1 << yMax);
    const uint32 setMask   = high - 1;
    const uint32 clearMask = low - 1;

    uint32 result = setMask;
    if (yMin)
    {
        result &= ~clearMask;
    }

    result |= high;
    result |= low;

    return result;
}



//==================================================================================
// class: QuadTreeNode
//==================================================================================
class QuadTreeNode
{
public:
    QuadTreeNode();
    ~QuadTreeNode();

    uint32 AddOrUpdateMember(SceneObject* pMember, const QuadTreeRect& rect);
    void RemoveMember(SceneObject* pMember);

    void TestLocalMembersForSearchResults(
        EntityID* visibleSceneObjects,
        int& numVisibleSceneObjects,
        const u32Flags yMask,
        const float yMin,
        const float yMax,
        const Frustum* pFrustum);

    void TestLocalMembersForSearchResults(
        EntityID* visibleSceneObjects,
        int& numVisibleSceneObjects,
        const u32Flags yMask,
        const Rect3d& trueRect,
        const Frustum* pFrustum);

    bool IsEmpty() const;
    u32Flags GetYMask() const;
    u32Flags GetYLocalMask() const;

private:

    // private functions
    void DescendantMemberAdded(const u32Flags& yMask);
    void DescendantMemberRemoved();
    void RebuildLocalYMask();
    void RebuildYMask();

    // functions available to the quad tree
    friend QuadTree;
    void Setup(QuadTreeNode* pParent, QuadTreeNode* pChild0, QuadTreeNode* pChild1, QuadTreeNode* pChild2, QuadTreeNode* pChild3);

    // private data
    QuadTreeNode* pChildNodes_[4]{nullptr};
    QuadTreeNode* pParentNode_  = nullptr;
    SceneObject*  pFirstMember_ = nullptr;

    u32Flags yLocalMask_ = 0;
    u32Flags yMask_      = 0;
};

//==================================================================================
// INLINE METHODS
//==================================================================================

//---------------------------------------------------------
// Desc:   default constructor
//---------------------------------------------------------
inline QuadTreeNode::QuadTreeNode() :
    pParentNode_(nullptr),
    pFirstMember_(nullptr),
    yLocalMask_(0),
    yMask_(0)
{
    pChildNodes_[0] = nullptr; 
    pChildNodes_[1] = nullptr; 
    pChildNodes_[2] = nullptr; 
    pChildNodes_[3] = nullptr; 
}

//---------------------------------------------------------
// Desc:    default destructor
//---------------------------------------------------------
inline QuadTreeNode::~QuadTreeNode()
{
    if (yMask_.value_ != 0)
        LogErr(LOG, "the quad tree still has members");
}

//---------------------------------------------------------
// Desc:    accessors / getters
//---------------------------------------------------------
inline bool QuadTreeNode::IsEmpty() const
{
    return yMask_.value_ == 0;
}

inline u32Flags QuadTreeNode::GetYMask() const
{
    return yMask_;
}

inline u32Flags QuadTreeNode::GetYLocalMask() const
{
    return yLocalMask_;
}

} // namespace
