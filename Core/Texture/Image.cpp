// =================================================================================
// Filename:  Image.cpp
//
// Created:   24.05.2025  by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include <CoreCommon/RawFile.h>
#include "image.h"

#include <math.h>   // for abs()

#pragma warning (disable : 4996)


namespace Core
{

// swapping values with XOR (NOTE: works only for int values)
#define SWAP(a, b) ((a ^= b), (b ^= a), (a ^= b))

// --------------------------------------------------------
// Image's destructor
// --------------------------------------------------------
Image::~Image()
{
    Unload();
}

// --------------------------------------------------------
// Desc:   create space for use with a new texture
// Args:   - width, height: dimensions of the new image
//         - bpp:           bits per pixel
// --------------------------------------------------------
bool Image::Create(const uint width, const uint height, const uint bpp)
{
    try
    {
        Unload();

        // allocate memory
        pixels_ = new uint8[width * height * (bpp / 8)]{ 0 };

        // set the member variables
        width_         = width;
        height_        = height;
        bpp_           = bpp;
        bytesPerPixel_ = bpp / 8;
        isLoaded_      = true;

        return true;
    }
    catch (std::bad_alloc& e)
    {
        LogErr(e.what());
        LogErr("can't allocate memory for the empty image");
        return false;
    }
}

bool Create(const uint8* data, const uint width, const uint height, const uint bpp);


// --------------------------------------------------------
// Desc:   load only the data for a new image (do not
//         create a DirectX texture)
// Args:   filename: the file to load in
// --------------------------------------------------------
bool Image::LoadData(const char* filename)
{
    FILE* pFile = nullptr;

    try
    {
        // check input params
        if (!filename || filename[0] == '\0')
        {
            throw EngineException("input filename is empty!");
        }

        // save the filename
        size_t len = strlen(filename);
        len = (len > 64) ? 64 : len;
        strncpy(name_, filename, len);

#if 0
      
#endif

        // get file extension
        char extension[8]{'\0'};
        FileSys::GetFileExt(filename, extension);

        if (strcmp(extension, "invalid") == 0)
        {
            sprintf(g_String, "can't get extension of file: %s", filename);
            throw EngineException(g_String);
        }


        // check if we read BMP image
        if (strcmp(extension, ".bmp") == 0)
        {
            if (!LoadRgbBMP(filename))
            {
                sprintf(g_String, "can't load BMP file: %s", filename);
                throw EngineException(g_String);
            }
        }
        // check if we read TGA image
        else if (strcmp(extension, ".tga") == 0)
        {
            if (!LoadTGA(filename))
            {
                sprintf(g_String, "can't load TGA file: %s", filename);
                throw EngineException(g_String);
            }
        }
        // if we read RAW file
        else if (strcmp(extension, ".raw") == 0)
        {
            int fileSize = 0;

            if (LoadRAW(filename, &pixels_, fileSize))
            {
                // we expect square image which is grayscale with bpp == 8
                uint dimension = (uint)sqrtf((float)fileSize);
                width_  = dimension;
                height_ = dimension;
                bpp_    = 8;
            }
           
        }
        // the file is not supported by our image loader
        else
        {
            sprintf(g_String, "%s file has a NOT supported image type", filename);
            throw EngineException(g_String);
        }


        // the file's data was successfully loaded
        isLoaded_ = true;
        sprintf(g_String, "File is loaded correctly: %s", filename);
        LogMsg(g_String);
        return true;
    }
    catch (std::bad_alloc& e)
    {
        LogErr(e.what());
        sprintf(g_String, "can't allocate the memory for image data: %s", filename);
        throw EngineException(g_String);
    }
    catch (EngineException& e)
    {
        if (pFile)
            fclose(pFile);

        LogErr(e);
        SafeDeleteArr(pixels_);
        return false;
    }
}

// --------------------------------------------------------
// Desc:   completely setup a new texture, first load the data in,
//         and the setup the texture for use with DirectX. This
//         function supports both the BMP and TGA formats
// Args:   - filename: the file to load in
//         - minFilter / maxFilter:
//         - mipmapped: flag to define if we need to create mipmaps
// --------------------------------------------------------
bool Image::Load(const char* filename, const bool mipmapped)
{
    assert(0 && "TODO: do you need me?");

    return true;
}

// --------------------------------------------------------
// --------------------------------------------------------
bool Image::Save(const char* filename)
{
    // check input params
    if (StrHelper::IsEmpty(filename))
    {
        LogErr("input height map filename is empty");
        return false;
    }

    // check if we have any data to save
    if (!width_ || !height_ || !pixels_ || !isLoaded_)
    {
        sprintf(g_String, "can't save: image is empty or corrupted (for file: %s)", filename);
        LogErr(g_String);
        return false;
    }

    char extension[8]{'\0'};
    FileSys::GetFileExt(filename, extension);

    // store height map into RAW file
    if (strcmp(extension, ".raw") == 0)
        SaveRAW(filename, pixels_, width_*height_);

    // store height map into BMP file
    else if (strcmp(extension, ".bmp") == 0)
        SaveBMP(filename);


    sprintf(g_String, "The height map was saved successfully: %s", filename);
    LogMsg(g_String);

    return true;
}

// --------------------------------------------------------
// Desc:   save the current image info to a file.
//         this method only supports the BMP format
// Args:   - filename: the filename of the file to be saved
// --------------------------------------------------------
bool Image::SaveBMP(const char* filename) const
{
    BMPFileHeader   bitmapFileHeader;
    BMPInfoHeader   bitmapInfoHeader;

    constexpr uint  bytesPerPixel = 3;
    const uint      offset        = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
    const uint bytesInScanline = (width_ * bytesPerPixel);
    const int overFour = (bytesInScanline % 4);
    const int      paddingSize    = (overFour == 0) ? 0 : 4-overFour ;

    //uint imageSize = (width_* 24 + 7) / 8 * abs((int)height_);

    uint      imageSize     = bytesInScanline * height_ + (height_ * paddingSize);

    // compensate for the extra byte at end of each line in non-divide by 2 bitmaps (eg. 257x257)
    //imageSize += (height_ * paddingSize);

    // check input params
    if (!filename || filename[0] == '\0')
    {
        LogErr("input filename is empty!");
        return false;
    }

    // open a file that we can write to
    FILE* pFile = fopen(filename, "wb");
    if (!pFile)
    {
        sprintf(g_String, "can't open a file to save BMP image: %s", filename);
        LogErr(g_String);
        return false;
    }

    // define the BMP file header
    bitmapFileHeader.type            = 0x4D42;// BITMAP_ID;
    bitmapFileHeader.size            = offset + imageSize;
    bitmapFileHeader.reserved1       = 0;
    bitmapFileHeader.reserved2       = 0;
    bitmapFileHeader.offBits         = offset;
  
    // define the BMP info header
    bitmapInfoHeader.size            = sizeof(BMPInfoHeader);
    bitmapInfoHeader.width           = width_;
    bitmapInfoHeader.height          = height_;
    bitmapInfoHeader.planes          = 1;
    bitmapInfoHeader.bitCount        = 24;
    bitmapInfoHeader.compression     = BI_RGB;     // no compression
    bitmapInfoHeader.sizeImage       = imageSize;
    bitmapInfoHeader.xPixelsPerMeter = 0;
    bitmapInfoHeader.yPixelsPerMeter = 0;
    bitmapInfoHeader.colorUsed       = 0;
    bitmapInfoHeader.colorImportant  = 0;


    // copy pixels into temp buffer
    uint8* bmpPixels = new uint8[imageSize]{ 0 };


    // if the original image is grayscale
    if (bpp_ == 8)
    {
        const int width = width_;
        const int height = height_;

        // if we have any padding at the end of each image scanline
        // we must consider this padding when save the image
        if (paddingSize)
        {
            int k = 0;
            for (int j = 0; j < height_; ++j)
            {
                // bitmaps are upside down so load bottom to top into the array
                const int rowIdx = (height - 1 - j) * (bytesInScanline + paddingSize);

                for (int i = 0; i < width_; ++i)
                {
                    bmpPixels[rowIdx + i*3 + 0] = pixels_[k];
                    bmpPixels[rowIdx + i*3 + 1] = pixels_[k];
                    bmpPixels[rowIdx + i*3 + 2] = pixels_[k];
                    k++;
                }
            }
        }
        // no paddings: just copy the data
        else
        {
            for (int i = 0; i < (int)(width_ * height_); ++i)
            {
                bmpPixels[i*3 + 0] = pixels_[i];
                bmpPixels[i*3 + 1] = pixels_[i];
                bmpPixels[i*3 + 2] = pixels_[i];
            }
        }
    }

    // if original image is already 24 bits per pixel
    else if (bpp_ == 24)
    {
        memcpy(bmpPixels, pixels_, imageSize);

        // swap the image bit order (RGB to BGR)
        for (int i = 0; i < (int)imageSize; i += bytesPerPixel)
        {
            SWAP(bmpPixels[i], bmpPixels[i + 2]);
        }
    }

    // convert original 32 bits image into 24 bits image
    else
    {
        for (int i = 0; i < (int)(width_ * height_); ++i)
        {
            // rgb => bgr
            bmpPixels[i*3 + 0] = pixels_[i*4 + 2];
            bmpPixels[i*3 + 1] = pixels_[i*4 + 1];
            bmpPixels[i*3 + 2] = pixels_[i*4 + 0];
        }
    }


    // write the .bmp file header to the file
    fwrite(&bitmapFileHeader, 1, sizeof(BMPFileHeader), pFile);

    // write the .bmp file info header to the file
    fwrite(&bitmapInfoHeader, 1, sizeof(BMPInfoHeader), pFile);

    // now write the actual image data
    fwrite(bmpPixels, 1, imageSize, pFile);

    // the file has been successfully saved
    fclose(pFile);

    return true;
}

// --------------------------------------------------------
// Desc:   unload the texture that is currently loaded
// --------------------------------------------------------
void Image::Unload()
{
    if (isLoaded_)
    {
        SafeDeleteArr(pixels_);

        width_  = 0;
        height_ = 0;
        bpp_    = 0;

        isLoaded_ = false;
    }
}

//--------------------------------------------------------------
// Desc:   a windows RGB bitmap (BMP) image loader
// Args:   - filename: path to the file
//--------------------------------------------------------------
bool Image::LoadRgbBMP(const char* filename)
{
    FILE*            pFile = nullptr;
    size_t           retCode = 0;
    BMPFileHeader    fileHeader;
    BMPInfoHeader    infoHeader;
    uint8_t*         bitmapImage = nullptr;

    try
    {
        CAssert::True(!StrHelper::IsEmpty(filename), "input filename str is empty");

        // open the bitmap map file in binary
        pFile = fopen(filename, "rb");
        if (!pFile)
        {
            sprintf(g_String, "can't open file for reading: %s", filename);
            throw EngineException(g_String);
        }

        // read in the bitmap file header
        retCode = fread(&fileHeader, sizeof(BMPFileHeader), 1, pFile);
        if (retCode != 1)
        {
            sprintf(g_String, "error reading bitmap file header: %s", filename);
            throw EngineException(g_String);
        }

        // read in the bitmap info header
        retCode = fread(&infoHeader, sizeof(BMPInfoHeader), 1, pFile);
        if (retCode != 1)
        {
            sprintf(g_String, "error reading bitmap info header: %s", filename);
            throw EngineException(g_String);
        }

        // assert that we load height data only for square height maps
        if (infoHeader.width != infoHeader.height)
        {
            sprintf(g_String, "wrong height map dimensions (width != height): %s", filename);
            throw EngineException(g_String);
        }

        // allocate memory for empty image
        if (!Create(infoHeader.width, infoHeader.height, infoHeader.bitCount))
        {
            LogErr("can't create an empty space for bmp image");
            return false;
        }

        // calculate the size of the bitmap image data;
        const int imageSize = (int)infoHeader.sizeImage;

        // move to the beginning of the bitmap data
        fseek(pFile, fileHeader.offBits, SEEK_SET);

#if 0

        bitmapImage = new uint8[infoHeader.sizeImage]{ 0 };

        // read in the bitmap image data
        retCode = fread(bitmapImage, 1, imageSize, pFile);
        if (retCode != imageSize)
        {
            sprintf(g_String, "can't read the bitmap image data: %s", filename);
            throw EngineException(g_String);
        }


        // initialize the position in the image data buffer
        int k   = 0;
        int idx = 0;
        const int size  = (int)infoHeader.width;
        const int isOdd = (size % 2 != 0);

        // read the image data into the height map array
        for (int j = 0; j < size; ++j)
        {
            // bitmaps are upside down so load bottom to top into the array
            const int rowIdx = (size * (size - 1 - j));

            for (int i = 0; i < size; ++i)
            {
                const int off = rowIdx + i*3;

                pixels_[off + 0] = bitmapImage[k + 2];
                pixels_[off + 1] = bitmapImage[k + 1];
                pixels_[off + 2] = bitmapImage[k + 0];

                // increment the bitmap image data idx (skip G and B channels)
                k += 3;
            }
        }
#else
        // read in the bitmap image data
        retCode = fread(pixels_, 1, imageSize, pFile);
        if (retCode != imageSize)
        {
            sprintf(g_String, "can't read the bitmap image data: %s", filename);
            throw EngineException(g_String);
        }

        // swap the R and B values to get RGB since the bitmap color format is in BGR
        for (int i = 0; i < (int)infoHeader.sizeImage; i += 3)
        {
            // swap values without using extra space for a temp variable
            SWAP(pixels_[i], pixels_[i + 2]);
        }
#endif

        // close the file
        fclose(pFile);

        // release temp buffer for image data
        SafeDeleteArr(bitmapImage);

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

        SafeDeleteArr(bitmapImage);
        SafeDeleteArr(pixels_);
        LogErr(e);
        return false;
    }









#if 0
    BMPFileHeader* pFileHeader = nullptr;
    BMPInfoHeader* pInfoHeader = nullptr;
    uint8*         pData = pixels_;

    // load in the file header
    pFileHeader = (BMPFileHeader*)pData;

    // advance the buffer, and load in the file info header
    pData += sizeof(BMPFileHeader);
    pInfoHeader = (BMPInfoHeader*)pData;

    // advance the buffer to load in the actual image data
    pData += sizeof(BMPInfoHeader);
    pData += pFileHeader->offBits;

    // init the image memory
    if (!Create(pInfoHeader->width, pInfoHeader->height, pInfoHeader->bitCount))
    {
        LogErr("can't create an empty space for bmp image");
        return false;
    }

    // copy the data to the class's data buffer
    const uint imgSize = pFileHeader->size - pFileHeader->offBits;
    memcpy(pixels_, pData, imgSize);

    // swap the R and B values to get RGB since the bitmap color format is in BGR
    for (int i = 0; i < (int)pInfoHeader->sizeImage; i += 3)
    {
        // swap values without using extra space for a temp variable
        SWAP(pixels_[i], pixels_[i + 2]);
    }

    // the BMP has been successfully loaded
    return true;
#endif
}

//---------------------------------------------------------
//---------------------------------------------------------
bool Image::LoadGrayscaleBMP(const char* filename)
{
    FILE* pFile = nullptr;
    size_t           retCode = 0;
    BMPFileHeader    fileHeader;
    BMPInfoHeader    infoHeader;
    uint8_t* bitmapImage = nullptr;

    try
    {
        CAssert::True(!StrHelper::IsEmpty(filename), "input filename str is empty");

        // open the bitmap map file in binary
        pFile = fopen(filename, "rb");
        if (!pFile)
        {
            sprintf(g_String, "can't open file for reading: %s", filename);
            throw EngineException(g_String);
        }

        // read in the bitmap file header
        retCode = fread(&fileHeader, sizeof(BMPFileHeader), 1, pFile);
        if (retCode != 1)
        {
            sprintf(g_String, "error reading bitmap file header: %s", filename);
            throw EngineException(g_String);
        }

        // read in the bitmap info header
        retCode = fread(&infoHeader, sizeof(BMPInfoHeader), 1, pFile);
        if (retCode != 1)
        {
            sprintf(g_String, "error reading bitmap info header: %s", filename);
            throw EngineException(g_String);
        }

        // assert that we load height data only for square height maps
        if (infoHeader.width != infoHeader.height)
        {
            sprintf(g_String, "wrong height map dimensions (width != height): %s", filename);
            throw EngineException(g_String);
        }

        // allocate memory for empty image
        if (!Create(infoHeader.width, infoHeader.height, 8))
        {
            LogErr("can't create an empty space for bmp image");
            return false;
        }

        // move to the beginning of the bitmap data
        fseek(pFile, fileHeader.offBits, SEEK_SET);

        // calculate the size of the bitmap image data;
        const int imageSize = (int)infoHeader.sizeImage;
      
        // allocate memory for image temp buffer
        bitmapImage = new uint8[infoHeader.sizeImage]{ 0 };

        // read in the bitmap image data
        retCode = fread(bitmapImage, 1, infoHeader.sizeImage, pFile);
        if (retCode != infoHeader.sizeImage)
        {
            sprintf(g_String, "can't read the bitmap image data: %s", filename);
            throw EngineException(g_String);
        }

        // initialize the position in the image data buffer
        int k = 0;
        int idx = 0;
        const int size = (int)infoHeader.width;
        const int isOdd = (size % 2 != 0);

        const int width  = infoHeader.width;
        const int height = infoHeader.height;

        // read the image data into the height map array
        for (int j = 0; j < height; ++j)
        {
            // bitmaps are upside down so load bottom to top into the array
            //const int rowIdx = (width * (height - 1 - j));

            // from top to bottom
            const int rowIdx = j * width;

            for (int i = 0; i < width; ++i)
            {
                // get the grey scale pixel value from the bitmap image data at this location
                pixels_[rowIdx + i] = bitmapImage[k];

                // increment the bitmap image data idx (skip G and B channels)
                k += 3;
            }

            // compensate for the extra byte at end of each line in non-divide by 2 bitmaps (eg. 257x257)
            k += isOdd;
        }

        // release temp buffer
        SafeDeleteArr(bitmapImage);

        // close the file
        fclose(pFile);

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

        SafeDeleteArr(pixels_);
        LogErr(e);
        return false;
    }
}

//--------------------------------------------------------------
// Desc:   targa (TGA) image loader
//--------------------------------------------------------------
bool Image::LoadTGA(const char* filename)
{
    TGAInfoHeader tgaInfo;
    FILE*         pFile = nullptr;
    int           fileSize = 0;

    if (!filename || filename[0] == '\0')
    {
        LogErr("input filename is empty");
        return false;
    }

    // open the file for reading (in binary mode)
    pFile = fopen(filename, "rb");
    if (!pFile)
    {
        sprintf(g_String, "can't open file: %s", filename);
        throw EngineException(g_String);
    }

    // get file length
    fseek(pFile, 0, SEEK_END);
    fileSize = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);


    pixels_ = new uint8[fileSize]{0};

    if (fread(pixels_, 1, fileSize, pFile) != (unsigned)fileSize)
    {
        sprintf(g_String, "can't read in pixels data: %s", filename);
        LogErr(g_String);
        SafeDeleteArr(pixels_);
        fclose(pFile);

        return false;
    }

    uint8* fileData = pixels_;

    // skip header
    //fileData += 12;

    const int headerSize = sizeof(TGAInfoHeader);

    memcpy(&tgaInfo, fileData, headerSize);

    const uint width = (uint)tgaInfo.width;
    const uint height = (uint)tgaInfo.height;
    const uint bpp = (uint)tgaInfo.bitsPerPixel;

    Create(width, height, bpp);

    fileData += 18;

    uint imageSize = width*height*(bpp/8);

    memcpy(pixels_, fileData, imageSize);

#if 0

    // read in the image header
    fread(&tgaInfo, sizeof(tgaInfo), 1, pFile);

    const uint width  = (uint)tgaInfo.width;
    const uint height = (uint)tgaInfo.height;
    const uint bpp    = (uint)tgaInfo.bitsPerPixel;

    // make sure that the image's dimensions are supported by this loader
    if ((width <= 0) || (height <= 0) || ((bpp != 24) && (bpp != 32)))
    {
        sprintf(g_String, "TGA image is not supported: %s", name_);
        LogErr(g_String);
        return false;
    }

    // create empty space for the image
    if (!Create(width, height, bpp))
    {
        sprintf(g_String, "can't create an empty space for tga image: %s", name_);
        LogErr(g_String);
        return false;
    }

    fseek(pFile, sizeof(tgaInfo), SEEK_SET);

    // calculate the size of the targa image in bytes
    const int imageSize = width * height * (bpp / 8);

    // allocate memory for the targa image temp buffer
    uint8* pixelsData = new uint8[imageSize]{ 0 };

    // read in pixels data
    const size_t res = fread(pixelsData, 1, imageSize, pFile);
    if (res != (unsigned)imageSize)
    {
        SafeDeleteArr(pixelsData);
        sprintf(g_String, "%s is corrupted, could not read all data", filename);
        throw EngineException(g_String);
    }
#endif

    // close the file
    fclose(pFile);

    // define if tga image is compressed or uncompressed
    switch (tgaInfo.imageType)
    {
        case UncompressedRgb:
        {
            LoadUncompressedTGA(tgaInfo, fileData);
            return true;
        }
        case RleRgb:
        {
            LoadCompressedTGA(tgaInfo, fileData);
            return true;
        }
        default:
        {
            sprintf(g_String, "unsupported TGA image type: %d", tgaInfo.imageType);
            LogErr(g_String);
            return false;
        }
    } // switch

    // release the temp data buffer
    //SafeDeleteArr(pixelsData);
}

//--------------------------------------------------------------
// Desc:   read in compressed TGA image data with 24 bits per pixel
// Args:   - data:   pixels raw data
//         - header: tga image header
//--------------------------------------------------------------
void Image::LoadCompressedTGA24(uint8*& data, const TGAInfoHeader& header)
{
    constexpr uint bytesPerPixel = 3;
    uint           currByte = 0;
    uint8          colorBuf[bytesPerPixel]{ 0 };


    for (int y = 0; y < header.height; ++y)
    {
        for (int x = 0; x < header.width;)
        {
            uint8 c = *(uint*)data;
            data++;

            if (c & 0x80)
            {
                c = (c & 0x7f) + 1;
                x += c;

                // read in BGR pixel
                memcpy(colorBuf, data, bytesPerPixel);
                data += bytesPerPixel;

                // fill with the same color (and store BGR as RGB)
                for (int i = 0; i < c; ++i)
                {
                    pixels_[currByte + 0] = colorBuf[2];
                    pixels_[currByte + 1] = colorBuf[1];
                    pixels_[currByte + 2] = colorBuf[0];

                    currByte += bytesPerPixel;
                }
            }
            else
            {
                ++c;
                x += c;

                for (int i = 0; i < c; ++i)
                {
                    // read in BGR pixel
                    memcpy(colorBuf, data, bytesPerPixel);
                    data += bytesPerPixel;

                    // store BGR as RGB
                    pixels_[currByte + 0] = colorBuf[2];
                    pixels_[currByte + 1] = colorBuf[1];
                    pixels_[currByte + 2] = colorBuf[0];

                    currByte += bytesPerPixel;
                }
            }
        }
    }
}

//--------------------------------------------------------------
// Desc:   read in compressed TGA image data with 32 bits per pixel
// Args:   - data:   pixels raw data
//         - header: tga image header
//--------------------------------------------------------------
void Image::LoadCompressedTGA32(uint8*& data, const TGAInfoHeader& header)
{
    constexpr uint bytesPerPixel = 4;
    uint           currByte = 0;
    uint8          colorBuf[bytesPerPixel]{ 0 };

    for (int y = 0; y < header.height; ++y)
    {
        for (int x = 0; x < header.width;)
        {
            uint8 c = *(uint*)data;
            data++;

            if (c & 0x80)
            {
                c = (c & 0x7f) + 1;
                x += c;

                // read in BGRA pixel
                memcpy(colorBuf, data, bytesPerPixel);
                data += bytesPerPixel;

                // fill with the same color
                for (int i = 0; i < c; ++i)
                {
                    // store BGRA as RGBA
                    pixels_[currByte + 0] = colorBuf[2];
                    pixels_[currByte + 1] = colorBuf[1];
                    pixels_[currByte + 2] = colorBuf[0];
                    pixels_[currByte + 3] = colorBuf[3];

                    currByte += bytesPerPixel;
                }
            }
            else
            {
                ++c;
                x += c;

                for (int i = 0; i < c; ++i)
                {
                    // read in BGRA pixel
                    memcpy(colorBuf, data, bytesPerPixel);
                    data += bytesPerPixel;

                    // store BGRA as RGBA
                    pixels_[currByte + 0] = colorBuf[2];
                    pixels_[currByte + 1] = colorBuf[1];
                    pixels_[currByte + 2] = colorBuf[0];
                    pixels_[currByte + 3] = colorBuf[3];

                    currByte += bytesPerPixel;
                }
            }
        }
    }
}

//--------------------------------------------------------------
// Desc:   private HELPER to load a compressed targe (TGA) image
// Args:   
//--------------------------------------------------------------
void Image::LoadCompressedTGA(const TGAInfoHeader& header, uint8*& pixelsData)
{
    // if we want to read in RGB image
    if (header.BytesPerPixel() == 3)
    {
        LoadCompressedTGA24(pixelsData, header);
    }
    // we want to read in RGBA image
    else
    {
        LoadCompressedTGA32(pixelsData, header);
    }
}

// --------------------------------------------------------
// Desc:   private HELPER load an uncompressed targa (TGA) image
// --------------------------------------------------------
void Image::LoadUncompressedTGA(const TGAInfoHeader& header, uint8*& pixelsData)
{
    const uint numPixels      = header.width * header.height;
    const uint bytesPerPixel  = header.BytesPerPixel();
    const uint imgSizeInBytes = numPixels * bytesPerPixel;

    // copy the image data
    memcpy(pixels_, pixelsData, imgSizeInBytes);

    // byte swapping
    for (int idx = 0; idx < (int)imgSizeInBytes; idx += bytesPerPixel)
    {
        SWAP(pixels_[idx], pixels_[idx+2]);
    }
  
}

} // namespace Core
