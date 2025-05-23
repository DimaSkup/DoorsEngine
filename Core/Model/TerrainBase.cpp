// =================================================================================
// Filename:   TerrainBase.cpp
// =================================================================================
#include "TerrainBase.h"
#include <CoreCommon/MemHelpers.h>
#include <CoreCommon/StrHelper.h>
#include <CoreCommon/Assert.h>
#include <CoreCommon/Log.h>
#include <CoreCommon/MathHelper.h>

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
bool TerrainBase::LoadSetupFile(const char* filename)
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

    constexpr int bufsize = 128;
    char buf[bufsize]{ '\0' };

    LogDbg("Start: read in the terrain setup file");

    // read in terrain common data
    fgets(buf, bufsize, pFile);
    sscanf(buf, "Terrain Filename: %s", terrainFilename_);

    fgets(buf, bufsize, pFile);
    sscanf(buf, "Terrain Depth: %d", &terrainDepth_);

    fgets(buf, bufsize, pFile);
    sscanf(buf, "Terrain Width: %d", &terrainWidth_);

    fgets(buf, bufsize, pFile);
    sscanf(buf, "Terrain Scaling: %f", &heightScale_);

    // close the setup file
    fclose(pFile);

    LogDbg("End: the terrain setup file is read in successfully!");

    return true;
}

// --------------------------------------------------------
// Desc:  load a height map from the file
// Args:  - filename: path to the file
//        - size:     dimension of the terrain by X and Z
//                    (expected to be the same as filename's)
// --------------------------------------------------------
bool TerrainBase::LoadHeightMap(const char* filename, const int size)
{
    FILE* pFile = nullptr;
    size_t retCode = 0;
    BITMAPFILEHEADER bitmapFileHeader;
    BITMAPINFOHEADER bitmapInfoHeader;
    uint8_t* bitmapImage = nullptr;

    try
    {
        Assert::True(!StrHelper::IsEmpty(filename), "input filename str is empty");
        Assert::True(size >= 0,                     "input size must be >= 0");

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

        // alloc the memory for the height map data
        const int numElems = size * size;
        heightData_.pData  = new uint8_t[numElems]{0};

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

        // make sure the height map dimensions are the same as the terrain dimensions
        // for each 1 to 1 mapping
        if ((bitmapInfoHeader.biHeight != size) || (bitmapInfoHeader.biWidth  != size))
        {
            sprintf(g_String, "wrong height map dimensions (expected to be %d)", size);
            throw EngineException(g_String);
        }

        // calculate the size of the bitmap image data;
        // since we use non-divide by 2 dimensions (eg. 257x257) we need to add an extra byte to each line
        const int imageSize = size * ((size * 3) + 1);

        // alloc memory for the bitmap image data
        bitmapImage = new uint8_t[imageSize]{ 0 };

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

        // read the image data into the height map array
        for (int j = 0; j < size; ++j)
        {
            for (int i = 0; i < size; ++i)
            {
                // bitmaps are upside down so load bottom to top into the height map array
                idx = (size * (size - 1 - j)) + i;

                // get the grey scale pixel value from the bitmap image data at this location
                heightData_.pData[idx] = bitmapImage[k];

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
        size_            = size;

        // print msg
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
// --------------------------------------------------------
bool TerrainBase::GenHeightFaultFormation(
    const int size,
    const int numIterations,
    const int minDelta,       
    const int maxDelta,    
    const float filter)  
{
    float* tempBuf = nullptr;   // we use this temp buffer because we need higher precision during computations

    try
    {
        // check input params
        Assert::True(size > 0,          "input size (dimensions) for height map must be > 0");
        Assert::True(numIterations > 0, "input number of iterations must be > 0");

        if (heightData_.pData)
            UnloadHeightMap();

        size_ = size;

        // need it for truly random generation of heights
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
// Args:  - size:      desired size of the height map
//        - roughness: desired roughness of the terrain
//                     (best results are from 0.25f to 1.5f)
// --------------------------------------------------------
bool TerrainBase::GenHeightMidpointDisplacement(const int size, float roughness)
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

        size_ = size;

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
    const int size = size_;

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

    const float invHeight = 1.0f / (max - min) * 255.0f;

    // scale the values to a range of 0-255
    for (int i = 0; i < size*size; ++i)
        heightData[i] = (heightData[i] - min) * invHeight;
}


} // namespace
