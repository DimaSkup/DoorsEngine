// =================================================================================
// Filename:   TerrainBase.cpp
// =================================================================================
#include "TerrainBase.h"
#include <CoreCommon/MemHelpers.h>
#include <CoreCommon/StrHelper.h>
#include <CoreCommon/Assert.h>
#include <CoreCommon/Log.h>
#include <CoreCommon/MathHelper.h>
#include <CoreCommon/FileSystem.h>

#include "../Texture/TextureMgr.h"

#include <time.h>
#include <stdexcept>

namespace Core
{

// =================================================================================
// Public methods
// =================================================================================

// --------------------------------------------------------
// Desc:  load a file with terrain's settings
// Args:  - filename: path to the setup file
// --------------------------------------------------------
bool TerrainBase::LoadSetupFile(const char* filename, TerrainConfig& outConfigs)
{
    // check input params
    if (StrHelper::IsEmpty(filename))
    {
        sprintf(g_String, "terrain setup filename is empty: %s", filename);
        LogErr(g_String);
        return false;
    }

    // open the setup file
    FILE* pFile = fopen(filename, "r");
    if (!pFile)
    {
        sprintf(g_String, "can't open a terrain setup file: %s", filename);
        LogErr(g_String);
        return false;
    }

    constexpr int bufsize = 256;
    char buf[bufsize]{ '\0' };

    LogDbg("Start: read in the terrain setup file");

    // read in terrain common data
    fgets(buf, bufsize, pFile);
    sscanf(buf, "Path_terrain_height_map: %s", outConfigs.pathHeightMap);

    fgets(buf, bufsize, pFile);
    sscanf(buf, "Terrain_depth: %d", &outConfigs.depth);

    fgets(buf, bufsize, pFile);
    sscanf(buf, "Terrain_width: %d", &outConfigs.width);

    fgets(buf, bufsize, pFile);
    sscanf(buf, "Height_scaling: %f", &outConfigs.heightScale);

    // setup data fields
    terrainDepth_ = outConfigs.depth;
    terrainWidth_ = outConfigs.width;
    heightScale_  = outConfigs.heightScale;

    // do we want to generate terrain heights?
    int tempBool = 0;
    fgets(buf, bufsize, pFile);
    sscanf(buf, "Generate_heights: %d", &tempBool);
    outConfigs.generateHeights = (uint8)tempBool;

    fgets(buf, bufsize, pFile);
    sscanf(buf, "Use_gen_fault_formation: %d", &tempBool);
    outConfigs.useGenFaultFormation = (uint8)tempBool;

    fgets(buf, bufsize, pFile);
    sscanf(buf, "Use_gen_midpoint_displacement: %d", &tempBool);
    outConfigs.useGenMidpointDisplacement = (uint8)tempBool;


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
    sscanf(buf, "Path_save_generated_height_map: %s", outConfigs.pathSaveHeightMap);

    fgets(buf, bufsize, pFile);
    sscanf(buf, "Path_save_generated_texture_map: %s", outConfigs.pathSaveTextureMap);



    // close the setup file
    fclose(pFile);

    LogDbg("End: the terrain setup file is read in successfully!");

    return true;
}

// --------------------------------------------------------
// Desc:   load height map from bmp-file by filename
// Args:   - filename:  path to image
// --------------------------------------------------------
bool TerrainBase::LoadHeightMapFromBMP(const char* filename)
{
    FILE*            pFile = nullptr;
    size_t           retCode = 0;
    BITMAPFILEHEADER bitmapFileHeader;
    BITMAPINFOHEADER bitmapInfoHeader;
    uint8_t*         bitmapImage = nullptr;

    try
    {
        Assert::True(!StrHelper::IsEmpty(filename), "input filename str is empty");

        // check to see if the data has been set
        if (heightData_.pData)
            UnloadHeightMap();

        // open the bitmap map file in binary
        pFile = fopen(filename, "rb");
        if (!pFile)
        {
            sprintf(g_String, "can't open file for reading: %s", filename);
            throw EngineException(g_String);
        } 
   
        // read in the bitmap file header
        retCode = fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, pFile);
        if (retCode != 1)
        {
            sprintf(g_String, "error reading bitmap file header: %s", filename);
            throw EngineException(g_String);
        }

        // read in the bitmap info header
        retCode = fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, pFile);
        if (retCode != 1)
        {
            sprintf(g_String, "error reading bitmap info header: %s", filename);
            throw EngineException(g_String);
        }

        // assert that we load height data only for square height maps
        if (bitmapInfoHeader.biWidth != bitmapInfoHeader.biHeight)
        {
            sprintf(g_String, "wrong height map dimensions (width != height): %s", filename);
            throw EngineException(g_String);
        }

        // calculate the size of the bitmap image data;
        // since we use non-divide by 2 dimensions (eg. 257x257) we need to add an extra byte to each line
        uint bytesPerPixel   = bitmapInfoHeader.biBitCount / 8;
        const uint imageSize = bitmapInfoHeader.biSizeImage;
        
        //const int imageSize = size * ((size * bytesPerPixel) + 1);

        // alloc memory for the bitmap image data
        bitmapImage = new uint8_t[imageSize]{ 0 };

        // alloc the memory for the height map data
        const uint numHeightElems = imageSize / bytesPerPixel;   // == num pixels
        heightData_.pData = new uint8_t[numHeightElems]{ 0 };


        // move to the beginning of the bitmap data
        fseek(pFile, bitmapFileHeader.bfOffBits, SEEK_SET);

        // read in the bitmap image data
        retCode = fread(bitmapImage, 1, imageSize, pFile);
        if (retCode != imageSize)
        {
            sprintf(g_String, "can't read the bitmap image data: %s", filename);
            throw EngineException(g_String);
        }

        // close the file
        fclose(pFile);

        // initialize the position in the image data buffer
        int k = 0;
        int idx = 0;
        const int size = (int)bitmapInfoHeader.biWidth;

        // read the image data into the height map array
        for (int j = 0; j < size; ++j)
        {
            // bitmaps are upside down so load bottom to top into the height map array
            const int rowIdx = (size * (size - 1 - j));

            for (int i = 0; i < size; ++i)
            {
                // get the grey scale pixel value from the bitmap image data at this location
                heightData_.pData[rowIdx + i] = bitmapImage[k];

                // increment the bitmap image data idx (skip G and B channels)
                k += 3;
            }

            // compensate for the extra byte at end of each line in non-divide by 2 bitmaps (eg. 257x257)
            k++;
        }

        // release the bitmap img since the height map data has been loaded
        SafeDeleteArr(bitmapImage);

        // set the size data
        heightData_.size = size;
        heightMapSize_            = size;

        LogMsgf("Height map is loaded: %s", filename);
        return true;
    }
    catch (std::bad_alloc& e)
    {
        LogErr(e.what());
        sprintf(g_String, "couldn't alloc memory for height data / height map image: %s", filename);
        throw EngineException(g_String);
    }
    catch (EngineException& e)
    {
        if (pFile)
            fclose(pFile);

        SafeDeleteArr(heightData_.pData);
        SafeDeleteArr(bitmapImage);
        LogErr(e);
        return false;
    }
}

// --------------------------------------------------------
// Desc:   load a grayscale RAW height map
//         (it contains num elements and raw height data)
// Args:   - filename: path to the file
// --------------------------------------------------------
bool TerrainBase::LoadHeightMapFromRAW(const char* filename)
{
    FILE* pFile = nullptr;

    try
    {
        // check input params
        if (StrHelper::IsEmpty(filename))
        {
            LogErr("input filename is empty!");
            return false;
        }

        // open RAW file
        pFile = fopen(filename, "rb");
        if (!pFile)
        {
            sprintf(g_String, "can't open file: %s", filename);
            LogErr(g_String);
            return false;
        }

        // release previous data (if we have any)
        if (heightData_.pData)
            UnloadHeightMap();

        // read in the number of height elements
        u32 numElems = 0;
        size_t res = fread(&numElems, sizeof(u32), 1, pFile);
        if (res != 1)
        {
            sprintf(g_String, "can't read the number of elements from .raw file: %s", filename);
            throw EngineException(g_String);
        }

        // allocate the memory for our height data
        heightData_.pData = new uint8[numElems]{0};

        // read in the height map into context
        res = fread(heightData_.pData, 1, numElems, pFile);
        if (res != numElems)
        {
            sprintf(g_String, "can't read in height data from the file: %s", filename);
            throw EngineException(g_String);
        }

        // close the file
        fclose(pFile);

        // setup data fields
        heightMapSize_ = (int)sqrtf((float)numElems);    // set height map side size (map is a square)

        sprintf(g_String, "height map is loaded from the file: %s", filename);
        LogMsg(g_String);
        return true;
    }
    catch (std::bad_alloc& e)
    {
        LogErr(e.what());
        sprintf(g_String, "can't allocate memory for the height data from file: %s", filename);
        throw EngineException(g_String);
    }
    catch (EngineException& e)
    {
        if (pFile)
            fclose(pFile);

        SafeDeleteArr(heightData_.pData);
        LogErr(e);
        return false;
    }
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
            sprintf(g_String, "can't load height map from the file: %s", filename);
            LogErr(g_String);
            return false;
        }
    }
    // load from raw
    else if (strcmp(extension, ".raw") == 0)
    {
        if (!LoadHeightMapFromRAW(filename))
        {
            sprintf(g_String, "can't load height map from the file: %s", filename);
            LogErr(g_String);
            return false;
        }
    }

    return true;
}

// --------------------------------------------------------
// Desc:  save a grayscale RAW height map
// Args:  filename: the file name of the height map
// --------------------------------------------------------
bool TerrainBase::SaveHeightMap(const char* filename)
{
    // check input params
    if (StrHelper::IsEmpty(filename))
    {
        LogErr("input height map filename is empty");
        return false;
    }

    // check to see if our height map actually has data in it
    if (heightData_.pData == nullptr)
    {
        sprintf(g_String, "The height data buffer for %s is empty", filename);
        LogErr(g_String);
        return false;
    }

    // open a file to write to
    FILE* pFile = fopen(filename, "wb");
    if (!pFile)
    {
        sprintf(g_String, "Couldn't create a height map file: %s", filename);
        LogErr(g_String);
        return false;
    }

    // write data to the file
    const u32 numElems = (u32)(heightMapSize_ * heightMapSize_);
    fwrite(&numElems, sizeof(u32), 1, pFile);          // write the number of height elements
    fwrite(heightData_.pData, 1, numElems, pFile);

    // close the file
    fclose(pFile);

    sprintf(g_String, "The height map was saved successfully: %s", filename);
    LogMsg(g_String);
    return true;
}

// --------------------------------------------------------
// Desc:  release the memory from the heights data
//        and reset the map dimensions
// --------------------------------------------------------
void TerrainBase::UnloadHeightMap()
{
    SafeDeleteArr(heightData_.pData);   
    heightData_.size = 0;               
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

    try
    {
        // check input params
        Assert::True(size > 0,          "input size (dimensions) for height map must be > 0");
        Assert::True(numIterations > 0, "input number of iterations must be > 0");

        if (heightData_.pData)
            UnloadHeightMap();

        heightMapSize_ = size;

        // need it for truly random generation of heights
        if (trueRandom)
            srand((unsigned int)time(NULL));

        // alloc the memory for our height data
        heightData_.pData = new uint8_t[size * size]{ 0 };
        tempBuf           = new float[size * size]{ 0.0f };

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
                SetHeightAtPoint((uint8_t)tempBuf[(z * size) + x], x, z);
        }

        // delete temp buffer
        SafeDeleteArr(tempBuf);

        return true;
    }
    catch (std::bad_alloc& e)
    {
        LogErr(e.what());
        throw EngineException("can't allocate the memory for heights data");
    }
    catch (EngineException& e)
    {
        SafeDeleteArr(heightData_.pData);
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
    float* tempBuf = nullptr;    // temp buf for height data (we need it for higher precision)
    int    rectSize = size;

    try
    {
        // check input params
        Assert::True(size > 0,                   "input size of mipmap must be > 0");
        Assert::True(size && !(size & (size-1)), "input size must be a power of 2");

        // unload previous height data if we has any
        if (heightData_.pData)
            UnloadHeightMap();

        if (roughness < 0)
            roughness *= -1;

        float height = (float)(size / 2);                   
        const float heightReducer = powf(2, -1*roughness);

        heightMapSize_ = size;

        // need it for truly random generation of heights
        if (trueRandom)
            srand((unsigned int)time(NULL));

        // alloc the memory for our height data
        heightData_.pData = new uint8_t[size*size]{0};
        tempBuf           = new float[size*size]{0.0f};

        // being the displacement process
        while (rectSize > 0)
        {
            // min/max heights for the current iteration
            const float minHeight = -height * 0.5f;
            const float maxHeight = +height * 0.5f;

            const int halfRectSize = rectSize / 2;

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
                    tempBuf[idx] += MathHelper::RandF(minHeight, maxHeight);
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
                    tempBuf[idx1] += MathHelper::RandF(minHeight, maxHeight);

                    // calculate the square value for the left side of the rectangle
                    const int idx2 = i + (mj*size);

                    tempBuf[idx2] =
                        tempBuf[i   + (j*size)]   +
                        tempBuf[i   + (nj*size)]  +
                        tempBuf[pmi + (mj*size)]  +
                        tempBuf[mi  + (mj*size)];

                    tempBuf[idx2] *= 0.25f;
                    tempBuf[idx2] += MathHelper::RandF(minHeight, maxHeight);
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

        return true;
    }
    catch (std::bad_alloc& e)
    {
        LogErr(e.what());
        throw EngineException("can't allocate the memory for height data");
    }
    catch (EngineException& e)
    {
        SafeDeleteArr(heightData_.pData);
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
TexID TerrainBase::GenerateTextureMap(ID3D11Device* pDevice, const uint texMapSize)
{
    // check input params
    if (texMapSize <= 0)
    {
        LogErr("can't generate texture map: input size for texture map must be > 0");
        return INVALID_TEXTURE_ID;
    }

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
    const float mapRatio = (float)heightMapSize_ / texMapSize;    // for instance: 128 / 256

    // create the texture data
    for (int z = 0; z < (int)texMapSize; ++z)
    {
        for (int x = 0; x < (int)texMapSize; ++x)
        {
            // set our total color counters to 0.0f
            float totalRed        = 0.0f;
            float totalGreen      = 0.0f;
            float totalBlue       = 0.0f;
            const uint8 interpHeight = InterpolateHeight(x, z, mapRatio);

            // loop through the loaded tiles
            for (int i = 0; i < numTiles; ++i)
            {
                const int tileIdx = loadedTilesIdxs[i];
                const Image& tile = tiles_.textureTiles[tileIdx];

                uint texX = x;
                uint texZ = z;

                // get texture coordinates
                GetTexCoords(tile, texX, texZ);

                // get the curr color in the texture at the coordinates that we got in GetTexCoords
                uint8 red, green, blue;
                tile.GetColor(texX, texZ, red, green, blue);

                // get the curr coordinate's blending percentage for this tile
                const float blendFactor = RegionPercent(tileIdx, interpHeight);

                // calculate the RGB values that will be used
                totalRed   += (red * blendFactor);
                totalGreen += (green * blendFactor);
                totalBlue  += (blue * blendFactor);
            }

            // set our terrain's texture color to the one that we previously calculated
            texture_.SetColor(x, z, (uint8)totalRed, (uint8)totalGreen, (uint8)totalBlue);

        } // for by X
    } // for by Z


    // since we've generate 24 bits image we have to expand it up to 32 bits (add alpha channel)
    // to make possible creation of the DirectX texture
    const uint8* srcImage           = texture_.GetData();                          // 24 bits per pixel == 3 bytes per pixel
    const uint   dstBytesPerPixel   = 4;
    const uint   dstSizeInBytes     = texMapSize * texMapSize * dstBytesPerPixel;
    uint8*       dstImage           = new uint8[dstSizeInBytes]{0};   // 32 bits per pixel == 4 bytes per pixel

    // copy data from the generated 24bits texture map into the 32bits image buffer
    for (int i = 0; i < (int)(texMapSize * texMapSize); i++)
    {
        dstImage[i*4 + 0] = srcImage[i*3 + 0];      // R
        dstImage[i*4 + 1] = srcImage[i*3 + 1];      // G
        dstImage[i*4 + 2] = srcImage[i*3 + 2];      // B
        dstImage[i*4 + 3] = 255;                    // A (255 because we use uint8)
    }

    // create a DirectX texture
    Texture terrainTileMap(pDevice, "terrain_tile_map", dstImage, dstSizeInBytes);

    // move texture into the textures manager and get an ID of the texture
    const TexID texID = g_TextureMgr.Add("terrain_tile_map", std::move(terrainTileMap));

    // set the texture's ID
    texture_.SetID(texID);

    return texID;
}

// ----------------------------------------------------
// Desc:   save the current texture map to a file
// Args:   - filename: name of the file to save to
// ----------------------------------------------------
bool TerrainBase::SaveTextureMap(const char* filename)
{
    if (StrHelper::IsEmpty(filename))
    {
        LogErr("input texture map filename is empty!");
        return false;
    }

    // check if a texture is loaded, if so, save it
    if (texture_.IsLoaded())
        return texture_.SaveBMP(filename);
    else
        return false;
}

// --------------------------------------------------------
// Desc:   get the percentage of which a texture tile should
//         be visible at a given height
// Args:   - tileType: type of the tile to check
//         - height:   the current height to test for
// Ret:    a float values: the percentage of which the
//         current texture occupies at the give height
// --------------------------------------------------------
float TerrainBase::RegionPercent(const int tileType, const uint8 height)
{
    // if the height is lower than the lowest tile's height, then we want full brightness,
    // if we don't do this, the area will get darkened, and no texture will get shown
    if (tileType == LOWEST_TILE)
    {
        if (tiles_.textureTiles[LOWEST_TILE].IsLoaded() &&
            height < tiles_.regions[LOWEST_TILE].optimalHeight)
            return 1.0f;
    }

    const TerrainTextureRegions& region = tiles_.regions[tileType];

    // height is lower than the region's boundary
    if (height < region.lowHeight)
        return 0.0f;

    // height is higher than the region's boundary
    else if (height > region.highHeight)
        return 0.0f;

    // height is higher than the region's boundary
    if (height < region.optimalHeight)  // lowHeight < height < optimalHeight
    {
        // calculate the texture percentage for the given tile's region
        const float temp1 = (float)(height - region.lowHeight);
        const float temp2 = (float)region.optimalHeight - region.lowHeight;

        return temp1 / temp2;
    }

    // height is exactly the same as the optimal height
    else if (height == region.optimalHeight)
        return 1.0f;

    // height is above the optimal height
    else if (height > region.optimalHeight)  // optimalHeight < height < highHeight
    {
        // calculate the texture percentage for the given tile's region
        const float temp = (float)(region.highHeight - region.optimalHeight);
        return (temp - (height - region.optimalHeight)) / temp;
    }

    // something is seriously wrong if the height doesn't fit the previous cases
    sprintf(g_String, "wrong height: %d", height);
    LogErr(g_String);
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
    const int repeatX = (int)(inOutX / width);
    const int repeatY = (int)(inOutY / height);

    // update the given texture map coordinates so we will be able to get
    // the correct coordinates from the input tile
    inOutX = inOutX - (width * repeatX);
    inOutY = inOutY - (height * repeatY);
}

// --------------------------------------------------------
// Desc:   interpolate the height in the height map so that
//         the generated texture map doesn't look incredibly blocky
//         (is used to get rid of heightmap resolution depencency,
//          so our texture map may have any resolution)
// Args:   - x, z: texture map coordinates to get the height at
//         - heightToTexRatio: height map size to texture map size ratio
// Ret:    uint8 value: the interpolated height
// --------------------------------------------------------
uint8 TerrainBase::InterpolateHeight(
    const int x,
    const int z,
    const float heightToTexRatio)
{
    uint8       highX         = 0;
    uint8       highZ         = 0;
    float       interpolation = 0.0f;
    const float scaledX       = x * heightToTexRatio;
    const float scaledZ       = z * heightToTexRatio;
    const float heightMapSize = (float)heightMapSize_;

    // set the middle boundary
    const uint8 low = GetTrueHeightAtPoint((int)scaledX, (int)scaledZ);

    // interpolate along the X axis:
    // set the high boundary
    if ((scaledX + 1) > heightMapSize)
        return low;
    else
        highX = GetTrueHeightAtPoint((int)scaledX + 1, (int)scaledZ);

    // calculate the interpolation (for the X axis)
    interpolation = (scaledX - (int)scaledX);                    // remove integer part
    const float xCoord = ((highX - low) * interpolation) + low;  // interpolated height by X-axis

    // interpolate along the Z axis:
    // set the high boundary
    if ((scaledZ + 1) > heightMapSize)
        return low;
    else
        highZ = GetTrueHeightAtPoint((int)scaledX, (int)scaledZ + 1);

    // calculate the interpolation (for the Z axis)
    interpolation = (scaledZ - (int)scaledZ);                    // remove integer part
    const float zCoord = ((highZ - low) * interpolation) + low;  // interpolated height by Z-axis

    // calculate the overall interpolation (average of the two values)
    return (uint8)((xCoord + zCoord) * 0.5f);
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
    const int size = heightMapSize_;

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
    for (int i = 1; i < size * size; ++i)
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


} // namespace
