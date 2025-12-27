/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: animation_importer.h
    Desc:     helper class for importing animations using ASSIMP

    Created:  27.12.2025  by DimaSkup
\**********************************************************************************/
#pragma once

#include <assimp/scene.h>

namespace Core
{

class AnimationImporter
{
public:
    void LoadSkeletonAnimations(const aiScene* pScene, const char* skeletonName);

    // for debug
    void PrintNodesHierarchy(const aiScene* pScene, const bool printNodeTransformMatrix);
};

} // namespace
