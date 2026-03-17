/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: quad_tree.cpp
    Desc:     quad-tree functional implementation

              This code I mainly rewrote from sources to the amazing book
              "Real Time 3D Terrain Engines Using C++ and DirectX"
              by Greg Snook

    Created:  16.09.2025 by DimaSkup
\**********************************************************************************/
#include "../Common/pch.h"
#include "quad_tree.h"
#include "scene_object.h"


void QuadTree::Init(const Rect3d& worldAABB, const int depth)
{
    assert(!IsReady() && "the quad tree has already been created");
    assert(depth >= MIN_TREE_DEPTH && "invalid tree depth");
    assert(depth <= MAX_TREE_DEPTH && "invalid tree depth");

    depth_ = depth;
    worldExtents_ = worldAABB.Size();
    worldOffset_ = -worldAABB.MinPoint();

    worldScale_.x = 256.0f / worldExtents_.x;
    worldScale_.y = 32.0f  / worldExtents_.y;
    worldScale_.z = 256.0f / worldExtents_.z;

    // allocate the nodes
    memorySize_ = 0;

    for (int i = 0; i < depth_; ++i)
    {
        const int nodeCount = (1<<i) * (1<<i);
        const uint32 size   = sizeof(QuadTreeNode) * nodeCount;

        levelNodes_[i] = new QuadTreeNode[nodeCount];

        memorySize_ += size;
    }

    // setup each node
    for (int i = 0; i < depth_; ++i)
    {
        int levelDimension = (1<<i);
        int levelIndex = 0;

        for (int z = 0; z < levelDimension; ++z)
        {
            for (int x = 0; x < levelDimension; ++x)
            {
                levelNodes_[i][levelIndex].Setup(
                    GetNodeFromLevelXZ(i-1, (x>>1),   (z>>1)),     // parent node
                    GetNodeFromLevelXZ(i+1, (x<<1),   (z<<1)),     // child: bottom left
                    GetNodeFromLevelXZ(i+1, (x<<1)+1, (z<<1)),     // child: bottom right
                    GetNodeFromLevelXZ(i+1, (x<<1),   (z<<1)+1),   // child: top left
                    GetNodeFromLevelXZ(i+1, (x<<1)+1, (z<<1)+1));  // child: top right

                levelIndex++;
            }
        }
    }
}

//---------------------------------------------------------
// Desc:  release memory from this quad tree
//---------------------------------------------------------
void QuadTree::Shutdown()
{
    for (int i = 0; i < MAX_TREE_DEPTH; ++i)
        SafeDeleteArr(levelNodes_[i]);

    depth_ = 0;
}

//---------------------------------------------------------
// Desc:  get depth level, and x,z index of quad tree node
//        that completely fits in to input rectangle
// NOTE:  to understand wtf is going on you should read
//        the book mentioned in the header
//---------------------------------------------------------
void QuadTree::FindTreeNodeInfo(const QuadTreeRect& rect, int& level, int& levelX, int& levelZ)
{
    const int xPattern = rect.x0 ^ rect.x1;
    const int zPattern = rect.z0 ^ rect.z1;

    const int bitPattern = Max(xPattern, zPattern);
    const int highBit    = (bitPattern) ? HighestBitSet(bitPattern)+1 : 0;

    level = MAX_TREE_DEPTH - highBit - 1;
    level = Min(level, depth_ - 1);

    const int shift = MAX_TREE_DEPTH - level - 1;

    levelX = rect.x1 >> shift;
    levelZ = rect.z1 >> shift;
}

//---------------------------------------------------------
// Desc:  return a ptr to quad tree node that fits in the input rectangle
//---------------------------------------------------------
QuadTreeNode* QuadTree::FindTreeNode(const QuadTreeRect& rect)
{
    int level, levelX, levelZ;

    FindTreeNodeInfo(rect, level, levelX, levelZ);

    return GetNodeFromLevelXZ(level, levelX, levelZ);
}

//---------------------------------------------------------
// Desc:  build a rectangle in "quad tree space" by input world rectangle (AABB)
//---------------------------------------------------------
void QuadTree::BuildByteRect(const Rect3d& worldRect, QuadTreeRect& outRect)
{
    outRect.Convert(worldRect, worldOffset_, worldScale_);
}

//---------------------------------------------------------
// Desc:  add a new object into quad tree,  or just update
//        its location within the quad tree (if it is already in this quad tree)
//---------------------------------------------------------
u32Flags QuadTree::AddOrUpdateSceneObject(SceneObject* pObj)
{
    QuadTreeRect byteRect;
    byteRect.Convert(pObj->GetWorldBox(), worldOffset_, worldScale_);

    QuadTreeNode* pNode = FindTreeNode(byteRect);
    assert(pNode && "failed to locate quad tree node");

    return pNode->AddOrUpdateMember(pObj, byteRect);
}

//---------------------------------------------------------
// Desc:  get all the scene objects within input world rectangle
//        and frustum (optional) volume
//---------------------------------------------------------
SceneObject* QuadTree::Search(
    const Rect3d& worldRect,
    const Frustum* pOptionalFrustum)
{
    SceneObject* pResultListBeg = nullptr;
    SceneObject* pResultListEnd = nullptr;

    QuadTreeRect byteRect;
    BuildByteRect(worldRect, byteRect);

    u32Flags yMask = YMASK(byteRect.y0, byteRect.y1);

    bool bContinueSearch = true;
    int level = 0;

    const u32Flags zeroYMask = 0;

    while (level < depth_ && bContinueSearch)
    {
        int shiftCount = 8 - level;

        QuadTreeRect localRect(byteRect.x0 >> shiftCount,
                               byteRect.x1 >> shiftCount,
                               0, 0,
                               byteRect.z0 >> shiftCount,
                               byteRect.z1 >> shiftCount);

        // do not continue unless a populated node is found
        bContinueSearch = false;

        for (int z = localRect.z0; z <= localRect.z1; ++z)
        {
            for (int x = localRect.x0; x <= localRect.x1; ++x)
            {
                QuadTreeNode* pNode = GetNodeFromLevelXZ(level, x, z);
                assert(pNode);

                if (!(pNode->GetYMask() & yMask))
                    continue;

                // a populated node has been found
                bContinueSearch = true;

                // search all the edge cells with the full world rectangle,
                // but non-edge cells are completely contained within the search rect
                // and can be called with just the y extents
                if (z == localRect.z0 || z == localRect.y1 ||
                    x == localRect.x0 || x == localRect.x1)
                {
                    // test all the members of this node against the world rect
                    pNode->Search(
                        &pResultListBeg,
                        &pResultListEnd,
                        yMask,
                        worldRect,
                        pOptionalFrustum);
                }
                else
                {
                    // test all members of this node agains the world Y extents only
                    pNode->Search(
                        &pResultListBeg,
                        &pResultListEnd,
                        yMask,
                        worldRect.y0,
                        worldRect.y1,
                        pOptionalFrustum);
                }
            }
        }

        // step up to the next level of the tree
        ++level;
    }

    return pResultListBeg;
}
