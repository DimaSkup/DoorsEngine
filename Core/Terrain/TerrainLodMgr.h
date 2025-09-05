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
    ~TerrainLodMgr() { Release(); }

    int  Init   (const int patchSize, const int numPatchesPerSide);
    void Update (const float camPosX, const float camPosY, const float camPosZ);
    void Release();

    void PrintMap() const;

    //-----------------------------------------------------
    // Desc:   get patch's LOD metadata by its idx
    //-----------------------------------------------------
    inline const PatchLod& GetPatchLodInfo(const int idx) const
    {   return map_[idx];   }

    //-----------------------------------------------------
    // Desc:   get patch's LOD metadata by its idx along X and Z-axis
    // Args:   - patchX:  index by X-axis (not position)
    //         - patchZ:  index by Z-axis (not position)
    //-----------------------------------------------------
    inline const PatchLod& GetPatchLodInfo(const int patchX, const int patchZ) const
    {   return map_[(patchZ * numPatchesPerSide_) + patchX];    }

    //-----------------------------------------------------
    // Desc:   get a size of patch along X and Z-axis (patch is a square)
    //-----------------------------------------------------
    inline int GetPatchSize() const
    {   return patchSize_;  }

    //-----------------------------------------------------
    // Desc:   return a distance from the camera where LOD starts
    //-----------------------------------------------------
    inline int GetDistanceToLOD(const int lod) const
    {
        if ((lod < 0) || (lod > maxLOD_))
            return -1;

        return regions_[lod];
    }

    //-----------------------------------------------------

    bool SetDistanceToLOD(const int lod, const int dist);


private:
    void CalcLodRegions(const float farZ);
    void CalcMaxLOD();
    void UpdateLodMapPass1(const float camPosX, const float camPosY, const float camPosZ);
    void UpdateLodMapPass2(const float camPosX, const float camPosY, const float camPosZ);

    int  GetLodByDistance(const float distance);
    
public:
    // number of patches quads along X and Z-axis
    // (for instance: number == 256 + 1 (terrain width) / 16 + 1 (patch size))
    int       numPatchesPerSide_ = 0;

    int       maxLOD_ = 0;                // the number of LODs for this terrain's patches
    int       patchSize_ = 17;            // size (by X and Z-axis) of a single patch

    PatchLod* map_     = nullptr;         
    int       regions_[8]{0};             // distance from the camera where LOD starts
};

} // namespace Core
