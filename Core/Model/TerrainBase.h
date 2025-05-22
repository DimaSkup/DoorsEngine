#pragma once
#include <CoreCommon/MemHelpers.h>
#include <stdint.h>


namespace Core
{

struct HeightData
{
    uint8_t* pData = nullptr;   // the height data
    int      size = 0;          // the height size (power of 2)
};

// =================================================================================

class TerrainBase
{
public:
    TerrainBase() {}
    ~TerrainBase()
    {
        SafeDeleteArr(heightData_.pData);
    }

    bool LoadSetupFile(const char* filename);

    bool LoadHeightMap(const char* filename, const int size);
    bool SaveHeightMap(const char* filename);
    void UnloadHeightMap();



    // ----------------------------------------------------
    // Desc:  Set the height scaling variable
    // Args:  scale: how much to scale the terrain
    // Ret:   None
    // ----------------------------------------------------
    inline void SetHeightScale(const float scale) { heightScale_ = scale; }

    // ----------------------------------------------------
    // Desc:  Set the true height value at the given point
    // Args:  height: the new height value for the point
    //        x,z:    which height value to change
    // Ret:   None
    // ----------------------------------------------------
    inline void SetHeightAtPoint(const uint8_t height, const int x, const int z)
    {
        heightData_.pData[(z*size_) + x] = height;
    }

    // ----------------------------------------------------
    // Desc:  get the true height value (0-255) at a point
    // Args:  x,z: which height value to retrieve
    // Ret:   uchar value: the true height at the give point
    // ----------------------------------------------------
    inline uint8_t GetTrueHeightAtPoint(const int x, const int z) const
    {
        return heightData_.pData[(z*size_) + x];
    }

    // ----------------------------------------------------
    // Desc:  get the scaled height at a given point
    // Args:  x, z: which height value to retrieve
    // Ret:   float val: the scaled height at the give point
    // ----------------------------------------------------
    inline float GetScaledHeightAtPoint(const int x, const int z) const
    {
        return heightScale_ * (float)GetTrueHeightAtPoint(x, z);
    }

    inline const char*  GetFilename()    const { return terrainFilename_; }  
    inline int          GetDepth()       const { return terrainDepth_; }
    inline int          GetWidth()       const { return terrainWidth_; }
    inline float        GetHeightScale() const { return heightScale_; }

    // heightmap generators
    void GenHeightFaultFormation(const int size, const int numIterations, const int minDelta, const int maxDelta, const float filter);


private:
    // terrain heights erosion/bluring/normalization methods
    void FilterHeightBand (float* band, const int stride, const int count, const float filter);
    void FilterHeightField(float* heightData, const float filter);
    void NormalizeTerrain (float* heightData, const int size);

public:
    int        size_ = 0;                       // must be power of two

protected:
    char       terrainFilename_[64]{ '\0' };    // path to the terrain heightmap or raw-file
    int        terrainDepth_ = 0;               // size by Z-axis
    int        terrainWidth_  = 0;              // size by X-axis
    float      heightScale_   = 1.0f;           // it lets us dynamically scale the heights of the terrain
    HeightData heightData_;
};

} // namespace
