/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: animation_loader.h

    Desc:     load a skeleton, its bones, and animations
              from a file of engine's internal format

    Created:  29.12.2025  by DimaSkup
\**********************************************************************************/
#pragma once

namespace Core
{
// forward declaration (pointer use only)
class AnimSkeleton;


class AnimationLoader
{
public:
    bool Load(const AnimSkeleton* pSkeleton, const char* filename);
};

} // namespace
