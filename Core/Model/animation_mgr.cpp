/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: animation_mgr.cpp
    Desc:

    Created:  02.12.2025  by DimaSkup
\**********************************************************************************/
#include <CoreCommon/pch.h>
#include "animation_mgr.h"
#pragma warning (disable:4996)


namespace Core
{

using namespace DirectX;

//---------------------------------------------------------
// GLOBAL instance of the AnimationMgr
//---------------------------------------------------------
AnimationMgr g_AnimationMgr;


//---------------------------------------------------------
// constructor
//---------------------------------------------------------
AnimationMgr::AnimationMgr()
{
}

//---------------------------------------------------------
//---------------------------------------------------------
bool AnimationMgr::Init()
{
    // add default animation which will be return in cases when
    // we want to get skeleton by wrong id, or something like it
    AddSkeleton("invalid");

    return true;
}

//---------------------------------------------------------
//---------------------------------------------------------
void AnimationMgr::Update(const float dt, const char* skeletonName, const char* animName)
{
    // get skeleton and its animation clip
    AnimSkeleton& skeleton = GetSkeleton(skeletonName);
    const int animIdx      = skeleton.GetAnimationIdx(animName);
    AnimationClip& anim    = skeleton.GetAnimation(animIdx);

    // increase the time position
    anim.animTimePos += dt;

    if (anim.animTimePos >= anim.GetEndTime())
    {
        anim.animTimePos = 0.0f;   // loop animation back to beginning
    }
}

//---------------------------------------------------------
// Desc:  add a new skeleton and set a name for it
//---------------------------------------------------------
SkeletonID AnimationMgr::AddSkeleton(const char* name)
{
    const SkeletonID id = (uint)skeletons_.size();

    skeletons_.push_back(AnimSkeleton());
    skeletons_.back().id_ = id;

    names_.push_back(SkeletonName());
    SetSkeletonNameById(id, name);

    return id;
}

//---------------------------------------------------------
// Desc:  return an ID (idx) of skeleton by input name
//---------------------------------------------------------
SkeletonID AnimationMgr::GetSkeletonId(const char* name)
{
    if (StrHelper::IsEmpty(name))
    {
        LogErr(LOG, "input name is empty");
        return 0;
    }

    for (index i = 0; i < names_.size(); ++i)
    {
        if (strcmp(names_[i].name, name) == 0)
            return skeletons_[i].id_;
    }

    LogErr(LOG, "there is no skeleton by name: %s", name);
    return 0;
}

//---------------------------------------------------------
// Desc:  return a ref to skeleton by its id or name
//---------------------------------------------------------
AnimSkeleton& AnimationMgr::GetSkeleton(const uint id)
{
    assert(id < skeletons_.size());
    return skeletons_[id];
}

AnimSkeleton& AnimationMgr::GetSkeleton(const char* name)
{
    assert(!StrHelper::IsEmpty(name));

    for (index i = 0; i < names_.size(); ++i)
    {
        if (strcmp(names_[i].name, name) == 0)
            return skeletons_[i];
    }

    LogErr(LOG, "there is no skeleton by name: %s", name);
    return skeletons_[0];
}

//---------------------------------------------------------
// Desc:  the only way supposed to rename a skeleton by id
//        (because we also need to update names arr in the animation manager)
//---------------------------------------------------------
void AnimationMgr::SetSkeletonNameById(const uint id, const char* name)
{
    assert(id < names_.size());
    assert(name && name[0] != '\0' && "lig bollz");

    snprintf(names_[id].name,      MAX_LEN_SKELETON_NAME, "%s", name);
    snprintf(skeletons_[id].name_, MAX_LEN_SKELETON_NAME, "%s", name);
}

}
