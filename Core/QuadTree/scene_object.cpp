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
#pragma once
#include "scene_object.h"
#include "quad_tree.h"
#include "quad_tree_node.h"
#include <bit_flags_functions.h>

namespace Core
{

//---------------------------------------------------------
// Desc:  destroys the object. The object is removed from it's parent (if any)
//        and all the children object are orphaned (parent set to NULL)
//---------------------------------------------------------
void SceneObject::Destroy()
{
    // remove ourselved from the quad tree (if any)
    DetachFromQuadTree();
}

//---------------------------------------------------------
void SceneObject::PrepareForUpdate()
{
    // clear the temporary flags
    objectFlags_.ClearFlags(FLAG(NEW_LOCAL_BOUNDS) | FLAG(NEW_WORLD_BOUNDS));
}

//---------------------------------------------------------

void SceneObject::Update()
{
    RecalcWorldBounds();
    RefreshQuadTreeMembership();
}

//---------------------------------------------------------
// Desc:   nodes only get their bounds update as necessary
//---------------------------------------------------------
void SceneObject::RecalcWorldBounds()
{
    if (objectFlags_.TestBit(NEW_LOCAL_BOUNDS) || IsWorldMatrixNew())
    {
        // transform our local rectangle by the current world matrix
        worldBounds_ = localBounds_;

        // make sure we have some degree of thickness
        if (pQuadTree_)
        {
            worldBounds_.x1 = Max(worldBounds_.x1, worldBounds_.x0+0.01f);
            worldBounds_.y1 = Max(worldBounds_.y1, worldBounds_.x0+0.01f);
            worldBounds_.z1 = Max(worldBounds_.z1, worldBounds_.z0+0.01f);
        }

        //worldBounds_.Transform(&GetWorldMatrix())

        // set the flag that our bounding box has changed
        objectFlags_.SetBit(NEW_WORLD_BOUNDS);
    }
}

//---------------------------------------------------------

void SceneObject::RefreshQuadTreeMembership()
{
    if (pQuadTree_ && objectFlags_.TestBit(NEW_WORLD_BOUNDS))
    {
        pQuadTree_->AddOrUpdateSceneObject(this);
    }
}

//---------------------------------------------------------

void SceneObject::AttachToQuadTree(QuadTree* pParentTree)
{
    DetachFromQuadTree();
    pQuadTree_ = pParentTree;
    pQuadTree_->AddOrUpdateSceneObject(this);
}

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


} // namespace
