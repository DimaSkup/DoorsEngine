// =================================================================================
// Filename:   TerrainLodMgr.cpp
// Created:    20.07.2025 by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include "TerrainLodMgr.h"
#include <Render/d3dclass.h>

namespace Core
{


//---------------------------------------------------------
// Desc:   prepare the lod manager to do its stuff
// Args:   - patchSize:          the length of patch along X and Z-axis (is a square)
//         - numPatchesPerSide:  the number of patches along
//                               X and Z-axis (terrain is a square)
// Ret:    maximal LOD number for such patches
//---------------------------------------------------------
int TerrainLodMgr::Init(const int patchSize, const int numPatchesPerSide)
{
    patchSize_         = patchSize;
    numPatchesPerSide_ = numPatchesPerSide;

    CalcMaxLOD();

    // alloc memory for array of patches lods info
    const int numAllPatches = SQR(numPatchesPerSide);
    map_ = NEW PatchLod[numAllPatches];
    if (!map_)
    {
        LogErr(LOG, "can't allocate memory for arr of patches LOD data");
        exit(0);
    }

    CalcLodRegions(Render::D3DClass::Get()->GetFarZ());

    return maxLOD_;
}

//---------------------------------------------------------
// Desc:   update LOD data for each terrain's patch
// Args:   - cx, cy, cz:  camera's position in 3d space
//         - distFogged:  after this distance all the terrain patches will be
//                        completely fogged (so we set them as low detailed)
//---------------------------------------------------------
void TerrainLodMgr::Update(
    const float cx,
    const float cy,
    const float cz,
    const float distFogged,
    const cvector<int>& visiblePatchesIdxs,
    cvector<int>& highDetailedPatches,
    cvector<int>& midDetailedPatches,
    cvector<int>& lowDetailedPatches)
{
    UpdateLodMapPass1(
        cx, cy, cz,
        distFogged,
        visiblePatchesIdxs,
        highDetailedPatches,
        midDetailedPatches,
        lowDetailedPatches);

    UpdateLodMapPass2(
        cx, cy, cz,
        highDetailedPatches,
        midDetailedPatches,
        lowDetailedPatches);
}

//---------------------------------------------------------
// Desc:   clear memory from the terrain lod manager
//---------------------------------------------------------
void TerrainLodMgr::Release()
{
    SafeDeleteArr(map_);
}

//---------------------------------------------------------
// Desc:   print into a console numerical representation of terrain
//         (number is a LOD of respective terrain's patch)
//---------------------------------------------------------
void TerrainLodMgr::PrintMap() const
{
    const int numPatchesPerSide = numPatchesPerSide_;

    printf("\n\nTerrain LODs map:\n\t");

    // upper idxs
    for (int i = 0; i < numPatchesPerSide; ++i)
        printf("%4d", i);
    printf("\n");

    for (int patchZ = numPatchesPerSide - 1; patchZ >= 0; patchZ--)
    {
        // left idxs
        printf("%d:\t", patchZ);

        // print out a LOD number of each patch in this line by X-axis
        for (int patchX = 0; patchX < numPatchesPerSide; patchX++)
        {
            const int coreLod = GetPatchLodInfo(patchX, patchZ).core;
            printf("%4d", coreLod);
        }

        printf("\n");
    }
    printf("\n\n");
}

//---------------------------------------------------------
// Desc:   pass #1: calculate the core LOD based on the distance
//         from the camera to the center of the patch
// Args:   - cx, cy, cz:      camera's position in 3d space
//         - distFogged:      after this distance all the terrain patches will be
//                            completely fogged (so we set them as low detailed)
//         - visiblePatches:  arr of indices to currently visible patches
//---------------------------------------------------------
void TerrainLodMgr::UpdateLodMapPass1(
    const float cx,
    const float cy,
    const float cz,
    const float distFogged,
    const cvector<int>& visiblePatchesIdxs,
    cvector<int>& highDetailedPatches,
    cvector<int>& midDetailedPatches,
    cvector<int>& lowDetailedPatches)
{
    const int centerStep        = patchSize_ / 2;
    const int numPatchesPerSide = numPatchesPerSide_;
    const int numQuadsInPatch   = patchSize_ - 1;

    assert(numPatchesPerSide == 32);

    const int camPosX = (int)cx;
    const int camPosZ = (int)cz;

    const float sqrDistFogged = SQR(distFogged);

    // number of most/less detailed terrain's patches
    size numHighDetailed = 0;
    size numMidDetailed  = 0;
    size numLowDetailed  = 0;

    // go through each visible patch and update its LOD value
    for (int i = 0; i < (int)visiblePatchesIdxs.size(); ++i)
    {
        const int   patchIdx      = visiblePatchesIdxs[i];
        const int   idxByZ        = patchIdx >> 5;                           // patchIdx / numPatchesPerSide == patchIdx / 32;
        const int   idxByX        = patchIdx & (numPatchesPerSide-1);        // patchIdx % numPatchesPerSide == patchIdx % 32;
        const int   patchCenterX  = (idxByX * numQuadsInPatch) + centerStep;
        const int   patchCenterZ  = (idxByZ * numQuadsInPatch) + centerStep;

        const int sqrDistToCamera = SQR(patchCenterX - camPosX) + SQR(patchCenterZ - camPosZ);
        map_[patchIdx].core       = GetLodByDistanceSqr(sqrDistToCamera);

        if (map_[patchIdx].core == 0)
            highDetailedPatches[numHighDetailed++] = patchIdx;

        else if (sqrDistToCamera >= sqrDistFogged)
            lowDetailedPatches[numLowDetailed++] = patchIdx;

        else
            midDetailedPatches[numMidDetailed++] = patchIdx;
    }

    // resize so we will render proper number of patches for both types
    highDetailedPatches.resize(numHighDetailed);
    midDetailedPatches.resize(numMidDetailed);
    lowDetailedPatches.resize(numLowDetailed);
}

//---------------------------------------------------------
// Desc:   pass #2: match the ring LOD of every patch to the
//         core LOD of its neighbours
// Args:   - cx, cy, cz:      camera's position in 3d world space
//         - visiblePatches:  arr of indices to currently visible patches
//---------------------------------------------------------
void TerrainLodMgr::UpdateLodMapPass2(
    const float cx,
    const float cy,
    const float cz,
    cvector<int>& highDetailedPatches,
    cvector<int>& midDetailedPatches,
    cvector<int>& lowDetailedPatches)
{
    CalcNeighborsLods(highDetailedPatches);
    CalcNeighborsLods(midDetailedPatches);
    CalcNeighborsLods(lowDetailedPatches);
}

//---------------------------------------------------------
// Desc:  compute neighbors' LODs of patch (sector) by idx
//---------------------------------------------------------
void TerrainLodMgr::CalcNeighborsLods(cvector<int>& patchesIdxs)
{
    const int numPatchesPerSide = numPatchesPerSide_;
    assert(numPatchesPerSide == 32);

    // go through each visible patch and update data about its neighbours LODs
    for (index i = 0; i < patchesIdxs.size(); ++i)
    {
        const int patchIdx = patchesIdxs[i];
        PatchLod& patchLod = map_[patchIdx];
        const int coreLod = patchLod.core;

        const int idxByZ = patchIdx >> 5;                             // patchIdx / numPatchesPerSide == patchIdx / 32;
        const int idxByX = patchIdx & (numPatchesPerSide - 1);        // patchIdx % numPatchesPerSide == patchIdx % 32;


        // if we have a left neighbour patch we define if it has a higher LOD
        if (idxByX > 0)
        {
            patchLod.left = (GetPatchLodInfo(idxByX - 1, idxByZ).core > coreLod);
        }

        // if we have a right neighbour patch we define if it has a higher LOD
        if (idxByX < (numPatchesPerSide - 1))
        {
            patchLod.right = (GetPatchLodInfo(idxByX + 1, idxByZ).core > coreLod);
        }

        // if we have a bottom neighbour patch we define if it has a higher LOD
        if (idxByZ > 0)
        {
            patchLod.bottom = (GetPatchLodInfo(idxByX, idxByZ - 1).core > coreLod);
        }

        // if we have a top neighbour patch we define if it has a higher LOD
        if (idxByZ < (numPatchesPerSide - 1))
        {
            patchLod.top = (GetPatchLodInfo(idxByX, idxByZ + 1).core > coreLod);
        }
    }
}

//---------------------------------------------------------
// Desc:   figure out the max level of detail for the patch
//---------------------------------------------------------
void TerrainLodMgr::CalcMaxLOD()
{
    const int numSegments = patchSize_ - 1;

    if (!IsPow2(numSegments))
    {
        LogErr(LOG, "the number of vertices in the patch minus 1 must be a power of two!");
        LogErr(LOG, "your number of vertices-1: %d", numSegments);
        exit(0);
    }

    maxLOD_ = (int)log2f((float)numSegments) - 1;
}

//---------------------------------------------------------
// Desc:  compute ranges for each LOD;
//        here we use such approach:
//        - the range on LOD1 is 2x the range of LOD0
//        - the range of LOD2 is 3x the range of LOD0
//        - etc.
//---------------------------------------------------------
void TerrainLodMgr::CalcLodRegions(const float farZ)
{
    LogMsg(LOG, "compute LODs ranges");
    int temp = 0;
    int sum = 0;

    for (int i = 0; i <= maxLOD_; ++i)
        sum += (i + 1);

    const float x = farZ / (float)sum;

    for (int i = 0; i <= maxLOD_; ++i)
    {
        int curRange = (int)(x * (i+1));
        regions_[i] = temp + curRange;
        temp += curRange;

        printf("LOD%d has range: %d\n", i, regions_[i]);
    }
}

//-----------------------------------------------------
// Desc:   set a distance from the camera where LOD starts
// Args:   - lod:   the number of LOD to change
//         - dist:  new distance to lod
// Ret:    true if everything is ok
//-----------------------------------------------------
bool TerrainLodMgr::SetDistanceToLOD(const int lod, const int dist)
{
    assert(maxLOD_ > 0);

    if ((lod < 0) || (lod > maxLOD_) || (dist <= 0))
        return false;


    // if input LOD is maximal we just check if input distance isn't lower than the prev lod distance
    if ((lod == maxLOD_) && (dist > regions_[lod - 1]))
    {
        regions_[lod] = dist;
    }

    // if input LOD is minimal (equals to 0) we just check if input distance isn't bigger than the next lod distance
    else if ((lod == 0) && (dist < regions_[lod + 1]))
    {
        regions_[lod] = dist;
    }

    // check if input distance is in proper range: (dist lod-1, dist lod+1)
    else
    {
        const int prevLodDist = regions_[lod - 1];
        const int nextLodDist = regions_[lod + 1];

        if ((prevLodDist < dist) && (dist < nextLodDist))
            regions_[lod] = dist;
    }

    return true;
}

} // namespace Core
