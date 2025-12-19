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
    void Update(const float dt);

    uint AddSkeleton(const char* name);

    AnimSkeleton& GetSkeleton(const uint id);
    AnimSkeleton& GetSkeleton(const char* name);

    void SetSkeletonNameById(const uint id, const char* name);

public:
    cvector<AnimSkeleton> skeletons_;
    cvector<SkeletonName> names_;
};

//---------------------------------------------------------
// GLOBAL instance of the AnimationMgr
//---------------------------------------------------------
extern AnimationMgr g_AnimationMgr;

} // namespace
