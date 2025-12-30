/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: animation_loader.h

    Desc:     load a skeleton, its bones, weights, and animations
              from a file of engine's internal format

    Created:  29.12.2025  by DimaSkup
\**********************************************************************************/
#pragma once

#include <types.h>

namespace Core
{

class AnimationLoader
{
public:
    SkeletonID Load(const char* filename);
};

} // namespace
