/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: animation_saver.h

    Desc:     save a loaded skeleton, its bones, and animations
              into file of internal format

    Created:  28.12.2025  by DimaSkup
\**********************************************************************************/
#pragma once

namespace Core
{
// forward declaration (pointer use only)
class AnimSkeleton;

class AnimationSaver
{
public:
    bool SaveSkeleton(const AnimSkeleton* pSkeleton, const char* filename);
};

} // namespace
