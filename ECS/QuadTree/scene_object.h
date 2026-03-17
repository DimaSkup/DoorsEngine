/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: scene_object.h
    Desc:     implementation of scene object (member of quad tree node)

              This code I mainly rewrote from sources to the amazing book
              "Real Time 3D Terrain Engines Using C++ and DirectX"
              by Greg Snook

    Created:  15.09.2025 by DimaSkup
\**********************************************************************************/
#pragma once

#include <geometry/rect3d.h>
#include <geometry/sphere.h>
#include <bit_flags.h>


//---------------------------------------------------------
// forward declarations (pointer use only)
//---------------------------------------------------------
class QuadTree;
class QuadTreeNode;


//---------------------------------------------------------
// class name:  SceneObject
//---------------------------------------------------------
class SceneObject
{
public:

    // creators...
    SceneObject();
    ~SceneObject();

    bool Init(
        QuadTree* pQuadTree,
        const EntityID id,
        const Sphere& worldSphere,
        const Rect3d& worldBox);

    void Shutdown();

    // mutators...
    void          UpdateWorldBounds(const Sphere& worldSphere, const Rect3d& worldBox);

    // QuadTree functions...
    void          AttachToQuadTree(QuadTree* pParentTree);
    void          DetachFromQuadTree();

    // search functions...
    void          AttachToSearchResult(SceneObject* pPrevLink, SceneObject* pNextLink);
    void          DetachFromSearchResult();
    void          ClearSearchResults();
    SceneObject*  GetNextSearchLink();

    // accessors...
    EntityID      GetId()           const;
    u32Flags      GetObjectFlags()  const;

    QuadTreeNode* GetQuadTreeNode() const;
    u32Flags      GetYMask()        const;
    SceneObject*  GetNextTreeLink() const;
    SceneObject*  GetPrevTreeLink() const;

    const Sphere& GetWorldSphere()  const;
    const Rect3d& GetWorldBox()     const;

private:

    // private functions...
    void SetNextSearchLink(SceneObject* pNextLink);
    void SetPrevSearchLink(SceneObject* pPrevLink);

    friend QuadTreeNode;
    void SetQuadTreeData(QuadTreeNode* pParentNode, u32Flags yMask);
    void SetNextTreeLink(SceneObject* pLink);
    void SetPrevTreeLink(SceneObject* pLink);
    void RefreshQuadTreeMembership();

private:

    EntityID      id_;

    // QuadTree search result links
    SceneObject*  pNextSearchLink_;
    SceneObject*  pPrevSearchLink_;

    // world QuadTree membership info
    QuadTree*     pQuadTree_;
    QuadTreeNode* pQuadTreeNode_;
    SceneObject*  pNextTreeLink_;
    SceneObject*  pPrevTreeLink_;
    u32Flags      quadTreeYMask_;

    // bounding box info
    u32Flags      objectFlags_;
    Sphere        worldBoundSphere_;
    Rect3d        worldBoundBox_;
};

//---------------------------------------------------------
// inline functions
//---------------------------------------------------------
inline SceneObject::SceneObject() :
    id_(0),
    pNextSearchLink_(nullptr),
    pPrevSearchLink_(nullptr),
    pQuadTree_(nullptr),
    pQuadTreeNode_(nullptr),
    pNextTreeLink_(nullptr),
    pPrevTreeLink_(nullptr),
    quadTreeYMask_(0),
    objectFlags_(0),
    worldBoundSphere_(0,0,0,1),
    worldBoundBox_(0,0, 0,0, 0,0)
{
}

inline SceneObject::~SceneObject()
{
}

//---------------------------------------------------------
// accessors
//---------------------------------------------------------
inline EntityID SceneObject::GetId() const
{
    return id_;
}

inline u32Flags SceneObject::GetObjectFlags() const
{
    return objectFlags_;
}

inline QuadTreeNode* SceneObject::GetQuadTreeNode() const
{
    return pQuadTreeNode_;
}

inline u32Flags SceneObject::GetYMask() const
{
    return quadTreeYMask_;
}

inline SceneObject* SceneObject::GetNextTreeLink() const
{
    return pNextTreeLink_;
}

inline SceneObject* SceneObject::GetPrevTreeLink() const
{
    return pPrevTreeLink_;
}

inline SceneObject* SceneObject::GetNextSearchLink()
{
    return pNextSearchLink_;
}

inline const Sphere& SceneObject::GetWorldSphere() const
{
    return worldBoundSphere_;
}

inline const Rect3d& SceneObject::GetWorldBox() const
{
    return worldBoundBox_;
}

//---------------------------------------------------------
// private functions
//---------------------------------------------------------
inline void SceneObject::SetNextSearchLink(SceneObject* pNextLink)
{
    pNextSearchLink_ = pNextLink;
}

inline void SceneObject::SetPrevSearchLink(SceneObject* pPrevLink)
{
    pPrevSearchLink_ = pPrevLink;
}

inline void SceneObject::SetQuadTreeData(QuadTreeNode* pParentNode, u32Flags yMask)
{
    pQuadTreeNode_ = pParentNode;
    quadTreeYMask_ = yMask;
}

inline void SceneObject::SetNextTreeLink(SceneObject* pLink)
{
    pNextTreeLink_ = pLink;
}

inline void SceneObject::SetPrevTreeLink(SceneObject* pLink)
{
    pPrevTreeLink_ = pLink;
}
