// =================================================================================
// Filename:   TerrainBase.h
// Desc:       basic functional for terrain:
//             - loading terrain configs
// 
//             - generation of heightmaps (fault formation, midpoint displacement)
// 
//             - generation of texturemaps (one texture is binded to particular
//                                          height range, so on different heights we
//                                          have different textures but one texture
//                                          flows into another)
// 
//             - generation of lightmaps (limitation: the direction of light
//                                        must be of integer value)
//             - saving/loading of maps
// 
// =================================================================================
#pragma once

#include <Image.h>
#include <DMath.h>

namespace Core
{

// =================================================================================
// Enums
// =================================================================================
enum eTileTypes
{
    LOWEST_TILE = 0,   // sand, dirt, etc.
    LOW_TILE,          // grass
    HIGH_TILE,         // mountain side
    HIGHEST_TILE,      // tip of mountain
};

///////////////////////////////////////////////////////////

enum eLightingTypes
{
    HEIGHT_BASED = 0,
    LIGHTMAP,
    SLOPE_LIGHT
};

// =================================================================================
// Structures / constants
// =================================================================================

constexpr int TRN_NUM_TILES = 5;

///////////////////////////////////////////////////////////

struct TerrainConfig
{
    char    pathTextureMap[64]{'\0'};
    char    pathHeightMap[64]{'\0'};
    char    pathDetailMap[64]{'\0'};
    char    pathLightMap[64]{'\0'};

    char    pathLowestTile[64]{'\0'};
    char    pathLowTile[64]{'\0'};
    char    pathHighTile[64]{'\0'};
    char    pathHighestTile[64]{'\0'};

    char    pathSaveHeightMap[64]{ '\0' };
    char    pathSaveTextureMap[64]{ '\0' };
    char    pathSaveLightMap[64]{ '\0' };

    int     terrainLength = 257;                    // terrain length by X and Z-axis
    float   heightScale   = 0.4f;                   // scale factor for terrain heights


    uint8   generateTextureMap          :1 = 1;     // do we use any terrain texture map generation or load it from file?
    uint8   generateHeights             :1 = 1;     // do we use any terrain heights generation algorithm or load it from file?
    uint8   generateLightMap            :1 = 1;
    uint8   useGenFaultFormation        :1 = 1;     // what kind of heights generator will we use?
    uint8   useGenMidpointDisplacement  :1 = 0;
    uint8   saveTextureMap              :1 = 1;
    uint8   saveHeightMap               :1 = 1;
    uint8   saveLightMap                :1 = 1;

    // params related to the "Fault formation" algorithm of heights generation
    int     numIterations   = 64;      
    int     minDelta        = 0;                    // min height
    int     maxDelta        = 255;                  // max height
    float   filter          = 0.2f;                 // bigger value makes smoother terrain

    // params related to the "Midpoint displacement" algorithm of heights generation
    float   roughness       = 1.0f;                 // bigger value makes smoother terrain

    // params related to texture map properties
    int     textureMapSize  = 1024;                 // generated texture map width and height

    // lightmap generation params
    eLightingTypes      lightingType       = HEIGHT_BASED;
    Vec3                lightColor         = { 0.5f, 0.5f, 0.5f };
    int                 lightDirX          = 0;
    int                 lightDirZ          = 0;
    float               lightMinBrightness = 0.0f;
    float               lightMaxBrightness = 1.0f;
    float               shadowSoftness     = 10.0f;

    // geomipmapping params
    int patchSize = 17;
};

///////////////////////////////////////////////////////////

struct LightmapData
{
    uint8*   pData = nullptr;
    int      size  = 0;
    TexID    id    = INVALID_TEXTURE_ID;
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

struct CameraParams
{
    // 6 frustum planes (4 value per normalized plane: normal_vec(float3) + d(float))
    float planes[6][4];

    // position in world
    float posX;
    float posY;
    float posZ;

    // view matrix for UVN camera model
    float viewMatrix[16]{ 0 };
    float projMatrix[16]{ 0 };

    // frustum params in camera (view) space
    float rightSlope  = 0;          // positive X
    float leftSlope   = 0;          // negative X
    float topSlope    = 0;          // positive Y
    float bottomSlope = 0;          // negative Y
    float fovX        = 0;
    float fovY        = 0;
    float nearZ       = 0;
    float farZ        = 0;
};


//==================================================================================
// Class
//==================================================================================
class TerrainBase
{
public:
    TerrainBase() {}
    ~TerrainBase()
    {
        ClearMemoryFromMaps();
    }

    inline int   GetTerrainLength() const { return terrainLength_; }
    inline float GetHeightScale()   const { return heightScale_; }

    void ClearMemoryFromMaps(void );

    bool LoadSetupFile (const char* filename, TerrainConfig& outConfigs);
    bool LoadHeightMap (const char* filename, const int size);
    bool LoadTextureMap(const char* filename);

    bool SaveHeightMap(const char* filename);
    bool SaveTextureMap(const char* filename);
    bool SaveLightMap(const char* filename);

    void UnloadHeightMap();

    bool LoadHeightMapFromBMP(const char* filename);
    bool LoadHeightMapFromRAW(const char* filename);

    
    // texture map generation methods
    bool  GenerateTextureMap(const uint size);
    
    float RegionPercent     (const int tileType, const int height);
    void  GetTexCoords      (const Image& tex, uint& inOutX, uint& inOutY);

    int InterpolateHeight(
        const int x,
        const int z,
        const float heightToTexRatio,
        const float heightMapSize);


    // set the terrain's height scaling factor
    inline void SetHeightScale(const float scale) { heightScale_ = scale; }

    // Set the true height value at the given point
    inline void SetHeightAtPoint(const uint8_t height, const int x, const int z)
    {
        heightMap_.SetPixelGray(height, x, z);
    }

    // ----------------------------------------------------
    // Desc:  get the true height value (0-255) at a point
    // Args:  x,z: which height value to retrieve
    // Ret:   uchar value: the true height at the give point
    // ----------------------------------------------------
    inline uint8_t GetTrueHeightAtPoint(const int x, const int z) const
    {
        return heightMap_.GetPixelGray(x, z);
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

    // ----------------------------------------------------
    // Desc:   compute scaled height of terrain at given point (x,z)
    //         using bilinear interpolation
    // Args:   x, z:      our position
    // Ret:    float val: actual height at point (x,z)
    // ----------------------------------------------------
    inline float GetScaledInterpolatedHeightAtPoint(const float x, const float z) const
    {
        // make camera offset by Y-axis to be at fixed height over the terrain
        int posX = (int)(floorf(x));
        int posZ = (int)(floorf(z));

        float height1 = GetScaledHeightAtPoint(posX,   posZ);     //   3------4
        float height2 = GetScaledHeightAtPoint(posX+1, posZ);     //   |      |
        float height3 = GetScaledHeightAtPoint(posX,   posZ+1);   //   |      |
        float height4 = GetScaledHeightAtPoint(posX+1, posZ+1);   //   1------2

        // interpolation parameters
        float tx = x - posX;
        float tz = z - posZ;

        // bilinear interpolation
        float height12 = height1 + tx * (height2-height1);
        float height34 = height3 + tx * (height4-height3);

        // return terrain height at point (x,z)
        return height12 + tz * (height34-height12);
    }

    // ----------------------------------------------------


    // heightmap generators
    bool  GenHeightFaultFormation      (const int size, const int numIterations, const int minDelta, const int maxDelta, const float filter, const bool trueRandom = true);
    bool  GenHeightMidpointDisplacement(const int size, const float roughness, const bool trueRandom = true);

    // lighting methods
    bool  LoadLightMap(const char* filename);
    
    void  UnloadLightMap(void);

    uint8 CalculateLightingAtPoint(const int x, const int z);
    bool  CalculateLighting(void);

    void CalculateLightingHeightBased(const int size);
    void CalculateLightingSlope(
        const int size,
        const int dirX,
        const int dirZ,
        const float minBrightness,
        const float maxBrightness,
        const float softness);

    //--------------------------------------------------------------
    // load/unload a texture map for the terrain
    //--------------------------------------------------------------
    inline bool LoadTexture (const char* filename)
    {   return texture_.LoadData(filename);   }

    inline void UnloadTexture()
    {   texture_.Unload();  }

    //--------------------------------------------------------------
    // load/unload a detail map (which is used to add realism to the terrain)
    //--------------------------------------------------------------
    inline bool LoadDetailMap(const char* filename)
    {   return detailMap_.LoadData(filename);   }

    inline void UnloadDetailMap(void)
    {   detailMap_.Unload();   }

    //--------------------------------------------------------------
    // load/unload a single tile for the texture generation
    //--------------------------------------------------------------
    inline bool LoadTile(const eTileTypes tileType, const char* filename)
    {   return tiles_.textureTiles[tileType].LoadData(filename);   }

    inline void UnloadTile(const eTileTypes tileType)
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

    //--------------------------------------------------------------
    // Desc:   set the lighting technique to be used
    // Args:   - lightingType: which lighting technique to use
    //--------------------------------------------------------------
    inline void SetLightingType(const eLightingTypes lightingType)
    {   lightingType_ = lightingType;   }

    //--------------------------------------------------------------
    // Desc:   get the brightness values at a certain point in a lightmap
    // Args:   - x, z: which height value to retrieve
    // Ret:    uint8 value: the brightness
    //--------------------------------------------------------------
    inline uint8 GetBrightnessAtPoint(const int x, const int z)
    {   return lightmap_.pData[(z*lightmap_.size) + x]; }

    //--------------------------------------------------------------
    // Desc:   set brightness value at a certain point in a lightmap
    // Args:   - x, z:       which height value to set
    //         - brightness: value to set the lightmap pixel to
    //--------------------------------------------------------------
    inline void SetBrightnessAtPoint(const int x, const int z, const uint8 brightness)
    {   lightmap_.pData[(z*lightmap_.size) + x] = brightness;   }

    //--------------------------------------------------------------
    // Desc:   customize the parameters for slope lighting
    // Args:   - dirX, dirZ:    direction of the light, which can only be a whole number
    //         - softness:      the softness of the shadows
    //         - minBrightness: minimal brightness of the light
    //         - maxBrightness: maximal brightness of the light
    //--------------------------------------------------------------
    inline void CustomizeSlopeLighting(
        const int dirX,
        const int dirZ,
        const float minBrightness,
        const float maxBrightness,
        const float softness)
    {
        // set the min/max shading values
        minBrightness_ = minBrightness;
        maxBrightness_ = maxBrightness;

        lightSoftness_ = softness;

        // set the light direction
        directionX_ = dirX;
        directionZ_ = dirZ;
    }

private:
    // terrain heights erosion/bluring/normalization methods
    void FilterHeightBand (float* band, const int stride, const int count, const float filter);
    void FilterHeightField(float* heightData, const float filter);
    void NormalizeTerrain (float* heightData, const int size);

public:
    //int                 heightMapSize_ = 0;             // must be power of two
    int                 terrainLength_ = 0;    // since the terrain is always a square its width==depth (size by X == size by Z)
    float               heightScale_   = 1.0f;          // it lets us dynamically scale the heights of the terrain

    // texture info
    TerrainTextureTiles tiles_;
    Image               texture_;                 // diffuse texture
    Image               heightMap_;
    //Image               lightMap_;
    Image               detailMap_;
    bool                textureMapped_ = false;
    bool                multitextured_ = false;
    bool                detailMapped_ = false;

    // lighting info
    eLightingTypes      lightingType_  = HEIGHT_BASED;
    LightmapData        lightmap_;
    float               minBrightness_ = 0.0f;
    float               maxBrightness_ = 0.0f;
    float               lightSoftness_ = 0.0f;
    int                 directionX_    = 0;
    int                 directionZ_    = 0;

    // stat variables
    int vertsPerFrame_ = 0;
    int trisPerFrame_ = 0;
};

} // namespace
