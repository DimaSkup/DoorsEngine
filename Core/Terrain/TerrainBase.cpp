// =================================================================================
// Filename:   TerrainBase.cpp
// =================================================================================
#include <CoreCommon/pch.h>
#include "TerrainBase.h"
#include "../Texture/texture_mgr.h"
#include <time.h>

#pragma warning (disable : 4996)
#pragma warning (disable : 6031)

namespace Core
{

// =================================================================================
// Public methods
// =================================================================================

// --------------------------------------------------------
// Desc:   release memory from all the images raw pixels data
//         (usually we call it after creation of all the necessary
//          texture resources on GPU)
// --------------------------------------------------------
void TerrainBase::ClearMemoryFromMaps(void)
{
    UnloadHeightMap();
    UnloadLightMap();
    UnloadTexture();       // texture tile map (diffuse)
    UnloadDetailMap();
    UnloadAllTiles();
}

//---------------------------------------------------------
// Desc:  a helper for reading float4 from file
// 
// NOTE:  I don't check fucking any input args here so pay attention
//---------------------------------------------------------
inline void ReadFloat4(FILE* pFile, const char* fmt, float* arr)
{
    int count = fscanf(pFile, fmt, &arr[0], &arr[1], &arr[2], &arr[3]);

    if (count != 4)
        LogErr(LOG, "error when read: %s\n was read: %f %f %f %f",
                    fmt, arr[0], arr[1], arr[2], arr[3]);
}

//---------------------------------------------------------
// Desc:  read float3 from file
//---------------------------------------------------------
inline void ReadFloat3(FILE* pFile, const char* fmt, float* arr)
{
    int count = fscanf(pFile, fmt, &arr[0], &arr[1], &arr[2]);

    if (count != 3)
        LogErr(LOG, "error when read: %s\n was read: %f %f %f",
                    fmt, arr[0], arr[1], arr[2]);
}

//---------------------------------------------------------
// Desc:  read a float from file
//---------------------------------------------------------
inline void ReadFloat(FILE* pFile, const char* fmt, float* f)
{
    int count = fscanf(pFile, fmt, f);

    if (count != 1)
        LogErr(LOG, "error when read: %s\n was read: %f", fmt, f);
}

//---------------------------------------------------------
// Desc:  read an integer from from file
//---------------------------------------------------------
inline void ReadInt(FILE* pFile, const char* fmt, int* i)
{
    int count = fscanf(pFile, fmt, i);

    if (count != 1)
        LogErr(LOG, "error when read: %s\n was read: %d", fmt, i);
}

//---------------------------------------------------------
// Desc:  read a string from file
//---------------------------------------------------------
inline void ReadStr(FILE* pFile, const char* fmt, char* outStr)
{
    int count = fscanf(pFile, fmt, outStr);

    if (count != 1)
        LogErr(LOG, "error when read: %s\n was read: %s", fmt, outStr);
}

// --------------------------------------------------------
// Desc:  load a file with terrain's settings
// Args:  - filename: path to the setup file
// --------------------------------------------------------
bool TerrainBase::LoadSetupFile(const char* filename, TerrainConfig& outConfigs)
{
    // check input params
    if (StrHelper::IsEmpty(filename))
    {
        LogErr(LOG, "terrain setup filename is empty: %s", filename);
        return false;
    }

    // open the setup file
    FILE* pFile = fopen(filename, "r");
    if (!pFile)
    {
        LogErr(LOG, "can't open a terrain setup file: %s", filename);
        return false;
    }

    int           count = 0;
    int           tempBool = 0;
    constexpr int bufsize = 256;
    char          buf[bufsize]{ '\0' };
    char          tmpChars[64]{'\0'};
    float         fl[4]{0};

    LogDbg(LOG, "Start: read in the terrain setup file");

    // read in terrain common data
    ReadStr(pFile, "Name_tex_map_0:          %s\n", outConfigs.texDiffName0);
    ReadStr(pFile, "Name_tex_map_1:          %s\n", outConfigs.texDiffName1);
    ReadStr(pFile, "Name_tex_map_2:          %s\n", outConfigs.texDiffName2);
    ReadStr(pFile, "Name_tex_map_3:          %s\n", outConfigs.texDiffName3);
    ReadStr(pFile, "Name_alpha_map:          %s\n", outConfigs.texAlphaName);

    ReadStr(pFile, "Path_height_map:         %s\n", outConfigs.pathHeightMap);
    ReadStr(pFile, "Name_detail_map:         %s\n", outConfigs.texNameDetailMap);
    ReadStr(pFile, "Name_light_map:          %s\n", outConfigs.texNameLightmap);
    ReadStr(pFile, "Path_nature_density_map: %s\n", outConfigs.pathNatureDensityMap);

    ReadStr(pFile, "Name_normal_map_0:       %s\n", outConfigs.texNormName0);
    ReadStr(pFile, "Name_normal_map_1:       %s\n", outConfigs.texNormName1);
    ReadStr(pFile, "Name_normal_map_2:       %s\n", outConfigs.texNormName2);
    ReadStr(pFile, "Name_normal_map_3:       %s\n", outConfigs.texNormName3);

    ReadStr(pFile, "Name_specular_map_0:     %s\n", outConfigs.texSpecName0);
    ReadStr(pFile, "Name_specular_map_1:     %s\n", outConfigs.texSpecName1);
    ReadStr(pFile, "Name_specular_map_2:     %s\n", outConfigs.texSpecName2);
    ReadStr(pFile, "Name_specular_map_3:     %s\n", outConfigs.texSpecName3);

    // width==depth, height_scale
    ReadInt(pFile,   "Terrain_length: %d\n", &outConfigs.terrainLength);
    ReadFloat(pFile, "Height_scaling: %f\n", &outConfigs.heightScale);

    // setup data fields
    terrainLength_ = outConfigs.terrainLength;
    heightScale_   = outConfigs.heightScale;

    // --------------------------------

    // do we want to generate terrain texture (tile) map?
    fgets(buf, bufsize, pFile);
    count = sscanf(buf, "Generate_texture_map: %d", &tempBool);
    assert(count == 1);
    outConfigs.generateTextureMap = (uint16)tempBool;

    // do we want to generate terrain heights?
    fgets(buf, bufsize, pFile);
    sscanf(buf, "Generate_heights: %d", &tempBool);
    outConfigs.generateHeights = (uint16)tempBool;

    // do we want to generate lightmap for the terrain?
    fgets(buf, bufsize, pFile);
    sscanf(buf, "Generate_lightmap: %d", &tempBool);
    outConfigs.generateLightMap = (uint16)tempBool;

    // --------------------------------

    // do we want to store generated texture_map/height_map/light_map into the files?
    fgets(buf, bufsize, pFile);
    sscanf(buf, "Save_texture_map: %d", &tempBool);
    outConfigs.saveTextureMap = (uint16)tempBool;

    fgets(buf, bufsize, pFile);
    sscanf(buf, "Save_height_map: %d", &tempBool);
    outConfigs.saveHeightMap = (uint16)tempBool;

    fgets(buf, bufsize, pFile);
    sscanf(buf, "Save_light_map: %d", &tempBool);
    outConfigs.saveLightMap = (uint16)tempBool;

    fgets(buf, bufsize, pFile);
    sscanf(buf, "Use_lightmap: %d", &tempBool);
    outConfigs.useLightmap = (uint16)tempBool;

    // --------------------------------

    fgets(buf, bufsize, pFile);
    sscanf(buf, "Use_gen_fault_formation: %d", &tempBool);
    outConfigs.useGenFaultFormation = (uint16)tempBool;

    fgets(buf, bufsize, pFile);
    sscanf(buf, "Use_gen_midpoint_displacement: %d", &tempBool);
    outConfigs.useGenMidpointDisplacement = (uint16)tempBool;

    // --------------------------------

    // read in params for heights generation
    fgets(buf, bufsize, pFile);
    sscanf(buf, "Fault_formation_iterations: %d", &outConfigs.numIterations);

    fgets(buf, bufsize, pFile);
    sscanf(buf, "Fault_formation_min_delta: %d", &outConfigs.minDelta);

    fgets(buf, bufsize, pFile);
    sscanf(buf, "Fault_formation_max_delta: %d", &outConfigs.maxDelta);

    fgets(buf, bufsize, pFile);
    sscanf(buf, "Fault_formation_filter: %f", &outConfigs.filter);

    fgets(buf, bufsize, pFile);
    sscanf(buf, "Midpoint_displacement_roughness: %f", &outConfigs.roughness);

    // --------------------------------

    // read in params for texture map
    fgets(buf, bufsize, pFile);
    sscanf(buf, "Generated_texture_map_size: %d", &outConfigs.textureMapSize);

    fgets(buf, bufsize, pFile);
    sscanf(buf, "Path_lowest_tile: %s", outConfigs.pathLowestTile);

    fgets(buf, bufsize, pFile);
    sscanf(buf, "Path_low_tile: %s", outConfigs.pathLowTile);

    fgets(buf, bufsize, pFile);
    sscanf(buf, "Path_high_tile: %s", outConfigs.pathHighTile);

    fgets(buf, bufsize, pFile);
    sscanf(buf, "Path_highest_tile: %s", outConfigs.pathHighestTile);

    fgets(buf, bufsize, pFile);
    sscanf(buf, "Path_save_height_map: %s", outConfigs.pathSaveHeightMap);

    fgets(buf, bufsize, pFile);
    sscanf(buf, "Path_save_texture_map: %s", outConfigs.pathSaveTextureMap);

    fgets(buf, bufsize, pFile);
    sscanf(buf, "Path_save_light_map: %s", outConfigs.pathSaveLightMap);


    // lightmap generation params
    fgets(buf, bufsize, pFile);
    sscanf(buf, "Lighting_type: %s", tmpChars);

    if (strcmp(tmpChars, "HEIGHT_BASED") == 0)
        outConfigs.lightingType = HEIGHT_BASED;

    else if (strcmp(tmpChars, "LIGHTMAP") == 0)
        outConfigs.lightingType = LIGHTMAP;

    else if (strcmp(tmpChars, "SLOPE_LIGHT") == 0)
        outConfigs.lightingType = SLOPE_LIGHT;


    fgets(buf, bufsize, pFile);
    sscanf(buf, "Light_direction_x: %d",      &outConfigs.lightDirX);

    fgets(buf, bufsize, pFile);
    sscanf(buf, "Light_direction_z: %d",      &outConfigs.lightDirZ);

    fgets(buf, bufsize, pFile);
    sscanf(buf, "Light_min_brightness: %f",   &outConfigs.lightMinBrightness);

    fgets(buf, bufsize, pFile);
    sscanf(buf, "Light_max_brightness: %f",   &outConfigs.lightMaxBrightness);

    fgets(buf, bufsize, pFile);
    sscanf(buf, "Light_shadow_softness: %f",  &outConfigs.shadowSoftness);


    // load geomipmapping params
    fgets(buf, bufsize, pFile);
    sscanf(buf, "Geomipmapping_patch_size: %d", &outConfigs.patchSize);


    // read material colors
    ReadFloat4(pFile, "Material_ambient: %f %f %f %f\n",    outConfigs.ambient);
    ReadFloat4(pFile, "Material_diffuse: %f %f %f %f\n",    outConfigs.diffuse);

    ReadFloat3(pFile, "Material_specular: %f %f %f\n",      outConfigs.specular);
    ReadFloat (pFile, "Material_glossiness: %f\n",         &outConfigs.glossiness);
    ReadFloat4(pFile, "Material_reflection: %f %f %f %f\n", outConfigs.reflect);


    // read LODs distances
    ReadInt(pFile, "Distance_lod_0: %d\n", &outConfigs.distToLod0);
    ReadInt(pFile, "Distance_lod_1: %d\n", &outConfigs.distToLod1);
    ReadInt(pFile, "Distance_lod_2: %d\n", &outConfigs.distToLod2);
    ReadInt(pFile, "Distance_lod_3: %d\n", &outConfigs.distToLod3);


    // close the setup file
    fclose(pFile);

    LogDbg(LOG, "End: the terrain setup file is read in successfully!");
    return true;
}

// --------------------------------------------------------
// Desc:   load height map from bmp-file by filename
// Args:   - filename:  path to image
// --------------------------------------------------------
bool TerrainBase::LoadHeightMapFromBMP(const char* filename)
{
    return heightMap_.LoadGrayscaleBMP(filename);
}

// --------------------------------------------------------
// Desc:   load a grayscale RAW height map
//         (it contains num elements and raw height data)
// Args:   - filename: path to the file
// --------------------------------------------------------
bool TerrainBase::LoadHeightMapFromRAW(const char* filename)
{
    return heightMap_.LoadData(filename);
}

// --------------------------------------------------------
// Desc:  load a height map from the file
// Args:  - filename: path to the file
//        - size:     dimension of the terrain by X and Z
//                    (expected to be the same as filename's)
// --------------------------------------------------------
bool TerrainBase::LoadHeightMap(const char* filename, const int size)
{
    char extension[8]{'\0'};
    FileSys::GetFileExt(filename, extension);

    // load from bmp
    if (strcmp(extension, ".bmp") == 0)
    {
        if (!LoadHeightMapFromBMP(filename))
        {
            LogErr(LOG, "can't load height map from the file: %s", filename);
            return false;
        }
    }
    // load from raw
    else if (strcmp(extension, ".raw") == 0)
    {
        if (!LoadHeightMapFromRAW(filename))
        {
            LogErr(LOG, "can't load height map from the file: %s", filename);
            return false;
        }
    }

    return true;
}

// --------------------------------------------------------
// Desc:  load a texture (tile) map from the file 
//        NOTE: 1. supported formats: BMP, DDS
//              2. doesn't create texture resource (!!!)
// 
// Args:  - texName:  a name of the loaded tilemap
// --------------------------------------------------------
bool TerrainBase::LoadTextureMap(const char* texName)
{
    if (!texName || texName[0] == '\0')
    {
        LogErr(LOG, "empty name");
        return false;
    }


    const TexID texId = g_TextureMgr.GetTexIdByName(texName);
    if (texId == INVALID_TEX_ID)
    {
        LogErr(LOG, "no texture by name: %s", texName);
        return false;
    }

    const Texture& tex = g_TextureMgr.GetTexByID(texId);

    // set params of tile map
    texture_.SetID(texId);
    texture_.SetWidth(tex.GetWidth());
    texture_.SetHeight(tex.GetHeight());
    texture_.SetBPP(32);

    return true;


#if 0
    char extension[8]{ '\0' };
    FileSys::GetFileExt(filename, extension);

    // load from BMP
    if (strcmp(extension, ".bmp") == 0)
    {
        // load in data
        if (!texture_.LoadData(filename))
        {
            LogErr(LOG, "can't load texture map from the file: %s", filename);
            return false;
        }

        Image& tileMap = texture_;
        constexpr bool mipMapped = true;

        // create texture resource
        const TexID tileMapTexId = g_TextureMgr.CreateTextureFromRawData(
            "terrain_tile_map",
            tileMap.GetData(),
            tileMap.GetWidth(),
            tileMap.GetHeight(),
            tileMap.GetBPP(),
            mipMapped);

        tileMap.SetID(tileMapTexId);
    }

    // load from DDS
    if (strcmp(extension, ".dds") != 0)
    {
        LogErr(LOG, "can't load terrain texture map from file: %s\n"
            "REASON: unsupported format: %s", filename, extension);
        return false;
    }
#endif
}

// --------------------------------------------------------
// Desc:  save a grayscale RAW height map
// Args:  filename: the file name of the height map
// --------------------------------------------------------
bool TerrainBase::SaveHeightMap(const char* filename)
{
    return heightMap_.Save(filename);
}

// --------------------------------------------------------
// Desc:  release the memory from the heights data
//        and reset the map dimensions
// --------------------------------------------------------
void TerrainBase::UnloadHeightMap()
{
    heightMap_.Unload();
}

// --------------------------------------------------------
// Desc:  generate a height data using the method of fractal
//        terrain generation is called "Fault Formation".
//        Note that this algorithm allows you specify any
//        dimension of terrain and size not necessarally must
//        be power of 2.
// Args:  - size:          desired size of the height map
//        - numIterations: number of detail passes to make
//        - minDelta:      lowest value for the height
//        - maxDelta:      highest value for the height
//        - filter:        strength of the filter
//        - trueRandom:    (true by default) if true each time
//                         when we call this method -- terrain
//                         will be completely different from the previous one
// --------------------------------------------------------
bool TerrainBase::GenHeightFaultFormation(
    const int size,
    const int numIterations,
    const int minDelta,       
    const int maxDelta,    
    const float filter,
    const bool trueRandom)
{
    float* tempBuf = nullptr;   // we use this temp buffer because we need higher precision during computations

    LogDbg(LOG, "wait while height map (fault formation) is generated");

    try
    {
        // check input params
        CAssert::True(size > 0,          "input size (dimensions) for height map must be > 0");
        CAssert::True(numIterations > 0, "input number of iterations must be > 0");

        // unload previous height data if we has any
        heightMap_.Unload();

        // need it for truly random generation of heights
        if (trueRandom)
            srand((unsigned int)time(NULL));

        // alloc the memory for our height data
        heightMap_.Create(size, size, 8);
        tempBuf = new float[size * size]{ 0.0f };

        for (int currIteration = 0; currIteration < numIterations; ++currIteration)
        {
            // calculate the height range (lerp from maxDelta to minDelta) for this fault-pass
            const float height = (float)(maxDelta - ((maxDelta - minDelta) * currIteration) / numIterations);

            // pick two points at random from the entire height map
            const int randX1 = rand() % size;
            const int randZ1 = rand() % size;
            int       randX2 = rand() % size;
            int       randZ2 = rand() % size;

            // check to make sure that the points are not the same
            while ((randX2 == randX1) && (randZ2 == randZ1))
            {
                randX2 = rand() % size;
                randZ2 = rand() % size;
            }

            // <dirX1, dirX2> is a vec going the same direction as the division line
            const int dirX1 = randX2 - randX1;
            const int dirZ1 = randZ2 - randZ1;

            // set heights for one half
            for (int z = 0; z < size; ++z)
            {
                for (int x = 0; x < size; ++x)
                {
                    // <dirX2, dirZ2> is a vector from point <randX1, randZ1> to the curr point (in the loop)
                    const int dirX2 = x - randX1;
                    const int dirZ2 = z - randZ1;

                    // if the result of (dirX2*dirZ1 - dirX1*dirZ2) is "up" (above 0), then raise this point by height
                    if ((dirX2 * dirZ1 - dirX1 * dirZ2) > 0)
                        tempBuf[(z * size) + x] += height;
                }
            }

            // erode terrain
            FilterHeightField(tempBuf, filter);
        }

        // normalize the terrain for our purposes
        NormalizeTerrain(tempBuf, size);

        // transfer the terrain into our class's uint8_t height buffer
        for (int z = 0; z < size; ++z)
        {
            for (int x = 0; x < size; ++x)
                SetHeightAtPoint((uint8)tempBuf[(z * size) + x], x, z);
        }

        // delete temp buffer
        SafeDeleteArr(tempBuf);

        LogMsg("height map is generated successfully");
        return true;
    }
    catch (std::bad_alloc& e)
    {
        LogErr(e.what());
        throw EngineException("can't allocate the memory for heights data");
    }
    catch (EngineException& e)
    {
        heightMap_.Unload();
        SafeDeleteArr(tempBuf);

        LogErr(e);
        return false;
    }
}

// --------------------------------------------------------
// Desc:  generate a height data using the method of 
//        terrain generation is called "Midpoint Displacement"
//        (also known as the "plasma fractal" and the
//        "diamond-square algorithm).
//        Note: this algorithm has limited use, since
//        CLOD algorithms usually require a height map
//        size of (n^2)+1 x (n^2)+1, and this algorithm
//        can only generate (n^2) x (n^2) maps
// Args:  - size:       desired size of the height map
//        - roughness:  desired roughness of the terrain
//                      (best results are from 0.25f to 1.5f)
//        - trueRandom: (true by default) if true each time
//                      when we call this method -- terrain
//                      will be completely different from the previous one
// --------------------------------------------------------
bool TerrainBase::GenHeightMidpointDisplacement(const int size, float roughness, const bool trueRandom)
{
    LogDbg(LOG, "wait while height map (midpoint displacement) is generated");

    float* tempBuf = nullptr;    // temp buf for height data (we need it for higher precision)
    int    rectSize = size;

    try
    {
        // check input params
        CAssert::True(size > 0,                   "input size of mipmap must be > 0");
        CAssert::True(IsPow2(size), "input size must be a power of 2");

        // unload previous height data if we has any
        heightMap_.Unload();

        if (roughness < 0)
            roughness *= -1;

        float height = (float)(size / 2);                   
        const float heightReducer = powf(2, -1*roughness);

        // need it for truly random generation of heights
        if (trueRandom)
            srand((unsigned int)time(NULL));

        // alloc the memory for our height data
        heightMap_.Create(size, size, 8);
        tempBuf = new float[size*size]{0.0f};

        // being the displacement process
        while (rectSize > 0)
        {
            // min/max heights for the current iteration
            const float minHeight = -height * 0.5f;
            const float maxHeight = +height * 0.5f;

            const int halfRectSize = rectSize >> 1;

            /* Diamond step -

            Find the values at the center of the rectangles by averaging the values at
            the corners and adding a random offset:

            a.....b
            .     .
            .  e  .
            .     .
            c.....d

            e = (a+b+c+d)/4 + random

            In the code below:
            a = (i,j)
            b = (ni, j)
            c = (i, nj)
            d = (ni,nj)
            e = (mi,mj)  */

            for (int i = 0; i < size; i += rectSize)
            {
                const int ni = (i+rectSize) & (size-1);       // (i + rectSize) % size
                const int mi = i + halfRectSize;

                for (int j = 0; j < size; j += rectSize)
                {
                    const int nj  = (j+rectSize) & (size-1);  // (j+rectSize) % size
                    const int mj  = j + halfRectSize;
                    const int idx = mi + (mj*size);

                    tempBuf[idx] =
                        tempBuf[i  + (j*size)]  +
                        tempBuf[ni + (j*size)]  +
                        tempBuf[i  + (nj*size)] +
                        tempBuf[ni + (nj*size)];

                    tempBuf[idx] *= 0.25f;
                    tempBuf[idx] += RandF(minHeight, maxHeight);
                }
            }

            /* Square step -

            Find the values on the left and top sides of each rectangle.
            The right and bottom sides are the left and top sides of the neighboring
            rectangles, so we don't need to calculate them.

            The height heightData_.pData wraps, so we're never left handing. The right
            side of the last rectangle in a row is the left side of the first rectangle
            in the row. The bottom side of the last rectangle in a column is the top side
            of the first rectangle in the column.
           
                  .......
                  .     .
                  .     .
                  .  d  .
                  .     .
                  .     .
            ......a..g..b
            .     .     .
            .     .     .
            .  e  h  f  .
            .     .     .
            .     .     .
            ......c......

            g = (d+f+a+b)/4 + random
            h = (a+c+e+f)/4 + random

            In the code below:
                a = (i,j)
                b = (ni,j)
                c = (i,nj)
                d = (mi,pmj)
                e = (pmi,mj)
                f = (mi,mj)
                g = (mi,j)
                h = (i,mj)  */
            for (int i = 0; i < size; i += rectSize)
            {
                const int ni  = (i+rectSize) & (size-1);                 // (i+rectSize) % size
                const int mi  = (i+halfRectSize);
                const int pmi = (i-halfRectSize+size) & (size-1);        // (i-halfRectSize+size) % size

                for (int j = 0; j < size; j += rectSize)
                {
                    const int nj  = (j+rectSize) & (size-1);             // (j + rectSize) % size
                    const int mj  = (j+halfRectSize);
                    const int pmj = (j-halfRectSize+size) & (size-1);    // (j-halfRectSize + size) % size

                    // calculate the square value for the top side of the rectangle
                    const int idx1 = mi + (j*size);

                    tempBuf[idx1] =
                        tempBuf[i  + (j*size)]    +
                        tempBuf[ni + (j*size)]    +
                        tempBuf[mi + (pmj*size)]  +
                        tempBuf[mi + (mj*size)];

                    tempBuf[idx1] *= 0.25f;
                    tempBuf[idx1] += RandF(minHeight, maxHeight);

                    // calculate the square value for the left side of the rectangle
                    const int idx2 = i + (mj*size);

                    tempBuf[idx2] =
                        tempBuf[i   + (j*size)]   +
                        tempBuf[i   + (nj*size)]  +
                        tempBuf[pmi + (mj*size)]  +
                        tempBuf[mi  + (mj*size)];

                    tempBuf[idx2] *= 0.25f;
                    tempBuf[idx2] += RandF(minHeight, maxHeight);
                }
            }

            // reduce the rectangle size by two to prepare for the next displacement stage
            rectSize /= 2;

            // reduce the height by the height reducer
            height *= heightReducer;
        }

        // normalize the terrain for our purposes (then we pack height values into uint8_t)
        NormalizeTerrain(tempBuf, size);

        // transfer the terrain into our class's uint_8 height buffer
        for (int z = 0; z < size; ++z)
        {
            for (int x = 0; x < size; ++x)
                SetHeightAtPoint((uint8_t)tempBuf[(z*size) + x], x, z);
        }

        // delete temp buffer
        SafeDeleteArr(tempBuf);

        LogMsg("height map is generated successfully");
        return true;
    }
    catch (std::bad_alloc& e)
    {
        LogErr(e.what());
        throw EngineException("can't allocate the memory for height data");
    }
    catch (EngineException& e)
    {
        heightMap_.Unload();
        SafeDeleteArr(tempBuf);
        LogErr(e);
        return false;
    }
}


// =================================================================================
// texture map related methods
// =================================================================================

// ----------------------------------------------------
// Desc:   generate a texture map from four tiles (that
//         must be loaded before this function is called)
// Args:   - size: the size of the texture map to be generated
// ----------------------------------------------------
bool TerrainBase::GenerateTextureMap(const uint texMapSize)
{
    // check input params
    if (texMapSize <= 0)
    {
        LogErr(LOG, "can't generate texture map: input size for texture map must be > 0");
        return false;
    }

    LogDbg(LOG, "wait while texture map is generated");

    // find out the number and indices of tiles that we have
    int numTiles = 0;
    int loadedTilesIdxs[TRN_NUM_TILES]{ 0 };

    for (int i = 0; i < TRN_NUM_TILES; ++i)
    {
        // if the curr tile is loaded, then we add one to the total tile count
        loadedTilesIdxs[numTiles] = i;
        numTiles += tiles_.textureTiles[i].IsLoaded();
    }

    tiles_.numTiles = numTiles;

    /*
        [idx] - index of the tile
        L     - lowest height
        O     - optimal height
        H     - highest height

        0  ---- 255/4 -1 ---- 255/4 -1 ---- 255/4 -1 ---- 255/4 -1
    [0] L          O             H
    [1]            L             O            H
    [2]                          L            O              H
    [3]                                       L              O               

    */

    // now, re-loop through, and calculate the texture regions
    int       lastHeight   = -1;
    const int heightStride = 255 / numTiles;

    for (int i = 0; i < numTiles; ++i)
    {
        // we only want to perform these calculations if we actually have a tile loaded
        const int tileIdx = loadedTilesIdxs[i];
        TerrainTextureRegions& reg = tiles_.regions[tileIdx];

        // calculate the three height boundaries (low, optimal, high)
        reg.lowHeight     = lastHeight + 1;
        lastHeight        += heightStride;

        reg.optimalHeight = lastHeight;
        reg.highHeight    = (lastHeight - reg.lowHeight) + lastHeight;
    }

    // create room for a new texture
    constexpr uint bpp = 24;
    texture_.Create(texMapSize, texMapSize, bpp);

    // get the height map to texture map ratio (since, the most of the time,
    // the texture map will be a higher resolution that the height map, so we
    // need the ration of height map pixels to texture map pixels)
    const float heightMapSize         = (float)heightMap_.GetWidth();
    const float mapRatio              = heightMapSize / texMapSize;    // for instance: 128 / 256

    const int iTexMapSize             = (int)texMapSize;
    const int lowestTileOptimalHeight = tiles_.regions[LOWEST_TILE].optimalHeight;
        
    // create the texture data
    for (int z = 0; z < iTexMapSize; ++z)
    {
        for (int x = 0; x < iTexMapSize; ++x)
        {
            // set our total color counters to 0.0f
            float totalRed   = 0.0f;
            float totalGreen = 0.0f;
            float totalBlue  = 0.0f;

            // compute interpolated height
            const int height = (int)InterpolateHeight(x, z, mapRatio, heightMapSize);


            // loop through the loaded tiles
            for (int i = 0; i < numTiles; ++i)
            {
                const int tileIdx                  = loadedTilesIdxs[i];
                const TerrainTextureRegions region = tiles_.regions[tileIdx];

                // if current height doesn't belong to the current region
                if ((height < region.lowHeight) || (height > region.highHeight))
                    continue;

                const Image& tile = tiles_.textureTiles[tileIdx];
                float blendFactor = 0.0f;

                uint texX = x;
                uint texZ = z;

                // get texture coordinates
                GetTexCoords(tile, texX, texZ);

                // get the curr color in the texture at the coordinates that we got in GetTexCoords
                uint8 red, green, blue;
                tile.GetColor(texX, texZ, red, green, blue);

                // if the height is lower than the lowest tile's height, then we want full brightness,
               // if we don't do this, the area will get darkened, and no texture will get shown
                if ((tileIdx == LOWEST_TILE) && (height < lowestTileOptimalHeight))
                    blendFactor = 1.0f;

                // get the curr coordinate's blending percentage for this tile
                else
                    blendFactor = RegionPercent(tileIdx, height);

                // calculate the RGB values that will be used
                totalRed   += (red   * blendFactor);
                totalGreen += (green * blendFactor);
                totalBlue  += (blue  * blendFactor);

            }

            // set our terrain's texture color to the one that we previously calculated
            texture_.SetColor(x, z, (uint8)totalRed, (uint8)totalGreen, (uint8)totalBlue);

        } // for by X
    } // for by Z

    LogMsg(LOG, "texture map is generated successfully");
    return true;
}

// ----------------------------------------------------
// Desc:   save the current texture map to a file
// Args:   - filename: name of the file to save to
// ----------------------------------------------------
bool TerrainBase::SaveTextureMap(const char* filename)
{
    if (StrHelper::IsEmpty(filename))
    {
        LogErr(LOG, "input texture map filename is empty!");
        return false;
    }

    // check if a texture is loaded, if so, save it
    if (texture_.IsLoaded())
    {
        return texture_.SaveBMP(filename);
    }
    // if we didn't load a texture from bmp/tga/raw we didn't initialize
    // this texture object, but we loaded it from dds and we have the texture data on
    // GPU side (if we really have such data we must have a valid texture ID),
    // so we just get texture by its ID, get data from GPU and store it into a file
    else if (texture_.GetID() != INVALID_TEX_ID)
    {
        LogErr(LOG, "I don't know how I got here :)");
        return false;
    }
    else
    {
        LogErr(LOG, "you want to save a texture map into file: %s\n\t%s", filename,
                    "but this image isn't initialized (has no data)");
        return false;
    }

    return true;
}

// --------------------------------------------------------
// Desc:   get the percentage of which a texture tile should
//         be visible at a given height
// Args:   - tileType: type of the tile to check
//         - height:   the current height to test for
// Ret:    a float values: the percentage of which the
//         current texture occupies at the give height
// --------------------------------------------------------
float TerrainBase::RegionPercent(const int tileType, const int height)
{
    const TerrainTextureRegions region = tiles_.regions[tileType];

    // height is higher than the region's boundary
    if (height < region.optimalHeight)       // lowHeight < height < optimalHeight
    {
        // calculate the texture percentage for the given tile's region
        const float temp1 = (float)(height - region.lowHeight);
        const float temp2 = (float)(region.optimalHeight - region.lowHeight);

        return temp1 / temp2;
    }

    // height is above the optimal height
    else if (height > region.optimalHeight)  // optimalHeight < height < highHeight
    {
        // calculate the texture percentage for the given tile's region
        const float temp = (float)(region.highHeight - region.optimalHeight);
        return (temp - (height - region.optimalHeight)) / temp;
    }

    // height is exactly the same as the optimal height
    else if (height == region.optimalHeight)
        return 1.0f;

    // something is seriously wrong if the height doesn't fit the previous cases
    LogErr(LOG, "wrong height: %d", height);
    return 0.0f;
}

// --------------------------------------------------------
// Desc:  get texture coords present in the final texture
//        (is used to get rid of the tile resolution depencency,
//         so our texture map can be bigger than the terrain's tiles)
// Args:  - tex: the texture to get coords for
//        - inOutX, inOutY: input unaltered texture coordinates
//          and storage place for the output altered coordinates
// --------------------------------------------------------
void TerrainBase::GetTexCoords(const Image& tex, uint& inOutX, uint& inOutY)
{
    const uint width  = tex.GetWidth();
    const uint height = tex.GetHeight();

    // define how many times the tile must repeat until the current coordinate
    const uint repeatX = (inOutX / width);
    const uint repeatY = (inOutY / height);

    // update the given texture map coordinates so we will be able to get
    // the correct coordinates from the input tile
    inOutX = inOutX - (width  * repeatX);
    inOutY = inOutY - (height * repeatY);
}

// --------------------------------------------------------
// Desc:   interpolate the height in the height map so that
//         the generated texture map doesn't look incredibly blocky
//         (is used to get rid of heightmap resolution depencency,
//          so our texture map may have any resolution)
// 
// Args:   - x, z: texture map coordinates to get the height at
//         - heightToTexRatio: height map size to texture map size ratio
//         - heightMapSize:    size (width and height) of the terrain's height map
// Ret:    uint8 value: the interpolated height
// --------------------------------------------------------
int TerrainBase::InterpolateHeight(
    const int x,
    const int z,
    const float heightToTexRatio,
    const float heightMapSize)
{
    float       interpolation = 0.0f;
    const float fScaledX       = x * heightToTexRatio;
    const float fScaledZ       = z * heightToTexRatio;
    const int   iScaledX       = (int)fScaledX;
    const int   iScaledZ       = (int)fScaledZ;

    const uint8 low = GetTrueHeightAtPoint(iScaledX,     iScaledZ);
     
    if ((fScaledX + 1) > heightMapSize || ((fScaledZ + 1) > heightMapSize))
        return low;

    const uint8 highX = GetTrueHeightAtPoint(iScaledX + 1, iScaledZ);
    const uint8 highZ = GetTrueHeightAtPoint(iScaledX, iScaledZ + 1);

    // calculate the interpolation (for the X axis)
    interpolation = (fScaledX - iScaledX);                    // remove integer part
    const float xCoord = ((highX - low) * interpolation) + low;  // interpolated height by X-axis

    // calculate the interpolation (for the Z axis)
    interpolation = (fScaledZ - iScaledZ);                    // remove integer part
    const float zCoord = ((highZ - low) * interpolation) + low;  // interpolated height by Z-axis

    // calculate the overall interpolation (average of the two values)
    return (int)((xCoord + zCoord) * 0.5f);
}


// =================================================================================
// Private methods
// =================================================================================

// --------------------------------------------------------
// Desc:     apply the erosion filter to an individual
//           band of height values
// Args:     - band:   the band to be filtered
//           - stride: how far to advance per pass
//           - count:  the number of passes to make
//           - filter: bluring/erosion strength (in range 0 to 1)
// --------------------------------------------------------
void TerrainBase::FilterHeightBand(
    float* band,          
    const int stride,     
    const int count,      
    const float filter)   
{
    float v = band[0];
    int j = stride;

    // go through the height band and apply the erosion filter
    for (int i = 0; i < count - 1; ++i)
    {
        band[j] = (filter * v) + (1 - filter) * band[j];

        v = band[j];
        j += stride;
    }
}

// --------------------------------------------------------
// Desc:  apply the erosion filter to an entire heights buffer
// Args:  - heightData: the height values to be filtered
//        - filter:     the filter strength
// --------------------------------------------------------
void TerrainBase::FilterHeightField(float* heightData, const float filter)
{
    const int size = heightMap_.GetWidth();

    // erode left to right
    for (int i = 0; i < size; ++i)
        FilterHeightBand(&heightData[size*i], 1, size, filter);

    // erode right to left
    for (int i = 0; i < size; ++i)
        FilterHeightBand(&heightData[size*i + size-1], -1, size, filter);

    // erode top to bottom
    for (int i = 0; i < size; ++i)
        FilterHeightBand(&heightData[i], size, size, filter);

    // erode from bottom to top
    for (int i = 0; i < size; ++i)
        FilterHeightBand(&heightData[size*(size-1)+i], -size, size, filter);
}

// --------------------------------------------------------
// Desc:  scale the terrain height values to a range of 0-255
// Args:  - heightData: the height data buffer
//        - size:       number of height values
// --------------------------------------------------------
void TerrainBase::NormalizeTerrain(float* heightData, const int size)
{
    float min = heightData[0];
    float max = heightData[0];

    // find the min/max values of the input height buffer
    for (int i = 1; i < size*size; ++i)
    {
        if (heightData[i] > max)
            max = heightData[i];
        else if (heightData[i] < min)
            min = heightData[i];
    }

    // find the range of the altitude
    if (max <= min)
        return;

    const float invHeight = 255.0f / (max - min);

    // scale the values to a range of 0-255
    for (int i = 0; i < size*size; ++i)
        heightData[i] = (heightData[i] - min) * invHeight;
}

//---------------------------------------------------------
// Desc:  load map which later will be used during generation of nature stuff
//        (trees, grass, etc.);
//        black values on the map mean less density, light values mean more density
//---------------------------------------------------------
bool TerrainBase::LoadNatureDensityMap(const char* filename)
{
    if (StrHelper::IsEmpty(filename))
    {
        LogErr(LOG, "input filename is empty");
        return false;
    }

    const char* extension = filename + strlen(filename) - 3;
    assert(strcmp(extension, "bmp") == 0);

    if (!natureDensityMap_.LoadGrayscaleBMP(filename))
    {
        LogErr(LOG, "can't load terrain's nature density BMP map: %s", filename);
        return false;
    }

    return true;
}

// --------------------------------------------------------
// Desc:   load a grayscale RAW/BMP light map
// Args:   - texName:  a name of the lightmap texture
// --------------------------------------------------------
bool TerrainBase::LoadLightMap(const char* texName)
{
    return true;
#if 0

    try
    {
        // check input params
        CAssert::True(!StrHelper::IsEmpty(filename), "input filename is empty");

        // check to see if the data has been set
        if (lightmap_.pData)
            UnloadLightMap();

        // get a file extension
        char extension[8];
        memset(extension, 0, sizeof(extension));
        FileSys::GetFileExt(filename, extension);

        // if we want to load in a light map from bmp-file
        if (strcmp(extension, ".bmp") == 0)
        {
            Image img;

            if (!img.LoadGrayscaleBMP(filename))
            {
                LogErr(LOG, "can't load lightmap from bmp file: %s", filename);
                return false;
            }

            const uint size = img.GetWidth();
            lightmap_.size = size;

            // alloc memory for lightmap data
            lightmap_.pData = NEW uint8[size*size];
            if (!lightmap_.pData)
            {
                LogErr(LOG, "can't allocate memory for lightmap data from file: %s", filename);
                return false;
            }

            memcpy(lightmap_.pData, img.GetData(), size*size);
        }

        // if we want to load in a light map from raw-file
        else if (strcmp(extension, ".raw") == 0)
        {
            // load in data
            int fileSize = 0;
            LoadRAW(filename, &lightmap_.pData, fileSize);

            // width == height
            lightmap_.size = (int)sqrtf((float)fileSize);
        }

        // if we want to load in a light map from tga-file
        else if (strcmp(extension, ".tga") == 0)
        {
            Image img;

            if (!img.LoadData(filename))
            {
                LogErr(LOG, "can't load lightmap from tga file: %s", filename);
                return false;
            }

            const uint size = img.GetWidth();
            lightmap_.size = size;

            // alloc memory for lightmap data
            lightmap_.pData = NEW uint8[size * size];
            if (!lightmap_.pData)
            {
                LogErr(LOG, "can't allocate memory for lightmap data from file: %s", filename);
                return false;
            }

            memcpy(lightmap_.pData, img.GetData(), size * size);
        }

        else
        {
            LogErr(LOG, "can't load light map: unsupported image format: %s", filename);
            return false;
        }
      

        // great success!
        LogMsg(LOG, "Loaded light map: %s", filename);
        return true;
    }
    catch (const std::bad_alloc& e)
    {
        sprintf(g_String, "can't allocate memory for the light map: %s", filename);
        LogErr(e.what());
        LogErr(g_String);
        throw EngineException(g_String);
    }
    catch (EngineException& e)
    {
        SafeDeleteArr(lightmap_.pData);
        LogErr(e);
        return false;
    }
#endif
}

// --------------------------------------------------------
// Desc:   save a grayscale light map to a file
// Args:   - filename: the filename of the light map
// --------------------------------------------------------
bool TerrainBase::SaveLightMap(const char* filename)
{
    // check input args
    if (StrHelper::IsEmpty(filename))
    {
        LogErr("input filename is empty!");
        return false;
    }

    const uint8* data   = lightmap_.pData;
    const int    width  = lightmap_.size;
    const int    height = lightmap_.size;

    // get extension
    char extension[8]{'\0'};
    FileSys::GetFileExt(filename, extension);

    // save to RAW
    if (strcmp(extension, ".raw") == 0)
    {
        return SaveRAW(filename, data, width*height);
    }
    // save to BMP
    else if (strcmp(extension, ".bmp") == 0)
    {
        return Image::SaveRawAsGrayBmp(filename, data, width, height);
    }
    else
    {
        LogErr("can't save lightmap into file: unsupported output format");
    }

    return false;
}

// --------------------------------------------------------
// Desc:   unload the class's light map (if there is one)
// --------------------------------------------------------
void TerrainBase::UnloadLightMap(void)
{
    SafeDeleteArr(lightmap_.pData);
    lightmap_.size = 0;
}

// --------------------------------------------------------
// Desc:   calculate the lighting value for any given point
// --------------------------------------------------------
uint8 TerrainBase::CalculateLightingAtPoint(const int x, const int z)
{
    if (lightingType_ == HEIGHT_BASED)
        return GetTrueHeightAtPoint(x, z);

    else if (lightingType_ == LIGHTMAP)
        return GetBrightnessAtPoint(x, z);

    return 0;
}

// --------------------------------------------------------
// Desc:   create a lightmap using height-based lighting
// Args:   - size: the size of lightmap
// --------------------------------------------------------
void TerrainBase::CalculateLightingHeightBased(const int size)
{
    // loop through all vertices
    for (int z = 0; z < size; ++z)
    {
        for (int x = 0; x < size; ++x)
        {
            SetBrightnessAtPoint(x, z, GetTrueHeightAtPoint(x, z));
        }
    }
}

// --------------------------------------------------------
// Desc:   create a lightmap using slope-based lighting
// Args:   - size:          the size of lightmap
//         - dirX:          the light direction by X-axis
//         - dirZ:          the light direction by Z-axis
//         - minBrightness: minimal brightness of the light
//         - maxBrightness: maximal brightness of the light
//         - softness:      the softness of the shadows
// --------------------------------------------------------
void TerrainBase::CalculateLightingSlope(
    const int size,
    const int dirX,
    const int dirZ,
    const float minBrightness,
    const float maxBrightness,
    const float softness)
{
    float shade = 0.0f;
    const float invLightSoftness = 1.0f / softness;


    // loop through all vertices
    for (int z = 0; z < size; ++z)
    {
        for (int x = 0; x < size; ++x)
        {
            // ensure that we won't be stepping over array boundaries by doing this
            if (z >= dirZ && x >= dirX)
            {
                shade = 1.0f - (GetTrueHeightAtPoint(x-dirX, z-dirZ) -
                                GetTrueHeightAtPoint(x, z)) * invLightSoftness;
            }

            // if we are, then just return a very bright color value (white)
            else
                shade = 1.0f;

            // clamp the shading value to the min/max brightness boundaries
            if (shade < minBrightness)
                shade = minBrightness;
            if (shade > maxBrightness)
                shade = maxBrightness;

            SetBrightnessAtPoint(x, z, (uint)(shade * 255));
        }
    }
}

// --------------------------------------------------------
// Desc:   calculates lighting for the pre-set technique,
//         and stores all computations in a lightmap
// --------------------------------------------------------
bool TerrainBase::CalculateLighting(void)
{
    // a lightmap has already been provided, no need to create another one
    if (lightingType_ == LIGHTMAP)
        return true;

    const int size = heightMap_.GetWidth();

    LogDbg(LOG, "wait while light map is generated");

    // check if we have valid terrain size
    if (size <= 0)
    {
        LogErr(LOG, "invalid terrain size (%d); must be > 0", size);
        return false;
    }

    // allocate memory if it is needed
    if ((lightmap_.size != size) || (lightmap_.pData == nullptr))
    {
        // delete the memory for the old data
        SafeDeleteArr(lightmap_.pData);

        // allocate memory for the new lightmap data buffer
        lightmap_.pData = NEW uint8[size*size];
        if (lightmap_.pData == nullptr)
        {
            LogErr(LOG, "can't allocate memory for terrain's lightmap");
            return false;
        }

        memset(lightmap_.pData, 0, size*size);

        // setup lightmap width and height
        lightmap_.size = size;
    }

    // use height-based lighting
    if (lightingType_ == HEIGHT_BASED)
        CalculateLightingHeightBased(size);

    // use the slope-lighting technique
    else if (lightingType_ == SLOPE_LIGHT)
        CalculateLightingSlope(
            size,
            directionX_,
            directionZ_,
            minBrightness_,
            maxBrightness_,
            lightSoftness_);

    LogMsg("light map is generated successfully");

    return true;
}

//--------------------------------------------------------------
// Desc:  get a value from nature density map at world point
//--------------------------------------------------------------
uint8 TerrainBase::GetNatureDensityAtPoint(const float x, const float z) const
{
    // convert world position into texel position
    assert(x >= 0.0f && x < terrainLength_);
    assert(z >= 0.0f && z < terrainLength_);

    const uint mapSize = natureDensityMap_.GetWidth();
    assert(mapSize == natureDensityMap_.GetHeight());

    const float ratio  = (float)mapSize / (float)terrainLength_;
    const int   texelX = (int)(x * ratio);
    const int   texelY = (int)(mapSize - z * ratio);

    return natureDensityMap_.GetPixelGray(texelX, texelY);
}

} // namespace
