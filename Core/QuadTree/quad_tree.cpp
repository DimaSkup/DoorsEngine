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
#include "quad_tree.h"
#include <math/vec_functions.h>
#include <mem_helpers.h>
#include <numeric_tools.h>
#include "scene_object.h"

namespace Core
{
    
//---------------------------------------------------------
// global instance of the quad tree
//---------------------------------------------------------
QuadTree g_QuadTree;

//---------------------------------------------------------
// Desc:   generate a quad tree
//---------------------------------------------------------
void QuadTree::Create(const Rect3d& worldBoundingBox, int depth)
{
    // prevent double initialization
    if (IsReady())
    {
        LogErr(LOG, "the quad tree has already been created!");
        return;
    }

    // check if input depth for quat tree is valid, if no we clamp it
    if (depth < MIN_QTREE_DEPTH || depth > MAX_QTREE_DEPTH)
    {
        LogErr(LOG, "invalid tree depth (%d); clamp it to range: [%d, %d]", depth, (int)MIN_QTREE_DEPTH, (int)MAX_QTREE_DEPTH);
        Clamp(depth, (int)MIN_QTREE_DEPTH, (int)MAX_QTREE_DEPTH);
    }


    qTreeDepth_   = depth;
    worldExtents_ = worldBoundingBox.Size();
    worldOffset_  = -worldBoundingBox.MinPoint();

    // compute boundaries for this quad tree
    worldScale_.x = 256.0f / worldExtents_.x;
    worldScale_.y = 32.0f  / worldExtents_.y;
    worldScale_.z = 256.0f / worldExtents_.z;


    // allocate the nodes of quadtree
    memSize_ = 0;

    for (int i = 0; i < depth; ++i)
    {
        const int numNodes = (1<<i)*(1<<i);
        const uint32 size  = sizeof(QuadTreeNode) * numNodes;

        levelNodes_[i] = new QuadTreeNode[numNodes];
        memSize_ += size;
    }

    // setup each node
    for (int i = 0; i < depth; ++i)
    {
        const int levelDimension = (1 << i);
        int       levelIdx       = 0;

        for (int z = 0; z < levelDimension; ++z)
        {
            for (int x = 0; x < levelDimension; ++x)
            {
                levelNodes_[i][levelIdx].Setup(
                    GetNodeFromLevelXZ(i-1, (x>>1),   (z>>1)),
                    GetNodeFromLevelXZ(i+1, (x<<1),   (z<<1)),
                    GetNodeFromLevelXZ(i+1, (x<<1)+1, (z<<1)),
                    GetNodeFromLevelXZ(i+1, (x<<1),   (z<<1)+1),
                    GetNodeFromLevelXZ(i+1, (x<<1)+1, (z<<1)+1));

                levelIdx++;
            }
        }
    }
}

//---------------------------------------------------------
// Desc:   destroy the quad tree and release all the related memory
//---------------------------------------------------------
void QuadTree::Destroy()
{
    for (int i = 0; i < (int)MAX_QTREE_DEPTH; ++i)
    {
        SafeDeleteArr(levelNodes_[i]);
    }
    qTreeDepth_ = 0;
}

//---------------------------------------------------------
// Desc:   find a quad tree node info by input 3d rectangle in the world
// Output: - level:           quad tree level of the node
//         - levelX, levelZ:  index of the node along X and Z-axis
//---------------------------------------------------------
void QuadTree::FindTreeNodeInfo(
    const QuadTreeRect& worldByteRect,
    int& level,
    int& levelX,
    int& levelZ)
{
    const int xPattern   = worldByteRect.x0 ^ worldByteRect.x1;
    const int zPattern   = worldByteRect.z0 ^ worldByteRect.z1;

    const int bitPattern = FastMax(xPattern, zPattern);
    const int highBit    = bitPattern ? HighestBitSet(bitPattern)+1 : 0;

    level = MAX_QTREE_DEPTH - highBit - 1;
    level = FastMin(level, qTreeDepth_-1);

    const int shift = MAX_QTREE_DEPTH - level - 1;

    levelX = worldByteRect.x1 >> shift;
    levelZ = worldByteRect.z1 >> shift;
}

//---------------------------------------------------------
// Desc:   find a quad tree node by input 3d rectangle in the world
// Ret:    a ptr to quad tree node or null if there is no such a node
//---------------------------------------------------------
QuadTreeNode* QuadTree::FindTreeNode(const QuadTreeRect& worldByteRect)
{
    int level, levelX, levelZ;

    FindTreeNodeInfo(worldByteRect, level, levelX, levelZ);

    return GetNodeFromLevelXZ(level, levelX, levelZ);
}

//---------------------------------------------------------
// Desc:   convert input 3d rectangle to integer values,
//         taking the floor of each real
// Args:   - worldRect:      a 3d rectangle with float coords
// Out:    - worldByteRect:  a 3d rectangle with integer coords
//---------------------------------------------------------
void QuadTree::BuildByteRect(const Rect3d& worldRect, QuadTreeRect& worldByteRect)
{
    worldByteRect.Convert(worldRect, worldOffset_, worldScale_);
}

//---------------------------------------------------------
// Desc:   add a new scene object into quad tree
//         or just update it
// Args:   - pNode: a ptr to scene object
// Ret:    a YMask (bitfield) for this scene object
//---------------------------------------------------------
u32Flags QuadTree::AddOrUpdateSceneObject(SceneObject* pObj)
{
    QuadTreeRect byteRect;
    BuildByteRect(pObj->GetWorldBounds(), byteRect);

    QuadTreeNode* pNode = FindTreeNode(byteRect);
    if (!pNode)
    {
        LogErr(LOG, "failed to locate quad tree node");
        return 0;
    }

    return pNode->AddOrUpdateMember(pObj, byteRect);
}

//---------------------------------------------------------
// Desc:   add a new scene object into quad-tree
// Args:   - enttId:     identifier of related entity
//         - worldRect:  world boundaries (AABB)
//---------------------------------------------------------
void QuadTree::AddSceneObject(const EntityID enttId, const Rect3d& worldRect)
{
    assert(currNumSceneObjects_ != MAX_NUM_SCENE_OBJECTS && "you have too many scene objects");

    SceneObject& obj = sceneObjectsPool_[currNumSceneObjects_];
    currNumSceneObjects_++;

    // setup this scene object
    obj.enttId_ = 5;
    obj.worldBounds_ = worldRect;
    obj.AttachToQuadTree(this);
}

//---------------------------------------------------------
// Desc:   update a scene object by id
//---------------------------------------------------------
void QuadTree::UpdateSceneObject(const EntityID enttId)
{

}


//---------------------------------------------------------
// 
//---------------------------------------------------------
void QuadTree::CalcVisibleEntities(const Rect3d& cameraWorldRect, const Frustum& frustum)
{
    SceneObject* pResultListStart = nullptr;
    SceneObject* pResultListEnd   = nullptr;

    QuadTreeRect byteRect;
    BuildByteRect(cameraWorldRect, byteRect);

    // reset the counter of visible entities before calculation
    currNumVisibleEntts_ = 0;

    const u32Flags yMask = YMASK(byteRect.z0, byteRect.z1);

    bool continueSearch = true;
    int level = 0;

    while (level < qTreeDepth_ && continueSearch)
    {
        const int shiftCount = 8 - level;

        QuadTreeRect localRect(
            byteRect.x0 >> shiftCount,
            byteRect.x1 >> shiftCount,
            0,                              // y0
            0,                              // y1
            byteRect.z0 >> shiftCount,
            byteRect.z1 >> shiftCount);

        // do not continue unless a populated node is found
        continueSearch = false;

        for (int z = localRect.z0; z <= localRect.z1; ++z)
        {
            for (int x = localRect.x0; x <= localRect.x1; ++x)
            {
                QuadTreeNode* pNode = GetNodeFromLevelXZ(level, x, z);

                if (pNode->GetYMask() & yMask)
                {
                    // a populated node has been found
                    continueSearch = true;

                    // search all the edge cells with the full world rectangle,
                    // all non-edge cells are contained within the search rect
                    // and be called with just the y extents
                    if (z == localRect.z0 ||
                        z == localRect.z1 ||
                        x == localRect.x0 ||
                        x == localRect.x1)
                    {
                        // test all members of this node agains the world rect
                        pNode->TestLocalMembersForSearchResults(
                            visibleEntts_,
                            currNumVisibleEntts_,
                            yMask,
                            cameraWorldRect,
                            &frustum);
                    }
                    else
                    {
                        // test all members of this node against
                        // the world Y extents only
                        pNode->TestLocalMembersForSearchResults(
                            visibleEntts_,
                            currNumVisibleEntts_,
                            yMask,
                            cameraWorldRect.y0,
                            cameraWorldRect.y1,
                            &frustum);
                    }
                }
            }
        }

        // step to the next level of the tree
        ++level;
    }
}

} // namespace
