// =================================================================================
// Filename:   TerrainLodMgr.h
// Desc:       functional for selection of LOD permutation per patch
//             so later we will use it to define proper set of indices inside the
//             index buffer on GPU that we need to render such terrain patch
//
// Responsibilities:
//             1. calculate the maximum LOD:
//                based on the size of the patch (done once on startup)
//
//             2. choose the LOD for each patch (core+ring):
//                based on the location of the camera (on every frame)
//
// Design decision:
//             this implementation only supports patches where the number of
//             segments in the patch is a power of 2
//
//             Max LOD = log2(patch size-1), where patch size is the number of vertices
// 
// Created:    17.07.2025
// =================================================================================
#pragma once
#include <cvector.h>

namespace Core
{

class TerrainLodMgr
{
public:
    // data about LOD of the current patch and its neighbours
    struct PatchLod
    {
        int core   = 0;
        int left   = 0;
        int right  = 0;
        int top    = 0;
        int bottom = 0;
    };

public:
    TerrainLodMgr();
    ~TerrainLodMgr() { Release(); }

    int  Init(const int patchSize, const int numPatchesPerSide);
    void Release();

    void Update(
        const float camPosX,
        const float camPosY,
        const float camPosZ,
        const float distFogged,
        const cvector<int>& visiblePatches,
        cvector<int>& highDetailedPatches,
        cvector<int>& midDetailedPatches,
        cvector<int>& lowDetailedPatches);

    void PrintMap() const;

    const PatchLod& GetPatchLodInfo(const int idx) const;
    const PatchLod& GetPatchLodInfo(const int patchX, const int patchZ) const;

    int GetPatchSize() const;
    int GetDistanceToLOD(const int lod) const;

    bool SetDistanceToLOD(const int lod, const int dist);


private:
    void CalcLodRegions(const float farZ);
    void CalcMaxLOD();

    void UpdateLodMapPass1(
        const float camPosX,
        const float camPosY,
        const float camPosZ,
        const float distFogged,
        const cvector<int>& visiblePatches,
        cvector<int>& highDetailedPatches,
        cvector<int>& midDetailedPatches,
        cvector<int>& lowDetailedPatches);

    void UpdateLodMapPass2(
        const float camPosX,
        const float camPosY,
        const float camPosZ,
        cvector<int>& highDetailedPatches,
        cvector<int>& midDetailedPatches,
        cvector<int>& lowDetailedPatches);

    void CalcNeighborsLods(cvector<int>& patchesIdxs);

    int GetLodByDistance  (float distance)     const;
    int GetLodByDistanceSqr(const int distSqr) const;
    
    
public:
    // number of patches quads along X and Z-axis
    // (for instance: number == 256 + 1 (terrain width) / 16 + 1 (patch size))
    int numPatchesPerSide_;

    // the number of LODs for this terrain's patches
    int maxLOD_;

    // size (number of vertices by X and Z-axis) of a single patch
    int patchSize_;

    PatchLod* map_;

    // distance from the camera where LOD starts
    int regions_[8];
};


//==================================================================================
// INLINE FUNCTIONS
//==================================================================================

//-----------------------------------------------------
// Desc:   get patch's LOD metadata by its idx
//-----------------------------------------------------
inline const TerrainLodMgr::PatchLod& TerrainLodMgr::GetPatchLodInfo(const int idx) const
{
    return map_[idx];
}

//-----------------------------------------------------
// Desc:   get patch's LOD metadata by its idx along X and Z-axis
// Args:   - patchX:  index by X-axis (not position)
//         - patchZ:  index by Z-axis (not position)
//-----------------------------------------------------
inline const TerrainLodMgr::PatchLod& TerrainLodMgr::GetPatchLodInfo(const int patchX, const int patchZ) const
{
    return map_[(patchZ * numPatchesPerSide_) + patchX];
}

//-----------------------------------------------------
// Desc:   get a size of patch along X and Z-axis (patch is a square)
//-----------------------------------------------------
inline int TerrainLodMgr::GetPatchSize() const
{
    return patchSize_;
}

//-----------------------------------------------------
// Desc:   return a distance from the camera where LOD starts
//-----------------------------------------------------
inline int TerrainLodMgr::GetDistanceToLOD(const int lod) const
{
    if ((lod < 0) || (lod > maxLOD_))
        return -1;

    return regions_[lod];
}

//---------------------------------------------------------
  // Desc:  return a LOD number by input distance
  //        (longer distance gives us higher LOD number)
  //---------------------------------------------------------
inline int TerrainLodMgr::GetLodByDistance(float distance) const
{
    for (int lod = 0; lod <= maxLOD_; ++lod)
    {
        if (distance < regions_[lod])
            return lod;
    }
    return maxLOD_;
}

inline int TerrainLodMgr::GetLodByDistanceSqr(const int distSqr) const
{
    for (int lod = 0; lod <= maxLOD_; ++lod)
    {
        if (distSqr < (regions_[lod] * regions_[lod]))
            return lod;
    }
    return maxLOD_;
}

} // namespace
