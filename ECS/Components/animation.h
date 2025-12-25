/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: animation.h
    Desc:     ECS component to hold data about animations of entities (model skinning)

    Created:  22.12.2025  by DimaSkup
\**********************************************************************************/
#pragma once
#include <types.h>
#include <cvector.h>

namespace ECS
{

struct AnimData
{
    SkeletonID  skeletonId  = 0;        // id of a skeleton bounded to the entity
    AnimationID currAnimId  = 0;        // id (idx) of current animation
    float       timePos     = 0;        // current animation time
    float       endTime     = 0;        // end time of animation after which we reset timePos to zero
};

struct Animations
{
    Animations()
    {
        // alloc memory ahead
        ids.reserve(64);
        data.reserve(64);

        // push an "invalid" record which will be used when we try to get wrong data
        ids.push_back(INVALID_ENTITY_ID);
        data.push_back(AnimData());
    }

    cvector<EntityID> ids;      // ids will serve us as keys to records
    cvector<AnimData> data;
};

} // namespace
