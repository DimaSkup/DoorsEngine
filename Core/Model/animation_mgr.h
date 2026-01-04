/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: animation_mgr.h
    Desc:     

    Created:  02.12.2025  by DimaSkup
\**********************************************************************************/
#pragma once
#include "animation_helper.h"
#include <cvector.h>

namespace Core
{

class AnimationMgr
{
public:
    AnimationMgr();

    bool Init();

    SkeletonID      AddSkeleton         (const char* name);
    SkeletonID      GetSkeletonId       (const char* name);

    AnimSkeleton&   GetSkeleton         (const SkeletonID id);
    AnimSkeleton&   GetSkeleton         (const char* name);

    bool            RemoveSkeleton      (const SkeletonID id);
    void            SetSkeletonNameById (const SkeletonID id, const char* name);

    uint            GetNumSkeletons     (void) const;

    // for debug
    void            DumpSkeletons();

private:
    index           GetIdx(const SkeletonID id) const;

public:
    cvector<SkeletonID>    ids_;
    cvector<SkeletonName>  names_;
    cvector<AnimSkeleton*> skeletons_;
};

//---------------------------------------------------------
// GLOBAL instance of the AnimationMgr
//---------------------------------------------------------
extern AnimationMgr g_AnimationMgr;

} // namespace
