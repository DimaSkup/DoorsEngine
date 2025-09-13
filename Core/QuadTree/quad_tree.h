#pragma once
/***************************************************************\

    ******    ********  ********  ******    ********
    **    **  **    **  **    **  **    **  **    ** 
    **    **  **    **  **    **  **    **  **            
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******    ********  ********  **    **  ********

    Filename: quad_tree.h
    Desc:     quad-tree class declaration
    Created:  13.09.2025 by DimaSkup
\***************************************************************/
#include <Types.h>
#include <cvector.h>
#include <math/vec3.h>

namespace Core
{

class QuadTree
{
public:

    enum eConstants
    {
        MIN_QTREE_DEPTH = 1,
        MAX_QTREE_DEPTH = 9,      // must be a value between 1 and 9
    };

    QuadTree();
    ~QuadTree();

    void Create(
        const float worldExtentsX,
        const float worldExtentsY,
        const float worldExtentsZ, const int quadTreeDepth);
    void Destroy();

    cvector<EntityID>& GetVisibleEntities(
        const Rect3d& worldRect,
        const Frustum& frustum,
        const cvector<EntityID>& entities);

    u32Flags AddOrUpdateEntity(const EntityID id);

    bool IsQuadTreeReady() const;

private:

    // private data...
    QuadTreeNode* levelNodes_[MAX_QTREE_DEPTH];
    Vec3          worldExtents_ = {0,0,0};
    int           qTreeDepth_   = 0;
    uint32        memSize_      = 0;

    // private methods...
    void FindTreeNodeInfo(const QuadTreeRect& worldByteRect, int& level, int& levelX, int& levelZ);
    QuadTreeNode* FindTreeNode(const QuadTreeRect& worldByteRect);
    QuadTreeNode* GetNodeFromLevelXZ(const int level, const int x, const int z);
    void BuildByteRect(const Rect3d& worldRect, QuadTreeRect& worldByteRect);
};


//= Inline Methods ============================================

//---------------------------------------------------------
// Desc:   default constructor and destructor
//---------------------------------------------------------
inline QuadTree::QuadTree() :
    qTreeDepth_(0),
    memSize_(0)
{
    memset(levelNodes_, 0, sizeof(levelNodes_));
}
}
