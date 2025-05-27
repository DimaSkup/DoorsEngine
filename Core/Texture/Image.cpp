// =================================================================================
// Filename:  Image.cpp
//
// Created:   24.05.2025  by DimaSkup
// =================================================================================
#include "image.h"
#include <CoreCommon/log.h>
#include <CoreCommon/MemHelpers.h>
#include <stdexcept>
#include <stdio.h>

using namespace Core;

// swapping values with XOR (NOTE: works only for int values)
#define SWAP(a, b) ((a ^= b), (b ^= a), (a ^= b))


// ========================================================
// Globals
// ========================================================
uint8 g_UTGAcompare[12] = { 0, 0, 2,  0, 0, 0, 0, 0, 0, 0, 0, 0 };
uint8 g_CTGAcompare[12] = { 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


// --------------------------------------------------------
// Desc:   create space for use with a new texture
// Args:   - width, height: the dimensions of the new image
//         - bpp:           the bits per pixel
// --------------------------------------------------------
bool Image::Create(const uint width, const uint height, const uint bpp)
{
    try
    {
        // allocate memory
        pData_ = new uint8[width * height * (bpp / 8)]{ 0 };

        // set the member variables
        width_    = width;
        height_   = height;
        bpp_      = bpp;
        isLoaded_ = true;

        return true;
    }
    catch (std::bad_alloc& e)
    {
        LogErr(e.what());
        LogErr("can't allocate memory for the empty image");
        return false;
    }
}

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

        // open the file for reading (in binary mode)
        pFile = fopen(filename, "rb");
        if (!pFile)
        {
            sprintf(g_String, "can't open file: %s", filename);
            throw EngineException(g_String);
        }

        // get file length
        fseek(pFile, 0, SEEK_END);
        const int end = ftell(pFile);
        fseek(pFile, 0, SEEK_SET);

        const int start = ftell(pFile);
        const int size = end - start;

        // release memory from the previous image data (if we have any)
        Unload();

        // allocate the data buffer
        pData_ = new uint8[size]{ 0 };

        // read the image's data into the buffer
        if (fread(pData_, sizeof(uint8), size, pFile) != (unsigned)size)
        {
            // the file is corrupted
            sprintf(g_String, "%s is corrupted, could not read all data", filename);
            throw EngineException(g_String);
        }

        // check to see if the file is in the BMP format
        if (memcmp(pData_, "BM", 2) == 0)
        {
            // load the BMP using the BMP-loading method
            if (!LoadBMP())
            {
                sprintf(g_String, "Couldn't load BMP file: %s", filename);
                throw EngineException(g_String);
            }
        }

        // check to see if the file is an uncompressed TGA
        else if (memcmp(pData_, g_UTGAcompare, 12) == 0)
        {
            // load the file using the uncompressed TGA loader
            if (!LoadUncompressedTGA())
            {
                sprintf(g_String, "Couldn't load uncompressed TGA file: %s", filename);
                throw EngineException(g_String);
            }
        }

        // check to see if the file is a compressed TGA
        else if (memcmp(pData_, g_CTGAcompare, 12) == 0)
        {
            // load the file using the compressed TGA loader
            if (!LoadCompressedTGA())
            {
                sprintf(g_String, "Couldn't load compressed TGA file: %s", filename);
                throw EngineException(g_String);
            }
        }

        // the file is not supported by our image loader
        else
        {
            sprintf(g_String, "%s file is not a supported image type", filename);
            throw EngineException(g_String);
        }

        // the file's data was successfully loaded
        isLoaded_ = true;
        sprintf(g_String, "FS: File %s is loaded correctly", filename);
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
        SafeDeleteArr(pData_);
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
    // TODO: ???

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
    uint8              tempRGB = 0;

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

    uint fileHeaderSz = sizeof(BMPFileHeader);
    uint infoHeaderSz = sizeof(BMPInfoHeader);
    const uint headersSize = fileHeaderSz + infoHeaderSz;
    const uint imageSize = width_ * height_ * 3;
    

    // define the BMP file header
    bitmapFileHeader.type = 0x4D42;// BITMAP_ID;
    bitmapFileHeader.size            = headersSize + imageSize;
    bitmapFileHeader.reserved1       = 0;
    bitmapFileHeader.reserved2       = 0;
    bitmapFileHeader.offBits         = headersSize;

    // write the .bmp file header to the file
    fwrite(&bitmapFileHeader, 1, sizeof(BMPFileHeader), pFile);

    // define the BMP info header
    bitmapInfoHeader.size            = sizeof(BMPInfoHeader);
    bitmapInfoHeader.planes          = 1;
    bitmapInfoHeader.bitCount        = 24;
    bitmapInfoHeader.compression     = BI_RGB;
    bitmapInfoHeader.sizeImage       = imageSize;
    bitmapInfoHeader.xPixelsPerMeter = 0;
    bitmapInfoHeader.yPixelsPerMeter = 0;
    bitmapInfoHeader.colorUsed       = 0;
    bitmapInfoHeader.colorImportant  = 0;
    bitmapInfoHeader.width           = width_;
    bitmapInfoHeader.height          = height_;

    // write the .bmp file info header to the file
    fwrite(&bitmapInfoHeader, 1, sizeof(BMPInfoHeader), pFile);

    // swap the image bit order (RGB to BGR)
    for (int i = 0; i < (int)imageSize; i += 3)
    {
        tempRGB     = pData_[i];
        pData_[i]   = pData_[i+2];
        pData_[i+2] = tempRGB;

        // swap values without using extra space for a temp variable
        //SWAP(pData_[i], pData_[i+2]);
    }

    // now write the actual image data
    fwrite(pData_, 1, imageSize, pFile);

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
        SafeDeleteArr(pData_);

        width_  = 0;
        height_ = 0;
        bpp_    = 0;

        isLoaded_ = false;
    }
}

//--------------------------------------------------------------
// Desc:   a windows bitmap (BMP) image loader
//--------------------------------------------------------------
bool Image::LoadBMP()
{
    uint8* pData = pData_;

    // load in the file header
    const BMPFileHeader* pFileHeader = (BMPFileHeader*)pData;

    // advance the buffer, and load in the file info header
    pData += sizeof(BMPFileHeader);
    const BMPInfoHeader* pInfoHeader = (BMPInfoHeader*)pData;

    // advance the buffer to load in the actual image data
    pData += sizeof(BMPInfoHeader);
    pData += pFileHeader->offBits;

    // init the image memory
    const uint imgSize = pFileHeader->size - pFileHeader->offBits;

    if (!Create(pInfoHeader->width, pInfoHeader->height, pInfoHeader->bitCount))
    {
        LogErr("can't create an empty space for bmp image");
        return false;
    }

    // copy the data to the class's data buffer
    memcpy(pData_, pData, imgSize);

    // swap the R and B values to get RGB since the bitmap color format is in BGR
    for (int i = 0; i < (int)pInfoHeader->sizeImage; i += 3)
    {
        // swap values without using extra space for a temp variable
        SWAP(pData_[i], pData_[i + 2]);
    }

    // the BMP has been successfully loaded
    return true;
}

//--------------------------------------------------------------
// Desc:   private HELPER to load a compressed targe (TGA) image
//--------------------------------------------------------------
bool Image::LoadCompressedTGA()
{
    TGAInfoHeader tgaInfo;
    uint8* pFileData = pData_;
    
    try
    {
        // skip the image header
        pFileData += 12;
        memcpy(tgaInfo.header, pFileData, sizeof(uint8[6]));

        // allocate memory for the image data buffer
        const uint width  = tgaInfo.header[1] * 256 + tgaInfo.header[0];
        const uint height = tgaInfo.header[3] * 256 + tgaInfo.header[2];
        const uint bpp    = tgaInfo.header[4];

        if (!Create(width, height, bpp))
        {
            LogErr("can't create an empty space for uncompressed tga image");
            return false;
        }

        // make sure that the image's dimensions are supported by this loader
        if ((width_ <= 0) || (height_ <= 0) || ((bpp_ != 24) && (bpp_ != 32)))
            throw EngineException("image's params are not supported");

        // set the class's member variables
        tgaInfo.bpp    = bpp_;
        tgaInfo.height = height_;
        tgaInfo.width  = width_;

        // advance the file buffer ptr
        pFileData += 6;

        const uint numPixels  = tgaInfo.width * tgaInfo.height;
        tgaInfo.bytesPerPixel = tgaInfo.bpp / 8;
        tgaInfo.imageSize     = (tgaInfo.bytesPerPixel * numPixels);

        uint currPixel   = 0;
        uint currByte    = 0;
        uint8   colorBuf[4]{0};   // 3 bytes for 24 bits per pixel, 4 bytes for 32 bits


        // loop while there are still pixels left
        while (currPixel < numPixels)
        {
            uint8 chunkHeader = *(uint8*)pFileData;
            pFileData++;

            
            if (chunkHeader < 128)
            {
                chunkHeader++;

                // read RAW color values
                for (int i = 0; i < chunkHeader; ++i)
                {
                    memcpy(colorBuf, pFileData, tgaInfo.bytesPerPixel);
                    pFileData += tgaInfo.bytesPerPixel;

                    // flip R and B color values around
                    pData_[currByte + 0] = colorBuf[2];
                    pData_[currByte + 1] = colorBuf[1];
                    pData_[currByte + 2] = colorBuf[0];

                    // TODO: optimize it
                    if (tgaInfo.bytesPerPixel == 4)
                        pData_[currByte + 3] = colorBuf[3];

                    currByte += tgaInfo.bytesPerPixel;
                    currPixel++;

                    // make sure too many pixels have not been read in
                    if (currPixel > numPixels)
                        throw EngineException("pixels count overflow");
                } // for
            }

            // the chunk header is greater than 128 RLE data
            else
            {
                chunkHeader -= 127;

                memcpy(colorBuf, pFileData, tgaInfo.bytesPerPixel);
                pFileData += tgaInfo.bytesPerPixel;

                // copy the colors for image with 3 bytes per pixel
                if (tgaInfo.bytesPerPixel == 3)
                {
                    // copy the color into the image data as many times as needed
                    for (int i = 0; i < chunkHeader; ++i)
                    {
                        // switch R and B bytes around while copying
                        pData_[currByte + 0] = colorBuf[2];
                        pData_[currByte + 1] = colorBuf[1];
                        pData_[currByte + 2] = colorBuf[0];

                        currByte += tgaInfo.bytesPerPixel;
                        currPixel++;

                        // make sure that we have not written to many pixels
                        if (currPixel > numPixels)
                            throw EngineException("pixels count overflow");
                    }
                }

                // copy the colors for image with 4 bytes per pixel
                else
                {
                    // copy the color into the image data as many times as needed
                    for (int i = 0; i < chunkHeader; ++i)
                    {
                        // switch R and B bytes around while copying
                        pData_[currByte + 0] = colorBuf[2];
                        pData_[currByte + 1] = colorBuf[1];
                        pData_[currByte + 2] = colorBuf[0];
                        pData_[currByte + 3] = colorBuf[3];

                        currByte += tgaInfo.bytesPerPixel;
                        currPixel++;

                        // make sure that we have not written to many pixels
                        if (currPixel > numPixels)
                            throw EngineException("pixels count overflow");
                    }

                } // else
            } // else
        } // while

        // the compressed TGA has been successfully loaded
        return true;
    }
    catch (std::bad_alloc& e)
    {
        LogErr(e.what());
        throw EngineException("can't allocate memory for loading compressed TGA image");
    }
    catch (EngineException& e)
    {
        LogErr(e);
        SafeDeleteArr(pData_);
        return false;
    }
}

// --------------------------------------------------------
// Desc:   private HELPER load an uncompressed targa (TGA) image
// --------------------------------------------------------
bool Image::LoadUncompressedTGA()
{
    TGAInfoHeader tgaInfo;
    uint8* pData = pData_;

    // skip the TGA header in the data buffer
    pData += 12;

    // copy the header data
    memcpy(tgaInfo.header, pData, sizeof(uint8[6]));

    // allocate memory for the image's data buffer
    const uint width  = (tgaInfo.header[1] * 256) + tgaInfo.header[0];
    const uint height = (tgaInfo.header[3] * 256) + tgaInfo.header[2];
    const uint bpp    = (tgaInfo.header[4]);

    if (!Create(width, height, bpp))
    {
        LogErr("can't create an empty space for uncompressed tga image");
        return false;
    }

    // set the class's member variables
    tgaInfo.bpp     = bpp_;
    tgaInfo.width   = width_;
    tgaInfo.height  = height_;

    pData += 6;

    // check if the image has supported properties
    if ((width_ <= 0) || (height_ <= 0) || ((bpp_ != 24) && (bpp_ != 32)))
    {
        LogErr("uncompressed targa image's params are not supported");
        return false;
    }

    tgaInfo.bytesPerPixel = bpp_ / 8;
    tgaInfo.imageSize     = (tgaInfo.bytesPerPixel * tgaInfo.width * tgaInfo.height);

    // copy the image data
    memcpy(pData_, pData, tgaInfo.imageSize);

    // byte swapping (optimized by Steve Thomas)
    for (int idx = 0; idx < (int)tgaInfo.imageSize; idx += tgaInfo.bytesPerPixel)
    {
        pData_[idx] ^= pData_[idx + 2] ^=
        pData_[idx] ^= pData_[idx + 2];
    }

    // the uncompressed TGA has been successfully loaded
    return true;
}
