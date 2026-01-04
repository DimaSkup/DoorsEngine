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
// Desc:  do some initialization stuff here
//---------------------------------------------------------
bool AnimationMgr::Init()
{
    // add default animation which will be return in cases when
    // we want to get skeleton by wrong id, or something like it
    AddSkeleton("invalid");

    return true;
}

//---------------------------------------------------------
// Desc:  add a new skeleton and set a name for it
//---------------------------------------------------------
SkeletonID AnimationMgr::AddSkeleton(const char* name)
{
    if (StrHelper::IsEmpty(name))
    {
        LogErr(LOG, "empty name");
        return 0;
    }

    // alloc new skeleton
    AnimSkeleton* pSkeleton = NEW AnimSkeleton();
    if (!pSkeleton)
    {
        LogErr(LOG, "can't alloc memory for skeleton: %s", name);
        return 0;
    }

    // init
    SkeletonID id = (SkeletonID)skeletons_.size();

    ids_.push_back(id);
    skeletons_.push_back(pSkeleton);

    names_.push_back(SkeletonName());
    SetSkeletonNameById(id, name);

    pSkeleton->id_ = id;

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
            return skeletons_[i]->id_;
    }

    LogErr(LOG, "there is no skeleton by name: %s", name);
    DumpSkeletons();
    return 0;
}

//---------------------------------------------------------
// Desc:  return a ref to skeleton by its id or name
//---------------------------------------------------------
AnimSkeleton& AnimationMgr::GetSkeleton(const SkeletonID id)
{
    const index idx = GetIdx(id);

    if (idx == 0)
        return *skeletons_[0];

    return *skeletons_[idx];
}

//---------------------------------------------------------

AnimSkeleton& AnimationMgr::GetSkeleton(const char* name)
{
    assert(!StrHelper::IsEmpty(name));

    for (index i = 0; i < names_.size(); ++i)
    {
        if (strcmp(names_[i].name, name) == 0)
            return *skeletons_[i];
    }

    LogErr(LOG, "there is no skeleton by name: %s", name);
    DumpSkeletons();
    return *skeletons_[0];
}

//---------------------------------------------------------
// Desc:  remove a skeleton by input id
//---------------------------------------------------------
bool AnimationMgr::RemoveSkeleton(const SkeletonID id)
{
    if (!ids_.binary_search(id))
    {
        LogErr(LOG, "there is no skeletons by id: %d", (int)id);
        return false;
    }

    const index idx = GetIdx(id);
    if (idx == 0)
        return false;

    ids_.erase(idx);
    names_.erase(idx);

    SafeDelete(skeletons_[idx]);
    skeletons_.erase(idx);

    LogErr(LOG, "skeleton was removed: %d", id);
    return true;
}

//---------------------------------------------------------
// Desc:  the only way supposed to rename a skeleton by id
//        (because we also need to update names arr in the animation manager)
//---------------------------------------------------------
void AnimationMgr::SetSkeletonNameById(const SkeletonID id, const char* name)
{
    if (StrHelper::IsEmpty(name))
    {
        LogErr(LOG, "empty name: lig bollz");
        return;
    }

    const index idx = GetIdx(id);
    if (idx == 0)
        return;

    snprintf(names_[idx].name,       MAX_LEN_SKELETON_NAME, "%s", name);
    snprintf(skeletons_[idx]->name_, MAX_LEN_SKELETON_NAME, "%s", name);
}

//---------------------------------------------------------
// Desc:  return a number of all the currently loaded skeletons
//---------------------------------------------------------
uint AnimationMgr::GetNumSkeletons() const
{
    return (uint)skeletons_.size();
}

//---------------------------------------------------------
// Desc:  get index of skeleton by its id
//---------------------------------------------------------
index AnimationMgr::GetIdx(const SkeletonID id) const
{
    const index idx = ids_.get_idx(id);

    if (idx < 0 || idx >= ids_.size())
    {
        LogErr(LOG, "there is no skeleton by id: %d", (int)id);
        return 0;
    }

    return idx;
}

//---------------------------------------------------------
// for debug
//---------------------------------------------------------
void AnimationMgr::DumpSkeletons()
{
    printf("\n\nDUMP all the skeletons in animation mgr:\n");
    printf(" - num skeletons: %d\n", (int)skeletons_.size());

    for (int i = 0; AnimSkeleton* pSkeleton : skeletons_)
    {
        const char* skeletonName = names_[i].name;
        const int   numAnims     = (int)pSkeleton->animations_.size();

        printf("\tskeleton_%u:  %-36s animations %d\n", i++, skeletonName, numAnims);
    }
    printf("\n");
}

}
