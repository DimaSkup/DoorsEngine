/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: ui_entt_animation_controller.h
    Desc:     holds entity animations data
              (a part of the MVC pattern)

    Created:  24.12.2025  by DimaSkup
\**********************************************************************************/
#pragma once

#include <types.h>
#include <cvector.h>

namespace UI
{

class EnttAnimationData
{
public:
    SkeletonID              skeletonId = 0;         // current skeleton 
    SkeletonName            skeletonName;           
    cvector<AnimationName>* animNames;              // array of animations names related to the current skeleton

    AnimationID             animationId  = 0;       // current animation id
    AnimationName           currAnimName;           // name of current animation
    float                   currAnimTime = 0;       // current animation time (which is used in the engine for interpolated btw keyframes)
    float                   endAnimTime  = 0;       // end time of animation (after which we restart the animation)
};

} // namespace

