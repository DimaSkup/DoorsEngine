/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: AnimationSystem.h
    Desc:     ECS system to handle animations (model skinning) of entities

    Created:  22.12.2025  by DimaSkup
\**********************************************************************************/
#pragma once

#include "../Components/animation.h"

namespace ECS
{

class AnimationSystem
{
public:
    AnimationSystem(Animations* pAnimComponent);

    void Update(const float deltaTime);

    bool HasAnimation(const EntityID id) const;

    const cvector<EntityID>& GetEnttsIds() const;

    bool GetData(
        const EntityID enttId,
        SkeletonID& outSkeletonId,
        AnimationID& outAnimId,
        float& outTimePos) const;

    SkeletonID  GetSkeletonId  (const EntityID id) const;
    AnimationID GetAnimationId (const EntityID id) const;
    float       GetCurrAnimTime(const EntityID id) const;
    float       GetEndAnimTime (const EntityID id) const;


    bool AddRecord(
        const EntityID enttId,
        const SkeletonID skeletonId,
        const AnimationID animId,
        const float animEndTime);

    bool SetAnimation(
        const EntityID enttId,
        const AnimationID animId,
        const float animEndTime,
        const bool isRepeated);

private:
    // for debugging
    void DumpSystem() const;

    const index GetIdx(const EntityID id) const;

private:
    Animations* pAnimComponent_ = nullptr;
};

//---------------------------------------------------------
// Desc:  check if any skeleton and animation is bounded to entity
//---------------------------------------------------------
inline bool AnimationSystem::HasAnimation(const EntityID id) const
{
    return pAnimComponent_->ids.binary_search(id);
}

//---------------------------------------------------------
// Desc:  get arr of entities ids which have an animation component
//---------------------------------------------------------
inline const cvector<EntityID>& AnimationSystem::GetEnttsIds() const
{
    return pAnimComponent_->ids;
}

//---------------------------------------------------------
// Desc:  data getters
//---------------------------------------------------------
inline SkeletonID AnimationSystem::GetSkeletonId(const EntityID id) const
{
    return pAnimComponent_->data[GetIdx(id)].skeletonId;
}

//---------------------------------------------------------

inline AnimationID AnimationSystem::GetAnimationId(const EntityID id) const
{
    return pAnimComponent_->data[GetIdx(id)].currAnimId;
}

//---------------------------------------------------------

inline float AnimationSystem::GetCurrAnimTime(const EntityID id) const
{
    return pAnimComponent_->data[GetIdx(id)].timePos;
}

//---------------------------------------------------------

inline float AnimationSystem::GetEndAnimTime(const EntityID id) const
{
    return pAnimComponent_->data[GetIdx(id)].endTime;
}



} // namespace
