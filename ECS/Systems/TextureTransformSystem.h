// ********************************************************************************
// Filename:      TextureTransformSystem.h
// 
// Description:   Entity-Component-System (ECS) system for using 
//                of 3 types of texture transformations:
//                1. STATIC textures transformation (for instance: just scale texture);
//                2. DYNAMIC textures transformation (texture animation; for instance: to animate a fireflame)
//                3. rotation around particular texture coordinate (for instance: around its center to animate fireball)
// 
// Created:       29.06.24
// ********************************************************************************
#pragma once

#include "../Components/TextureTransform.h"
#include <cvector.h>

namespace ECS
{

class TextureTransformSystem
{
public:
    TextureTransformSystem(TextureTransform* pTexTransformComp);
    ~TextureTransformSystem() {}


    void AddTexTransformation(
        const EntityID* ids,
        const size numEntts,
        const TexTransformType type,
        const TexTransformInitParams& params);

    void GetTexTransformsForEntts(
        const EntityID* ids,
        const size numEntts,
        cvector<XMMATRIX>& outTexTransforms);

    void UpdateAllTextrureAnimations(const float totalGameTime, const float deltaTime);

private:
    inline bool CheckCanAddRecords(const EntityID* ids, const size numEntts) const { return !pTexTransformComponent_->ids.binary_search(ids, numEntts); }

    void AddStaticTexTransform(
        const EntityID* ids,
        const size numEntts,
        const TexTransformInitParams& inParams);

    void AddAtlasTextureAnimation(
        const EntityID* ids,
        const size numEntts,
        const TexTransformInitParams& inParams);

    void AddRotationAroundTexCoord(
        const EntityID* ids,
        const size numEntts,
        const TexTransformInitParams& inParams);

    const index AddAtlasAnimationData(
        const EntityID enttID,
        const TexAtlasAnimationData& data);

    void UpdateTextureStaticTransformation(const float deltaTime);
    void UpdateTextureAtlasAnimations(const float deltaTime);
    void UpdateTextureRotations(const float totalGameTime);

    void ApplyTexTransformsByIdxs(
        const cvector<index>& idxs,
        const cvector<XMMATRIX>& texTransforms);

    TextureTransform* pTexTransformComponent_ = nullptr;
};

};
