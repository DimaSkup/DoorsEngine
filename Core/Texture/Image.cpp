// =================================================================================
// Filename:  Image.cpp
//
// Created:   24.05.2025  by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include "image.h"

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
        // allocate memory
        pixels_ = new uint8[width * height * (bpp / 8)]{ 0 };

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

        // save the filename
        size_t len = strlen(filename);
        len = (len > 64) ? 64 : len;
        strncpy(name_, filename, len);

        // open the file for reading (in binary mode)
        pFile = fopen(filename, "rb");
        if (!pFile)
        {
            sprintf(g_String, "can't open file: %s", filename);
            throw EngineException(g_String);
        }
 
        // get file length
        fseek(pFile, 0, SEEK_END);
        const int size = ftell(pFile);
        fseek(pFile, 0, SEEK_SET);

        // release memory from the previous image data (if we have any)
        Unload();

        // allocate the data buffer
        pixels_ = new uint8[size]{ 0 };

        // read the image's data into the buffer
        if (fread(pixels_, sizeof(uint8), size, pFile) != (unsigned)size)
        {
            // the file is corrupted
            sprintf(g_String, "%s is corrupted, could not read all data", filename);
            throw EngineException(g_String);
        }

        // get file extension
        char extension[8]{'\0'};
        FileSys::GetFileExt(filename, extension);

        if (strcmp(extension, "invalid") == 0)
        {
            sprintf(g_String, "can't get extension of file: %s", filename);
            throw EngineException(g_String);
        }

        // check if we read BMP image
        if (*(uint16*)pixels_ == 0x4d42)
        {
            if (!LoadBMP())
            {
                sprintf(g_String, "can't load BMP file: %s", filename);
                throw EngineException(g_String);
            }
        }
        // check if we read TGA image
        else if (strcmp(extension, ".tga") == 0)
        {
            if (!LoadTGA())
            {
                sprintf(g_String, "can't load TGA file: %s", filename);
                throw EngineException(g_String);
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
    const uint      imageSize     = width_ * height_ * bytesPerPixel;


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

    // if original image is already 24 bits per pixel
    if (bpp_ == 24)
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
// Desc:   a windows bitmap (BMP) image loader
//--------------------------------------------------------------
bool Image::LoadBMP()
{
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
}

//--------------------------------------------------------------
// Desc:   targa (TGA) image loader
//--------------------------------------------------------------
bool Image::LoadTGA(void)
{
    TGAInfoHeader tgaInfo;

    // read in the image header
    memcpy(&tgaInfo, pixels_, sizeof(TGAInfoHeader));

    // skip header up to the pixels raw data
    uint8* pixelsData = pixels_ + sizeof(TGAInfoHeader);

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

    // define if tga image is compressed or uncompressed
    switch (tgaInfo.imageType)
    {
        case UncompressedRgb:
        {
            LoadUncompressedTGA(tgaInfo, pixelsData);
            return true;
        }
        case RleRgb:
        {
            LoadCompressedTGA(tgaInfo, pixelsData);
            return true;
        }
        default:
        {
            sprintf(g_String, "unsupported TGA image type: %d", tgaInfo.imageType);
            LogErr(g_String);
            return false;
        }
    } // switch
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
        SWAP(pixels_[idx+0], pixels_[idx+2]);
    }
  
}

} // namespace Core
