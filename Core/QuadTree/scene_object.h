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


#include <geometry/rect_3d.h>
#include <bit_flags.h>

namespace Core
{

//---------------------------------------------------------
// forward declarations
//---------------------------------------------------------
class QuadTree;
class QuadTreeNode;


//---------------------------------------------------------
// Class:  SceneObject
//---------------------------------------------------------
class SceneObject
{
public:

    //-----------------------------------------------------
    // data types & constants
    //-----------------------------------------------------
    enum eUpdateDataFlagsBits
    {
        SET_FOR_DESTRUCTION = 0,
        NEW_LOCAL_MATRIX = 1,
        NEW_WORLD_MATRIX = 2,
        REBUILD_INVERSE_WORLD_MATRIX = 3,
        REBUILD_WORLD_VECTORS = 4,
        IGNORE_ORIENTATION = 5,
    };

    enum eObjectFlagBits
    {
        NEW_LOCAL_BOUNDS = 0,
        NEW_WORLD_BOUNDS = 1,
    };

    //-----------------------------------------------------
    // creators
    //-----------------------------------------------------
    SceneObject();
    ~SceneObject();

    //-----------------------------------------------------
    // mutators / setters
    //-----------------------------------------------------
    //bool Create();
    void Destroy();
    void PrepareForUpdate();
    void Update();

    Rect3d& GetLocalBounds();
    void RecalcWorldBounds();

    // QuadTree methods
    void AttachToQuadTree(QuadTree* pParentTree);
    void DetachFromQuadTree();

    //-----------------------------------------------------
    // accessors / getters
    //-----------------------------------------------------
    bool          IsWorldBoundsNew() const;
    bool          IsWorldMatrixNew() const;

    EntityID      GetEntityId()      const;
    u32Flags      GetObjectFlags()   const;
    QuadTreeNode* GetQuadTreeNode()  const;
    u32Flags      GetYMask()         const;

    SceneObject*  GetNextTreeLink()  const;
    SceneObject*  GetPrevTreeLink()  const;

    const Rect3d& GetLocalBounds()   const;
    const Rect3d& GetWorldBounds()   const;
    float*        GetWorldMatrix()   const;


private:
    // private methods
    void SetNextSearchLink(SceneObject* pNext);
    void SetPrevSearchLink(SceneObject* pPrev);
    void RefreshQuadTreeMembership();

    friend QuadTreeNode;
    void SetQuadTreeData(QuadTreeNode* pParentNode, const u32Flags yMask);
    void SetNextTreeLink(SceneObject* pLink);
    void SetPrevTreeLink(SceneObject* pLink);


public:
    EntityID        enttId_ = INVALID_ENTITY_ID;

    // bounding box info
    u32Flags        objectFlags_ = 0;
    u32Flags        updateFlags_ = 0;
    Rect3d          localBounds_;
    Rect3d          worldBounds_;

    // world QuadTree membership info
    u32Flags        quadTreeYMask_  = 0;
    QuadTree*       pQuadTree_      = nullptr;
    QuadTreeNode*   pQuadTreeNode_  = nullptr;
    SceneObject*    pNextTreeLink_  = nullptr;
    SceneObject*    pPrevTreeLink_  = nullptr;
};


//==================================================================================
// Inline Functions 
//==================================================================================

//==================================================================================
// default constructor and destructor
//==================================================================================
inline SceneObject::SceneObject()
{
}

inline SceneObject::~SceneObject()
{
    Destroy();
}

//==================================================================================
// getters
//==================================================================================

inline EntityID SceneObject::GetEntityId() const
{
    return enttId_;
}

//-----------------------------------------------------

inline u32Flags SceneObject::GetObjectFlags() const
{
    return objectFlags_;
}

//-----------------------------------------------------

inline Rect3d& SceneObject::GetLocalBounds()
{
    objectFlags_.SetBit(NEW_LOCAL_BOUNDS);
    return localBounds_;
}

//-----------------------------------------------------

inline QuadTreeNode* SceneObject::GetQuadTreeNode() const
{
    return pQuadTreeNode_;
}

//-----------------------------------------------------

inline u32Flags SceneObject::GetYMask() const
{
    return quadTreeYMask_;
}

//-----------------------------------------------------

inline SceneObject* SceneObject::GetNextTreeLink() const
{
    return pNextTreeLink_;
}

//-----------------------------------------------------

inline SceneObject* SceneObject::GetPrevTreeLink() const
{
    return pPrevTreeLink_;
}


//==================================================================================
// setters
//==================================================================================

inline void SceneObject::SetQuadTreeData(QuadTreeNode* pParentNode, const u32Flags yMask)
{
    pQuadTreeNode_ = pParentNode;
    quadTreeYMask_ = yMask;
}

//-----------------------------------------------------

inline void SceneObject::SetNextTreeLink(SceneObject* pLink)
{
    pNextTreeLink_ = pLink;
}

inline void SceneObject::SetPrevTreeLink(SceneObject* pLink)
{
    pPrevTreeLink_ = pLink;
}

//-----------------------------------------------------

inline const Rect3d& SceneObject::GetLocalBounds() const
{
    return localBounds_;
}

inline const Rect3d& SceneObject::GetWorldBounds() const
{
    return worldBounds_;
}

//-----------------------------------------------------

inline float* SceneObject::GetWorldMatrix() const
{
    return nullptr;
}

//-----------------------------------------------------

inline bool SceneObject::IsWorldBoundsNew() const
{
    return TEST_BIT(objectFlags_.value_, NEW_WORLD_BOUNDS);
}

inline bool SceneObject::IsWorldMatrixNew() const
{
    return updateFlags_.TestBit(NEW_WORLD_MATRIX);
}

} // namespace
