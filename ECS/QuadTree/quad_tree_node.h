/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: quad_tree_node.h
    Desc:     implementation of quad tree node

              A QuadTreeNode represents a single node in our quad tree. Even though
              the QuadTree is a 2D spatial organization, each member of the node is
              given a 32bit number (called yMask) representing the areas of Y that
              they occupy in the world.
              The world Y extent is divided into 32 areas, each one represented by
              a bit in the yMask. Therefore, when testing with a 3D rectangle,
              the node can set a yMask bit pattern representing the Y areas begin
              searched and cull it's members by AND'ing (binary AND operation)
              the search yMask with the members yMask to see if there is any overlap.

              This code I mainly rewrote from sources to the amazing book
              "Real Time 3D Terrain Engines Using C++ and DirectX"
              by Greg Snook

    Created:  15.09.2025 by DimaSkup
\**********************************************************************************/
#pragma once

#include <types.h>
#include <bit_flags.h>
#include <geometry/rect3d.h>
#include <math/math_helpers.h>
#include <numeric_tools.h>


//------------------------------------------
// forward declarations (pointer use only)
//------------------------------------------
class Frustum;
class SceneObject;
class QuadTree;

//------------------------------------------
//  our worlds may have different sizes but we need somehow to scale
//  3D rectangles of our objects from world space into
//  "quad tree space" (which always has a fixed size) so we will
//  be able to properly handle such rectangles within quad trees
//------------------------------------------
class QuadTreeRect
{
public:
    int x0, x1;
    int y0, y1;
    int z0, z1;

    QuadTreeRect() {}
    ~QuadTreeRect() {}

    QuadTreeRect(const QuadTreeRect& src) :
        x0(src.x0), x1(src.x1),
        y0(src.y0), y1(src.y1),
        z0(src.z0), z1(src.z1)
    {}

    QuadTreeRect(const int _x0, const int _x1,
                 const int _y0, const int _y1,
                 const int _z0, const int _z1) :
        x0(_x0), x1(_x1),
        y0(_y0), y1(_y1),
        z0(_z0), z1(_z1)
    {}

    void Convert(const Rect3d& worldRect, const Vec3& offset, const Vec3& scale);
};

//------------------------------------------
// build a yMask by input min and max points
//------------------------------------------
inline uint32 YMASK(const uint8 ymin, const uint8 ymax)
{
    uint32 high      = (1 << ymax);
    uint32 low       = (1 << ymin);
    uint32 setMask   = high - 1;
    uint32 clearMask = low - 1;

    uint32 result = setMask;
    if (ymin)
    {
        result &= ~clearMask;
    }

    result |= high;
    result |= low;

    return result;
}

//------------------------------------------
// class name:  QuadTreeNode
//------------------------------------------
class QuadTreeNode
{
public:
    QuadTreeNode();
    ~QuadTreeNode();

    uint32 AddOrUpdateMember(SceneObject* pMember, const QuadTreeRect& rect);
    void   RemoveMember(SceneObject* pMember);

    void Search(
        SceneObject** pResultList,
        SceneObject** pResultListTail,
        const u32Flags& yMask,
        const float yMin,
        const float yMax,
        const Frustum* pOptionalFrustum = nullptr);

    void Search(
        SceneObject** pResultList,
        SceneObject** pResultListTail,
        const u32Flags& yMask,
        const Rect3d& worldRect,
        const Frustum* pOptionalFrustum = nullptr);

    bool     IsEmpty()       const;
    u32Flags GetYMask()      const;
    u32Flags GetYLocalMask() const;

private:

    // private functions...
    void DescendantMemberAdded(u32Flags yMask);
    void DescendantMemberRemoved();
    void RebuildLocalYMask();
    void RebuildYMask();

    // functions available to the quad tree
    friend QuadTree;

    void Setup(QuadTreeNode* pParent,
               QuadTreeNode* pChild0,
               QuadTreeNode* pChild1,
               QuadTreeNode* pChild2,
               QuadTreeNode* pChild3);


    // private data...
    QuadTreeNode* pChildNode_[4];
    QuadTreeNode* pParentNode_;
    SceneObject*  pFirstMember_;

    u32Flags      yLocalMask_;   // mask only for the members of current node
    u32Flags      yMask_;        // mask for the members of current node and all the descendant nodes as well
};

//---------------------------------------------------------
// inline functions
//---------------------------------------------------------
inline QuadTreeNode::QuadTreeNode() :
    pParentNode_(nullptr),
    pFirstMember_(nullptr),
    yMask_(0),
    yLocalMask_(0)
{
    memset(pChildNode_, 0, sizeof(pChildNode_));
}

inline QuadTreeNode::~QuadTreeNode()
{
    assert((yMask_.value_ == 0) && "the quad tree still has members");
}

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

