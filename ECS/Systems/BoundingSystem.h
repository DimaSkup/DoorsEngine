/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: BoundingSystem
    Desc:     ECS system to handle bounding shapes of entities

    Created:       26.09.2024
    Revisited:     30.11.2025 (God damn, I simplified this shit alot)
\**********************************************************************************/
#pragma once

#include "../Common/ECSTypes.h"
#include "../Components/Bounding.h"
#include "../Systems/TransformSystem.h"
#include <geometry/rect3d_functions.h>
#include <geometry/sphere_functions.h>

namespace ECS
{

class BoundingSystem
{
public:
    BoundingSystem(Bounding* pBoundingComponent, TransformSystem* pTransSys);
    ~BoundingSystem() {}

    void TranslateWorldBoundings(const EntityID id);
    void TranslateWorldBoundings(const EntityID* ids, const size count);

    void UpdateWorldBoundings(const EntityID id);
    void UpdateWorldBoundings(const EntityID* ids, const size count);

    // add one record
    bool Add(
        const EntityID id,
        const DirectX::BoundingSphere& localSphere,
        const DirectX::BoundingSphere& worldSphere);

    bool Add(
        const EntityID id,
        const DirectX::BoundingBox& localBox,
        const DirectX::BoundingBox& worldBox);

    // add multiple records
    bool Add(
        const EntityID* ids,
        const size numEntts,
        const DirectX::BoundingSphere& localSphere,
        const DirectX::BoundingSphere* worldSpheres);

    bool Add(
        const EntityID* ids,
        const size numEntts,
        const DirectX::BoundingBox& localBox,
        const DirectX::BoundingBox* worldBoxes);

    const BoundData& GetBoundingData(const EntityID id) const;

    void GetBoundSpheres(
        const EntityID* ids,
        const size numEntts,
        cvector<DirectX::BoundingSphere>& outSpheres);

    const DirectX::BoundingBox&    GetLocalBoundBox   (const EntityID id) const;
    const DirectX::BoundingBox&    GetWorldBoundBox   (const EntityID id) const;
    const DirectX::BoundingSphere& GetWorldBoundSphere(const EntityID id) const;

    Rect3d                         GetLocalBoxRect3d  (const EntityID id) const;
    Rect3d                         GetWorldBoxRect3d  (const EntityID id) const;
    Sphere                         GetWorldSphere     (const EntityID id) const;

    void PrintDump() const;

private:
    // private functions...
    index GetIdx(const EntityID id) const;

    // private data...
    Bounding*        pBoundingComponent_;
    TransformSystem* pTransSys_;
};


//---------------------------------------------------------
// inline functions
//---------------------------------------------------------

//-----------------------------------------------------
// Desc:  return idx if there is a record by such ID;
//        or return 0 (it is an index to "invalid" bounding shape)
//-----------------------------------------------------
inline index BoundingSystem::GetIdx(const EntityID id) const
{
    const index idx = pBoundingComponent_->ids.get_idx(id);

    if (idx > 0 && idx < pBoundingComponent_->ids.size())
        return idx;

    return 0;
}

//--------------------------------------------------------
// Desc:  get bounding data of entity by id
//--------------------------------------------------------
inline const BoundData& BoundingSystem::GetBoundingData(const EntityID id) const
{
    return pBoundingComponent_->data[GetIdx(id)];
}

//--------------------------------------------------------
// Desc:  get an axis-aligned bounding box around entity by input ID
//        1. in local space
//        2. in world space
//--------------------------------------------------------
inline const DirectX::BoundingBox& BoundingSystem::GetLocalBoundBox(const EntityID id) const
{
    return pBoundingComponent_->data[GetIdx(id)].localBox;
}

inline const DirectX::BoundingBox& BoundingSystem::GetWorldBoundBox(const EntityID id) const
{
    return pBoundingComponent_->data[GetIdx(id)].worldBox;
}

//--------------------------------------------------------
// Desc:  get a bounding sphere (in world space) around entity by input ID
//--------------------------------------------------------
inline const DirectX::BoundingSphere& BoundingSystem::GetWorldBoundSphere(const EntityID id) const
{
    return pBoundingComponent_->data[GetIdx(id)].worldSphere;
}

//--------------------------------------------------------
// Desc:  get an axis-aligned bounding box around entity by input ID (as a Rect3d)
//        1. in local space
//        2. in world space
//--------------------------------------------------------
inline Rect3d BoundingSystem::GetLocalBoxRect3d(const EntityID id) const
{
    const DirectX::BoundingBox& box = GetLocalBoundBox(id);
    const DirectX::XMFLOAT3&      c = box.Center;
    const DirectX::XMFLOAT3&      e = box.Extents;

    return Rect3d({c.x, c.y, c.z}, {e.x, e.y, e.z});
}

inline Rect3d BoundingSystem::GetWorldBoxRect3d(const EntityID id) const
{
    const DirectX::BoundingBox& box = GetWorldBoundBox(id);
    const DirectX::XMFLOAT3&      c = box.Center;
    const DirectX::XMFLOAT3&      e = box.Extents;

    return Rect3d({c.x, c.y, c.z}, {e.x, e.y, e.z});
}

//--------------------------------------------------------
// Desc:  get a bounding sphere (in world space) around entity by input ID
//--------------------------------------------------------
inline Sphere BoundingSystem::GetWorldSphere(const EntityID id) const
{
    const DirectX::BoundingSphere& s = GetWorldBoundSphere(id);
    return Sphere{ s.Center.x, s.Center.y, s.Center.z, s.Radius };
}

} // namespace ECS
