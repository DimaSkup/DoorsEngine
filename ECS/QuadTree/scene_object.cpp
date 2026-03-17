/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: scene_object.cpp
    Desc:     implementation of scene object (member of quad tree node)

              This code I mainly rewrote from sources to the amazing book
              "Real Time 3D Terrain Engines Using C++ and DirectX"
              by Greg Snook

    Created:  16.09.2025 by DimaSkup
\**********************************************************************************/
#include "../Common/pch.h"
#include "scene_object.h"
#include "quad_tree_node.h"
#include "quad_tree.h"
#include <assert.h>


//---------------------------------------------------------
// Desc:  setup this object and attach it to quad tree
//---------------------------------------------------------
bool SceneObject::Init(
    QuadTree* pQuadTree,
    const EntityID id,
    const Sphere& worldSphere,
    const Rect3d& worldBox)
{
    assert(pQuadTree);
    assert(id > 0);

    id_ = id;
    worldBoundSphere_ = worldSphere;
    worldBoundBox_    = worldBox;

    AttachToQuadTree(pQuadTree);

    return true;
}

//---------------------------------------------------------
// Desc:  destroy the object and remove it from it's parent (if any) quad tree
//---------------------------------------------------------
void SceneObject::Shutdown()
{
    DetachFromQuadTree();
}

//---------------------------------------------------------
// Desc:  update bounding info of the object
//        and its location withing the quad tree
//---------------------------------------------------------
void SceneObject::UpdateWorldBounds(const Sphere& worldSphere, const Rect3d& worldBox)
{
    worldBoundSphere_ = worldSphere;
    worldBoundBox_    = worldBox;
    pQuadTree_->AddOrUpdateSceneObject(this);
}

//---------------------------------------------------------
// Desc:  update location of the object within its quad tree
//---------------------------------------------------------
void SceneObject::RefreshQuadTreeMembership()
{
    if (pQuadTree_)
        pQuadTree_->AddOrUpdateSceneObject(this);
}

//---------------------------------------------------------
// Desc:  bind this object to input quad tree
//---------------------------------------------------------
void SceneObject::AttachToQuadTree(QuadTree* pParentTree)
{
    DetachFromQuadTree();

    pQuadTree_ = pParentTree;
    pQuadTree_->AddOrUpdateSceneObject(this);
}

//---------------------------------------------------------
// Desc:  detach this object from quad tree (if have any)
//---------------------------------------------------------
void SceneObject::DetachFromQuadTree()
{
    if (pQuadTreeNode_)
    {
        pQuadTreeNode_->RemoveMember(this);
        pQuadTreeNode_ = nullptr;
    }

    pQuadTree_     = nullptr;
    pNextTreeLink_ = nullptr;
    pPrevTreeLink_ = nullptr;
}

//---------------------------------------------------------
// Desc:  attach this object to the result list of the
//        quad tree's last search
//---------------------------------------------------------
void SceneObject::AttachToSearchResult(SceneObject* pPrevLink, SceneObject* pNextLink)
{
    pNextSearchLink_ = pNextLink;
    pPrevSearchLink_ = pPrevLink;

    if (pNextSearchLink_)
        pNextSearchLink_->SetPrevSearchLink(this);

    if (pPrevSearchLink_)
        pPrevSearchLink_->SetNextSearchLink(this);
}

//---------------------------------------------------------
// Desc:  remove this object from the quad tree's search results list
//---------------------------------------------------------
void SceneObject::DetachFromSearchResult()
{
    if (pNextSearchLink_)
        pNextSearchLink_->SetPrevSearchLink(pPrevSearchLink_);

    if (pPrevSearchLink_)
        pPrevSearchLink_->SetNextSearchLink(pNextSearchLink_);

    pNextSearchLink_ = nullptr;
    pPrevSearchLink_ = nullptr;
}

//---------------------------------------------------------
// Desc:  just reset pointers related to quad tree's search results
//---------------------------------------------------------
void SceneObject::ClearSearchResults()
{
    pNextSearchLink_ = nullptr;
    pPrevSearchLink_ = nullptr;
}
