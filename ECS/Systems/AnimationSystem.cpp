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
#include "../Common/pch.h"
#include "AnimationSystem.h"

namespace ECS
{

//---------------------------------------------------------
// Desc:  constructor
// Args:  pAnimComponent - ptr to animations ECS component 
//---------------------------------------------------------
AnimationSystem::AnimationSystem(Animations* pAnimComponent) :
    pAnimComponent_(pAnimComponent)
{
    if (pAnimComponent == nullptr)
    {
        LogErr(LOG, "input ptr to animation component == nullptr");
        exit(0);
    }
}

//---------------------------------------------------------
// Desc:  update timings of animations
// Args:  dt - delta time
//---------------------------------------------------------
void AnimationSystem::Update(const float dt)
{
    cvector<AnimData>& animations = pAnimComponent_->data;

    for (index i = 1; i < animations.size(); ++i)
    {
        AnimData& anim = animations[i];

        anim.timePos += dt;

        // loop animation back to beginning if necessary
        if ((anim.timePos >= anim.endTime) && anim.isRepeated)
            anim.timePos = 0;
    }
}

//---------------------------------------------------------
// Desc:  get data of animation bounded to entity
// Args:  enttId     - entity identifier
// Out:   skeletonId - output id of current skeleton
//        animId     - output id of current animation
//        timePos    - output current time position of animation (used for interpolation)
//---------------------------------------------------------
bool AnimationSystem::GetData(
    const EntityID enttId,
    SkeletonID& skeletonId,
    AnimationID& animId,
    float& timePos) const
{
    Animations& comp = *pAnimComponent_;
    const index idx  = GetIdx(enttId);

    // check to prevent fuck up
    if (idx == 0)
    {
        DumpSystem();
        return false;
    }

    skeletonId  = comp.data[idx].skeletonId;
    animId      = comp.data[idx].currAnimId;
    timePos     = comp.data[idx].timePos;

    return true;
}

//---------------------------------------------------------
// Desc:  bind a skeleton to entity and set current animation
// Args:  enttId      - entity identifier
//        skeletonId  - id of current animation skeleton
//        animId      - id of current animation
//        animEndTime - duration of current animation
//---------------------------------------------------------
bool AnimationSystem::AddRecord(
    const EntityID enttId,
    const SkeletonID skeletonId,
    const AnimationID animId,
    const float animEndTime)
{
    Animations& comp = *pAnimComponent_;
    const index idx  = comp.ids.get_insert_idx(enttId);

    if (comp.ids[idx] == enttId)
    {
        LogErr(LOG, "there is already a record by id: %" PRIu32, enttId);
        return false;
    }

    AnimData animData;
    animData.skeletonId  = skeletonId;
    animData.currAnimId  = animId;
    animData.timePos     = 0;
    animData.endTime     = animEndTime;
    animData.isRepeated  = true;           // by default we repeat initial animation (it usually is an "idle" animation)

    comp.ids.insert_before(idx, enttId);
    comp.data.insert_before(idx, animData);

    return true;
}

//---------------------------------------------------------
// Desc:  set another animation for entity
// Args:  enttId      - entity identifier
//        animId      - id of current animation
//        animEndTime - duration of current animation
//---------------------------------------------------------
bool AnimationSystem::SetAnimation(
    const EntityID enttId,
    const AnimationID animId,
    const float animEndTime,
    const bool isRepeated)
{
    Animations& comp = *pAnimComponent_;
    const index idx  = GetIdx(enttId);

    // check to prevent fuck up
    if (idx == 0)
    {
        DumpSystem();
        return false;
    }

    if (animEndTime <= 0)
    {
        LogErr(LOG, "end time of animation must be > 0 (passed: %f)", animEndTime);
        return false;
    }

    // switch animation completely only if differ...
    if (comp.data[idx].currAnimId != animId)
    {
        comp.data[idx].currAnimId = animId;
        comp.data[idx].timePos    = 0;
        comp.data[idx].endTime    = animEndTime;
        comp.data[idx].isRepeated = isRepeated;
    }

    // ... we want to set the same animation so just reset it if it already finished
    else
    {
        AnimData& anim = comp.data[idx];

        if (anim.timePos >= anim.endTime)
            anim.timePos = 0;
    }

    return true;
}

//---------------------------------------------------------
// Desc:  (for debug) dump all the records of the system
//---------------------------------------------------------
void AnimationSystem::DumpSystem() const
{
    const Animations& comp = *pAnimComponent_;
    const int numRecords   = (int)comp.ids.size();

    printf("\n\n");
    printf("DUMP ECS AnimationSystem:\n");
    printf(" - num records: %d\n", numRecords);

    for (int i = 0; i < numRecords; ++i)
    {
        printf("\t");
        printf("[%d] entt %-6u skeleton %-5u curr_anim %-5u time %-7.3f end_time %-7.3f\n",
            i,
            comp.ids[i],
            comp.data[i].skeletonId,
            comp.data[i].currAnimId,
            comp.data[i].timePos,
            comp.data[i].endTime);
    }
    printf("\n\n");
}


//---------------------------------------------------------
// Desc:  return an index to data of entity by id or
//        return 0 if there is no data for this entity
//---------------------------------------------------------
const index AnimationSystem::GetIdx(const EntityID id) const
{
    const index idx = pAnimComponent_->ids.get_idx(id);

    if (idx <= 0 || idx >= pAnimComponent_->ids.size())
    {
        LogErr(LOG, "there is no record by id: %" PRIu32, id);
        return 0;
    }

    return idx;
}



} // namespace
