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

    
#if 0
    const XMVECTOR q0 = XMQuaternionRotationAxis({ 0,1,0 }, DEG_TO_RAD(30));
    const XMVECTOR q1 = XMQuaternionRotationAxis({ 1,1,2 }, DEG_TO_RAD(45));
    const XMVECTOR q2 = XMQuaternionRotationAxis({ 0,1,0 }, DEG_TO_RAD(-30));
    const XMVECTOR q3 = XMQuaternionRotationAxis({ 1,0,0 }, DEG_TO_RAD(70));

    cubeAnimation_.keyframes.resize(5);
    cubeAnimation_.keyframes[0].timePos = 0.0f;
    cubeAnimation_.keyframes[0].scale = 0.25f;
    cubeAnimation_.keyframes[0].translation = { -7.0f, 0.0f, 0.0f };
    XMStoreFloat4(&cubeAnimation_.keyframes[0].rotQuat, q0);

    cubeAnimation_.keyframes[1].timePos = 2.0f;
    cubeAnimation_.keyframes[1].scale = 0.5f;
    cubeAnimation_.keyframes[1].translation = { 0.0f, 2.0f, 10.0f };
    XMStoreFloat4(&cubeAnimation_.keyframes[1].rotQuat, q1);

    cubeAnimation_.keyframes[2].timePos = 4.0f;
    cubeAnimation_.keyframes[2].scale = 0.25f;
    cubeAnimation_.keyframes[2].translation = { 7.0f, 0.0f, 0.0f };
    XMStoreFloat4(&cubeAnimation_.keyframes[2].rotQuat, q2);

    cubeAnimation_.keyframes[3].timePos = 6.0f;
    cubeAnimation_.keyframes[3].scale = 0.5f;
    cubeAnimation_.keyframes[3].translation = { 0.0f, 1.0f, -10.0f };
    XMStoreFloat4(&cubeAnimation_.keyframes[3].rotQuat, q3);

    cubeAnimation_.keyframes[4].timePos = 8.0f;
    cubeAnimation_.keyframes[4].scale = 0.25f;
    cubeAnimation_.keyframes[4].translation = { -7.0f, 0.0f, 0.0f };
    XMStoreFloat4(&cubeAnimation_.keyframes[4].rotQuat, q0);
#endif
    return true;
}

//---------------------------------------------------------
//---------------------------------------------------------
void AnimationMgr::Update(const float dt)
{
    // get skeleton and its animation clip
    AnimSkeleton&  skeleton     = GetSkeleton("boblampclean");
    const int      animSearchId = skeleton.GetAnimationIdx("search");
    const int      animTestId   = skeleton.GetAnimationIdx("test");
    AnimationClip& animSearch   = skeleton.GetAnimation(animSearchId);
    AnimationClip& animTest     = skeleton.GetAnimation(animTestId);

    // increase the time position
    animSearch.animTimePos += dt;
    animTest.animTimePos += dt;

    if (animSearch.animTimePos >= animSearch.GetEndTime())
    {
        animSearch.animTimePos = 0.0f;   // loop animation back to beginning
    }

    if (animTest.animTimePos >= animTest.GetEndTime())
    {
        animTest.animTimePos = 0.0f;   // loop animation back to beginning
    }
}

//---------------------------------------------------------
// Desc:  
//---------------------------------------------------------
uint AnimationMgr::AddSkeleton(const char* name)
{
    const uint id = (uint)skeletons_.size();

    skeletons_.push_back(AnimSkeleton());
    skeletons_.back().id_ = id;

    names_.push_back(SkeletonName());
    SetSkeletonNameById(id, name);

    return id;
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

    strncpy(names_[id].name, name, MAX_LEN_SKELETON_NAME);
    strncpy(skeletons_[id].name_, name, MAX_LEN_SKELETON_NAME);
}


}
