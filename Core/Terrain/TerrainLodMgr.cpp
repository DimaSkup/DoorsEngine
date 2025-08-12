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
//---------------------------------------------------------
void TerrainLodMgr::Update(const float cx, const float cy, const float cz)
{
    UpdateLodMapPass1(cx, cy, cz);
    UpdateLodMapPass2(cx, cy, cz);


    //PrintMap();
    //exit(0);
}

//---------------------------------------------------------
// Desc:   clear memory from the terrain lod manager
//---------------------------------------------------------
void TerrainLodMgr::Release()
{
    SafeDeleteArr(map_);
}

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
// Args:   - cx, cy, cz:  camera's position in 3d space
//---------------------------------------------------------
void TerrainLodMgr::UpdateLodMapPass1(const float cx, const float cy, const float cz)
{
    const int centerStep        = patchSize_ >> 1;
    const int numPatchesPerSide = numPatchesPerSide_;
    const int numQuadsInPatch   = patchSize_ - 1;


    for (int lodMapZ = 0; lodMapZ < numPatchesPerSide; ++lodMapZ)
    {
        const int patchCenterZ = (lodMapZ * numQuadsInPatch) + centerStep;

        for (int lodMapX = 0; lodMapX < numPatchesPerSide; ++lodMapX)
        {
            // calc the patch number and squared distance from camera to the patch center
            const int patchCenterX          = (lodMapX * numQuadsInPatch) + centerStep;
            const float distanceToCamera = sqrtf(SQR(patchCenterX-cx) + SQR(patchCenterZ-cz));
            const int patchNum              = (lodMapZ * numPatchesPerSide) + lodMapX;

            // calc the LOD for center of the current patch
            map_[patchNum].core             = GetLodByDistance(distanceToCamera);
        }
    }
}

//---------------------------------------------------------
// Desc:   pass #2: match the ring LOD of every patch to the
//         core LOD of its neighbours
// Args:   - cx, cy, cz:  camera's position in 3d space
//---------------------------------------------------------
void TerrainLodMgr::UpdateLodMapPass2(const float cx, const float cy, const float cz)
{
    const int step              = patchSize_ >> 1;
    const int numPatchesPerSide = numPatchesPerSide_;

    for (int lodMapZ = 0; lodMapZ < numPatchesPerSide; ++lodMapZ)
    {
        for (int lodMapX = 0; lodMapX < numPatchesPerSide; ++lodMapX)
        {
            const int patchNum = (lodMapZ * numPatchesPerSide) + lodMapX;
            PatchLod& patchLod = map_[patchNum];
            const int coreLod  = patchLod.core;

            // if we have a left neighbour patch we define if it has a higher LOD
            if (lodMapX > 0)
            {
                patchLod.left = (GetPatchLodInfo(lodMapX-1, lodMapZ).core > coreLod);
            }

            // if we have a right neighbour patch we define if it has a higher LOD
            if (lodMapX < (numPatchesPerSide - 1))
            {
                patchLod.right = (GetPatchLodInfo(lodMapX+1, lodMapZ).core > coreLod);
            }

            // if we have a bottom neighbour patch we define if it has a higher LOD
            if (lodMapZ > 0)
            {
                patchLod.bottom = (GetPatchLodInfo(lodMapX, lodMapZ-1).core > coreLod);
            }

            // if we have a top neighbour patch we define if it has a higher LOD
            if (lodMapZ < (numPatchesPerSide - 1))
            {
                patchLod.top = (GetPatchLodInfo(lodMapX, lodMapZ+1).core > coreLod);
            }
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

//---------------------------------------------------------
// Desc:  return a LOD number by input distance
//        (longer distance gives us higher LOD number)
//---------------------------------------------------------
int TerrainLodMgr::GetLodByDistance(float distance)
{
    for (int lod = 0; lod <= maxLOD_; ++lod)
    {
        if (distance < regions_[lod])
            return lod;
    }
    return maxLOD_;
}

} // namespace Core
