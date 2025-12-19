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
#include "scene_object.h"
#include <Types.h>
#include <math/vec3.h>
#include <geometry/rect_3d.h>
#include <bit_flags.h>

namespace Core
{

//----------------------------------------------------------------------------------
// CONSTANTS
//----------------------------------------------------------------------------------
constexpr int MAX_NUM_SCENE_OBJECTS = 1024;


//----------------------------------------------------------------------------------
// Class: QuadTree
//----------------------------------------------------------------------------------
class QuadTree
{
public:

    enum eConstants
    {
        MIN_QTREE_DEPTH = 1,
        MAX_QTREE_DEPTH = 9,      // must be a value between 1 and 9
    };

    //-----------------------------------------------------

    QuadTree();
    ~QuadTree();

    void Create(const Rect3d& worldBoundingBox, const int quadTreeDepth);
    void Destroy();

    void AddSceneObject(const EntityID enttId, const Rect3d& objRect);
    void UpdateSceneObject(const EntityID enttId);

    void CalcVisibleEntities(
        const Rect3d& worldRect,
        const Frustum& frustum);

    const EntityID* GetVisibleEnttsArr() const { return visibleEntts_; }
    const int       GetNumVisibleEntts() const { return currNumVisibleEntts_; }



    u32Flags AddOrUpdateSceneObject(SceneObject* pObj);

    bool IsReady() const;

private:
    EntityID    visibleEntts_    [MAX_NUM_SCENE_OBJECTS];
    SceneObject sceneObjectsPool_[MAX_NUM_SCENE_OBJECTS];

    int         currNumSceneObjects_ = 0;
    int         currNumVisibleEntts_ = 0;

    // private data...
    QuadTreeNode* levelNodes_[MAX_QTREE_DEPTH];
    Vec3          worldExtents_ = {0,0,0};
    Vec3          worldScale_   = {0,0,0};
    Vec3          worldOffset_  = {0,0,0};
    int           qTreeDepth_   = 0;
    uint32        memSize_      = 0;

    // private methods...
    void          FindTreeNodeInfo(const QuadTreeRect& worldByteRect, int& level, int& levelX, int& levelZ);
    QuadTreeNode* FindTreeNode(const QuadTreeRect& worldByteRect);
    QuadTreeNode* GetNodeFromLevelXZ(const int level, const int x, const int z);
    void          BuildByteRect(const Rect3d& worldRect, QuadTreeRect& worldByteRect);
};

//---------------------------------------------------------
// global instance of the quad tree
//---------------------------------------------------------
extern QuadTree g_QuadTree;


//---------------------------------------------------------
// Desc:   default constructor and desturctor
//---------------------------------------------------------
inline QuadTree::QuadTree() : qTreeDepth_(0), memSize_(0)
{
    memset(levelNodes_, 0, sizeof(levelNodes_));
}

inline QuadTree::~QuadTree()
{
}

//---------------------------------------------------------
// inline accessors / getters
//---------------------------------------------------------
inline bool QuadTree::IsReady() const
{
    return qTreeDepth_ && (levelNodes_ != nullptr);
}

//---------------------------------------------------------
// Desc:   get a ptr to quad tree node by input tree level,
//         and indices of node along X,Z-axis
//---------------------------------------------------------
inline QuadTreeNode* QuadTree::GetNodeFromLevelXZ(
    const int level,
    const int xIdx,
    const int zIdx)
{
    assert(IsReady() && "the quad tree has not been created");

    if (level >= 0 && level < qTreeDepth_)
    {
        return &levelNodes_[level][(zIdx<<level)+xIdx];
    }

    return nullptr;
}

} // namespace 
