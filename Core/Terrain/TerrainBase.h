#pragma once
#include "../Texture/Image.h"
#include <d3d11.h>



namespace Core
{
// =================================================================================
// Structures
// =================================================================================
constexpr int TRN_NUM_TILES = 5;

struct TerrainConfig
{
    char    pathTextureMap[64]{'\0'};
    char    pathHeightMap[64]{'\0'};
    char    pathDetailMap[64]{'\0'};

    char    pathSaveHeightMap[64]{'\0'};
    char    pathSaveTextureMap[64]{'\0'};

    char    pathLowestTile[64]{'\0'};
    char    pathLowTile[64]{'\0'};
    char    pathHighTile[64]{'\0'};
    char    pathHighestTile[64]{'\0'};

    int     depth       = 256;                  // terrain length by Z-axis
    int     width       = 256;                  // terrain length by X-axis
    float   heightScale = 0.4f;                 // scale factor for terrain heights


    uint8 generateTextureMap          :1 = 1;   // do we use any terrain texture map generation or load it from file?
    uint8 generateHeights             :1 = 1;   // do we use any terrain heights generation algorithm or load it from file?
    uint8 useGenFaultFormation        :1 = 1;   // what kind of heights generator will we use?
    uint8 useGenMidpointDisplacement  :1 = 0;

    // params related to the "Fault formation" algorithm of heights generation
    int     numIterations   = 64;      
    int     minDelta        = 0;       // min height
    int     maxDelta        = 255;     // max height
    float   filter          = 0.2f;    // bigger value makes smoother terrain

    // params related to the "Midpoint displacement" algorithm of heights generation
    float   roughness       = 1.0f;    // bigger value makes smoother terrain

    // params related to texture map properties
    int     textureMapSize  = 1024;    // generated texture map width and height


};


// =================================================================================
// Structures
// =================================================================================
enum eTileTypes
{
    LOWEST_TILE = 0,   // sand, dirt, etc.
    LOW_TILE,          // grass
    HIGH_TILE,         // mountain side
    HIGHEST_TILE,      // tip of mountain
};

///////////////////////////////////////////////////////////

struct HeightData
{
    uint8_t* pData = nullptr;   // the height data
    int      size = 0;          // the height size (power of 2)
};

///////////////////////////////////////////////////////////

struct TerrainTextureRegions
{   //    0%         optimalHeight        0%
    //    |               |               |
    //    |---------------|---------------|
    //    |               |               |
    // lowHeight         100%         highHeight

    int lowHeight     = 0;   // lower possible height (0%)   - from this height and until optimalHeight the texture presence increses
    int optimalHeight = 0;   // optimal height (100%)        - at this height the texture has 100% presense
    int highHeight    = 0;   // highest possible height (0%) - from optimalHeight and until highHeight the texture presence reduces
};

///////////////////////////////////////////////////////////

struct TerrainTextureTiles
{
    TerrainTextureRegions regions[TRN_NUM_TILES];    // texture regions (types of terrain heights)
    Image                 textureTiles[TRN_NUM_TILES];
    int                   numTiles = TRN_NUM_TILES;
};

///////////////////////////////////////////////////////////

class TerrainBase
{
public:
    TerrainBase() {}
    ~TerrainBase()
    {
        SafeDeleteArr(heightData_.pData);
    }

    bool LoadSetupFile (const char* filename, TerrainConfig& outConfigs);
    bool LoadHeightMap (const char* filename, const int size);
    bool LoadTextureMap(const char* filename);

    bool SaveHeightMap(const char* filename);
    void UnloadHeightMap();

    bool LoadHeightMapFromBMP(const char* filename);
    bool LoadHeightMapFromRAW(const char* filename);

    
    // texture map generation methods
    bool  GenerateTextureMap(const uint size);
    bool  SaveTextureMap(const char* filename);
    float RegionPercent(const int tileType, const uint8 height);
    void  GetTexCoords(const Image& tex, uint& inOutX, uint& inOutY);
    uint8 InterpolateHeight(const int x, const int z, const float heightToTexRatio);


    // set the terrain's height scaling factor
    inline void SetHeightScale(const float scale) { heightScale_ = scale; }

    // Set the true height value at the given point
    inline void SetHeightAtPoint(const uint8_t height, const int x, const int z)
    {
        heightData_.pData[(z*heightMapSize_) + x] = height;
    }

    // ----------------------------------------------------
    // Desc:  get the true height value (0-255) at a point
    // Args:  x,z: which height value to retrieve
    // Ret:   uchar value: the true height at the give point
    // ----------------------------------------------------
    inline uint8_t GetTrueHeightAtPoint(const int x, const int z) const
    {
        return heightData_.pData[(z*heightMapSize_) + x];
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

    inline int          GetDepth()       const { return terrainDepth_; }
    inline int          GetWidth()       const { return terrainWidth_; }
    inline float        GetHeightScale() const { return heightScale_; }

    // heightmap generators
    bool GenHeightFaultFormation      (const int size, const int numIterations, const int minDelta, const int maxDelta, const float filter, const bool trueRandom = true);
    bool GenHeightMidpointDisplacement(const int size, const float roughness, const bool trueRandom = true);

    //--------------------------------------------------------------
    // load a texture map for the terrain
    //--------------------------------------------------------------
    inline bool LoadTexture (const char* filename)
    {   return texture_.LoadData(filename);   }

    //--------------------------------------------------------------
    // unload the terrain's diffuse texture
    //--------------------------------------------------------------
    inline void UnloadTexture()
    {   texture_.Unload();  }

    //--------------------------------------------------------------
    // load a detail map to add realism to the terrain
    //--------------------------------------------------------------
    inline bool LoadDetailMap(const char* filename)
    {   return detailMap_.LoadData(filename);   }

    //--------------------------------------------------------------
    // unload the terrain's detail map
    //--------------------------------------------------------------
    inline void UnloadDetailMap(void)
    {   detailMap_.Unload();   }

    //--------------------------------------------------------------
    // load a single tile for the texture generation
    //--------------------------------------------------------------
    inline bool LoadTile(const eTileTypes tileType, const char* filename)
    {   return tiles_.textureTiles[tileType].LoadData(filename);   }

    //--------------------------------------------------------------
    // unload a single tile by input type
    //--------------------------------------------------------------
    inline bool UnloadTile(const eTileTypes tileType)
    {   tiles_.textureTiles[tileType].Unload();   }

    //--------------------------------------------------------------
    // Unload all tiles which were used for the texture generation
    //--------------------------------------------------------------
    inline void UnloadAllTiles()
    {
        UnloadTile(LOWEST_TILE);
        UnloadTile(LOW_TILE);
        UnloadTile(HIGH_TILE);
        UnloadTile(HIGHEST_TILE);
    }


private:
    // terrain heights erosion/bluring/normalization methods
    void FilterHeightBand (float* band, const int stride, const int count, const float filter);
    void FilterHeightField(float* heightData, const float filter);
    void NormalizeTerrain (float* heightData, const int size);

public:
    int                 heightMapSize_ = 0;             // must be power of two
    int                 terrainDepth_ = 0;              // size by Z-axis
    int                 terrainWidth_  = 0;             // size by X-axis
    float               heightScale_   = 1.0f;          // it lets us dynamically scale the heights of the terrain
    HeightData          heightData_;

    // texture information
    TerrainTextureTiles tiles_;
    Image               texture_;                 // diffuse texture
    Image               detailMap_;
    bool                textureMapped_ = false;
    bool                multitextured_ = false;
    bool                detailMapped_ = false;
};

} // namespace
