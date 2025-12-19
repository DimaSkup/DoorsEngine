// **********************************************************************************
// Filename:      BoundingSystem.h
// Description:   ECS system for handling bounding data of entts
// 
// Created:       26.09.2024
// Revisited:     30.11.2025 (God damn, I simplified this shit alot)
// **********************************************************************************
#pragma once

#include "../Common/ECSTypes.h"
#include "../Components/Bounding.h"

namespace ECS
{

class BoundingSystem
{
public:
    BoundingSystem(Bounding* pBoundingComponent);
    ~BoundingSystem() {}


    // add one record
    bool Add(const EntityID id, const DirectX::BoundingSphere& sphere);
    bool Add(const EntityID id, const DirectX::BoundingBox& aabb);

    // add multiple records
    bool Add(
        const EntityID* ids,
        const size numEntts,
        const DirectX::BoundingBox& aabb);

    // ----------------------------------------------------

    void GetBoundSpheres(
        const EntityID* ids,
        const size numEntts,
        cvector<DirectX::BoundingSphere>& outBoundSpheres);

    const DirectX::BoundingBox&    GetAABB(const EntityID id)        const;
    const DirectX::BoundingSphere& GetBoundSphere(const EntityID id) const;

private:

    //-----------------------------------------------------
    // Desc:  return idx if there is a record by such ID;
    //        or return 0 (it is an index to "invalid" bounding shape)
    //-----------------------------------------------------
    inline index GetIdxByID(const EntityID id) const
    {
        const Bounding& comp = *pBoundingComponent_;
        const index     idx  = comp.ids.get_idx(id);

        if (idx > 0 || idx < comp.ids.size())
            return idx;

        return 0;
    }

private:
    Bounding* pBoundingComponent_ = nullptr;
};



} // namespace ECS
