/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: quad_tree.h
    Desc:     quad-tree class declaration

              This code I mainly rewrote from sources to the amazing book
              "Real Time 3D Terrain Engines Using C++ and DirectX"
              by Greg Snook

    Created:  13.09.2025 by DimaSkup
\**********************************************************************************/
#pragma once
#include "quad_tree_node.h"
#include <geometry/rect3d.h>
#include <bit_flags.h>
#include <memory.h>
#include <assert.h>


//------------------------------------------
// forward declarations (pointer use only)
//------------------------------------------
class Frustum;
class SceneObject;

//------------------------------------------
// class name:  QuadTree
//------------------------------------------
class QuadTree
{
public:

    // data types & constants...
    enum eConstants
    {
        MIN_TREE_DEPTH = 1,
        MAX_TREE_DEPTH = 9,   // must be a value between 1 and 9
    };


    // creators...
    QuadTree();
    ~QuadTree();

    void Init(const Rect3d& worldAABB, const int depth);
    void Shutdown();

    // mutators..
    SceneObject* Search(
        const Rect3d& worldRect,
        const Frustum* pOptionalFrustum = nullptr);

    u32Flags AddOrUpdateSceneObject(SceneObject* pNewObj);

    // accessors...
    bool IsReady() const;

private:

    // private functions...
    void          FindTreeNodeInfo  (const QuadTreeRect& rect, int& level, int& levelX, int& levelZ);
    QuadTreeNode* FindTreeNode      (const QuadTreeRect& rect);
    QuadTreeNode* GetNodeFromLevelXZ(const int level, const int x, const int z);
    void          BuildByteRect     (const Rect3d& worldRect, QuadTreeRect& outRect);

    // private data...
    QuadTreeNode* levelNodes_[MAX_TREE_DEPTH];
    Vec3   worldExtents_;
    Vec3   worldScale_;
    Vec3   worldOffset_;
    int    depth_;
    uint32 memorySize_;
};


//---------------------------------------------------------
// inline functions
//---------------------------------------------------------
inline QuadTree::QuadTree() :
    depth_(0),
    memorySize_(0)
{
    memset(levelNodes_, 0, sizeof(levelNodes_));
}

inline QuadTree::~QuadTree()
{
    Shutdown();
}

inline bool QuadTree::IsReady() const
{
    return depth_ && levelNodes_ != nullptr;
}

inline QuadTreeNode* QuadTree::GetNodeFromLevelXZ(const int level, const int x, const int z)
{
    assert(IsReady() == true && "the quad tree has not been created");

    if (level >= 0 && level < depth_)
    {
        return &levelNodes_[level][(z << level) + x];
    }
    return nullptr;
}
