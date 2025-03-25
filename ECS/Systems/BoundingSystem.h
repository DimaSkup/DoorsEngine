// **********************************************************************************
// Filename:      BoundingSystem.h
// Description:   ECS system for handling bounding data of entts
// 
// Created:       26.09.24
// **********************************************************************************
#pragma once

#include "../Components/Bounding.h"

namespace ECS
{

class BoundingSystem final
{
public:
    BoundingSystem(Bounding* pBoundingComponent);
    ~BoundingSystem() {}

    void Update(
        const EntityID* ids,
        const XMMATRIX* transforms,
        const size numEntts,
        const size numMatrices);

    void Add(                                   // add only one entt with only one subset (mesh)
        const EntityID id,
        const BoundingType type,
        const DirectX::BoundingBox& aabb);

    void Add(
        const EntityID* ids,
        const size numEntts,
        const size numSubsets,                  // it is supposed that each input entt has the same number of meshes (for instance: the same trees)
        const BoundingType* types,              // AABB type per mesh
        const DirectX::BoundingBox* AABBs);     // AABB per mesh

    void Add(
        const EntityID* ids,
        const size numEntts,
        const DirectX::BoundingSphere* spheres);

#if 0
    const BoundingData& GetBoundingDataByID(const EntityID id);

    void GetBoundingDataByIDs(
        const std::vector<EntityID>& ids,
        std::vector<BoundingData>& outData);
#endif

    void GetEnttAABB(const EntityID id, DirectX::BoundingBox& outAABB);

    void GetOBBs(
        const EntityID* ids,
        const size numEntts,
        cvector<size>& outNumBoxesPerEntt,
        cvector<DirectX::BoundingOrientedBox>& outOBBs);

    void GetBoxLocalSpaceMatricesByIDs(
        const EntityID* ids,
        const size numEntts,
        cvector<size>& outNumBoxesPerEntt,
        cvector<DirectX::XMMATRIX>& outLocalMatrices);

    void GetBoxesLocalSpaceMatrices(
        const DirectX::BoundingBox* boundingBoxes,
        const size numBoundingBoxes,
        cvector<DirectX::XMMATRIX>& outMatrices);

    void GetBoxLocalSpaceMatrix(
        const DirectX::BoundingBox& aabb,
        DirectX::XMMATRIX& mat);

private:
    inline index GetIdxByID(const EntityID id)
    {
        // return valid idx if there is an entity by such ID;
        // or return -1 if there is no such entity;
        const Bounding& comp = *pBoundingComponent_;
        const index idx = comp.ids.get_idx(id);
        const bool exist = (comp.ids[idx] == id);

        return (exist) ? comp.ids.get_idx(id) : -1;
    }

private:
    Bounding* pBoundingComponent_ = nullptr;
};



} // namespace ECS
