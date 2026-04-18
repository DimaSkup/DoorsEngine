// =================================================================================
// Filename:  Image.cpp
//
// Created:   24.05.2025  by DimaSkup
// =================================================================================
#include "image.h"

#include "log.h"
#include "raw_file.h"        // load/save .RAW file

#include <stdio.h>
#include <assert.h>
#include <memory.h>
#include <string.h>
#include <math.h>

#pragma warning (disable : 4996)


//---------------------------------------
// helper to delete arrays
//---------------------------------------
template <typename T>
inline void SafeDeleteArr(T*& p)
{
    if (p) { delete[](p); p = nullptr; }
}

// --------------------------------------------------------
// default constructor
// --------------------------------------------------------
Image::Image()
{
}

// --------------------------------------------------------
// Desc:  create an image loading its data from file
// --------------------------------------------------------
Image::Image(const char* filepath)
{
    Load(filepath);
}

// --------------------------------------------------------
// destructor
// --------------------------------------------------------
Image::~Image()
{
    Shutdown();
}

// --------------------------------------------------------
// Desc:   create empty image
// Args:   - width, height: dimensions of the new image
//         - bpp:           bits per pixel
// --------------------------------------------------------
bool Image::CreateEmpty(const uint width, const uint height, const uint bpp)
{
    Shutdown();

    // allocate memory
    const int numBytes = width * height * (bpp >> 3);

    pixels_ = new uint8[numBytes];

    // fill in the pixels buffer with white color
    memset(pixels_, 255, numBytes);

    // set the member variables
    width_         = width;
    height_        = height;
    bpp_           = bpp;
    bytesPerPixel_ = bpp >> 3;
    isLoaded_      = true;

    return true;
}

// --------------------------------------------------------
// Desc:   unload the texture that is currently loaded
// --------------------------------------------------------
void Image::Shutdown()
{
    SafeDeleteArr(pixels_);

    width_ = 0;
    height_ = 0;
    bpp_ = 0;

    isLoaded_ = false;
}

//-----------------------------------------------------
// Desc:  extract file extension from input path
//-----------------------------------------------------
void Image::GetImageExt(const char* path, char* outExt) const
{
    if (!path || path[0] == '\0')
    {
        LogErr(LOG, "empty path");
        return;
    }

    const char* dotPos = strrchr(path, '.');

    if (!dotPos)
    {
        strcpy(outExt, "invalid");
        return;
    }
     
    strcpy(outExt, dotPos);
    outExt[strlen(dotPos)] = '\0';                     // put extra NULL
}


// ----------------------------------------------------
// Desc:   get a color (RGB triplet) from a texture
// Args:   - x, y:     position to get color from
// Output: - r, g, b:  value of each channel
// ----------------------------------------------------
void Image::GetPixelColor(const uint x, const uint y, 
                                        uint8& r, uint8& g, uint8& b) const
{
    assert(bpp_ == 24 || bpp_ == 32);

    const uint idx = ((y * height_) + x) * bytesPerPixel_;

    r = pixels_[idx + 0];
    g = pixels_[idx + 1];
    b = pixels_[idx + 2];
}

// ----------------------------------------------------
// Desc:   get a color (RGBA) from a texture
// Args:   - x, y:        position to get color from
// Output: - r, g, b, a:  value of each channel
// ----------------------------------------------------
void Image::GetPixelColor(const uint x, const uint y,
                          uint8& r, uint8& g, uint8& b, uint8& a) const
{
    assert(bpp_ == 32);

    const uint idx = ((y * height_) + x) * bytesPerPixel_;

    r = pixels_[idx + 0];
    g = pixels_[idx + 1];
    b = pixels_[idx + 2];
    a = pixels_[idx + 3];
}

// ----------------------------------------------------
// Desc:   set the color (RGB triplet) for a texture pixel
// Args:   - x, y:     pixel position
//         - r, g, b:  color to set
// ----------------------------------------------------
void Image::SetPixelColor(const uint x, const uint y, 
                                 const uint8 r, const uint8 g, const uint8 b)
{
    assert(bpp_ == 24 || bpp_ == 32);

    if ((x < width_) && (y < height_))
    {
        const uint idx = ((y * height_) + x) * bytesPerPixel_;

        pixels_[idx + 0] = r;
        pixels_[idx + 1] = g;
        pixels_[idx + 2] = b;
    }
}



// --------------------------------------------------------
// Desc:   load only the data for a new image
// Args:   filename: the file to load in
// --------------------------------------------------------
bool Image::Load(const char* filepath)
{
    if (!filepath || filepath[0] == '\0')
    {
        LogErr(LOG, "empty filepath");
        return false;
    }

    // get file extension
    char ext[8]{ '\0' };
    GetImageExt(filepath, ext);


    // read BMP image
    if (strcmp(ext, ".bmp") == 0)
    {
        if (!LoadRgbBMP(filepath))
        {
            LogErr(LOG, "can't load BMP file: %s", filepath);
            Shutdown();
            return false;
        }
    }

    // we read TGA image
    else if (strcmp(ext, ".tga") == 0)
    {
        LogErr(LOG, "tga loading is broken :(");
        return false;

#if 0
        if (!LoadTGA(filepath))
        {
            LogErr(LOG, "can't load TGA file: %s", filepath);
            Shutdown();
            return false;
        }
#endif
    }

    // read RAW file
    else if (strcmp(ext, ".raw") == 0)
    {
        if (!LoadRAW(filepath))
        {
            LogErr(LOG, "can't load RAW file: %s", filepath);
            Shutdown();
            return false;
        }
    }
     
    // the file is not supported by our image loader
    else
    {
        LogErr(LOG, "a file has a NOT supported image type: %s", filepath);
        Shutdown();
        return false;
    }


    isLoaded_ = true;
    LogMsg(LOG, "file is loaded: %s", filepath);
    return true;
}

// --------------------------------------------------------
// Desc:   save image into file (supported formats: BMP, RAW)
// Args:   - filepath: where to store the image
// --------------------------------------------------------
bool Image::Save(const char* filepath) const
{
    // check input params
    if (!filepath || filepath[0] == '\0')
    {
        LogErr(LOG, "empty filepath");
        return false;
    }

    // check if we have proper data to save
    if (!width_ || !height_ || !pixels_ || !isLoaded_)
    {
        LogErr(LOG, "can't save: image is empty or corrupted (%s)", filepath);
        return false;
    }

    char ext[8]{'\0'};
    GetImageExt(filepath, ext);

    // store height map into RAW file
    if (strcmp(ext, ".raw") == 0)
    {
        if (!SaveAsRAW(filepath))
        {
            LogErr(LOG, "can't save .raw file: %s", filepath);
            return false;
        }
    }

    // store height map into BMP file
    else if (strcmp(ext, ".bmp") == 0)
    {
        if (!SaveAsBMP(filepath))
        {
            LogErr(LOG, "can't save .bmp file: %s", filepath);
            return false;
        }
    }

    else
    {
        LogErr(LOG, "can't save image into file: unsupported format (%s)", ext);
        return false;
    }

    LogMsg(LOG, "The height map was saved successfully: %s", filepath);
    return true;
}



//==================================================================================
// 
//                            SPECIFIC LOADERS
// 
//==================================================================================

//--------------------------------------------------------------
// Desc:    load bmp image data
// Output:  bmp file header, info header
// Return:  ptr to array of pixels or nullptr if we didn't manage to load data
//--------------------------------------------------------------
uint8* LoadBmpData(
    const char* filepath,
    BMPFileHeader& fileHeader,
    BMPInfoHeader& infoHeader)
{
    if (!filepath || filepath[0] == '\0')
    {
        LogErr(LOG, "empty filepath");
        return nullptr;
    }


    uint8* pixels = nullptr;
    size_t retCode = 0;

    // open file in binary and read in headers
    FILE* pFile = fopen(filepath, "rb");
    if (!pFile)
    {
        LogErr(LOG, "can't open a file for reading in binary: %s", filepath);
        return nullptr;
    }

    retCode = fread(&fileHeader, sizeof(BMPFileHeader), 1, pFile);
    if (retCode != 1)
    {
        LogErr(LOG, "error reading a bitmap file header: %s", filepath);
        fclose(pFile);
        return nullptr;
    }

    // check for BM reversed...
    if (fileHeader.type != 'MB')
    {
        LogErr(LOG, "not a bitmap file: %s", filepath);
        fclose(pFile);
        return nullptr;
    }

    retCode = fread(&infoHeader, sizeof(BMPInfoHeader), 1, pFile);
    if (retCode != 1)
    {
        LogErr(LOG, "error reading a bitmap info header: %s", filepath);
        fclose(pFile);
        return nullptr;
    }

    if (infoHeader.compression != 0)
    {
        LogErr(LOG, "only non-compressed bitmap images are supported: %s", filepath);
        fclose(pFile);
        return nullptr;
    }

    if (infoHeader.bitCount != 24 && infoHeader.bitCount != 32)
    {
        LogErr(LOG, "unsupported bit count (%u), it must be 24 or 32 (for file: %s)", infoHeader.bitCount, filepath);
        fclose(pFile);
        return nullptr;
    }


    // calculate a size of image pixels data
    uint bitSize = 0;
    
    if (infoHeader.sizeImage != 0)
        bitSize = infoHeader.sizeImage;
    else
        bitSize = (infoHeader.width * infoHeader.bitCount + 7) / 8 * abs(infoHeader.height);

    // allocate memory
    pixels = new uint8[bitSize];

    // read in the bitmap image data
    retCode = fread(pixels, 1, bitSize, pFile);
    if (retCode != bitSize)
    {
        LogErr(LOG, "can't read the bitmap image data: %s", filepath);
        fclose(pFile);
        SafeDeleteArr(pixels);
    }

    fclose(pFile);
    return pixels;
}

//--------------------------------------------------------------
// Desc:  RGB bitmap (BMP) image loader
//--------------------------------------------------------------
bool Image::LoadRgbBMP(const char* filepath)
{
    BMPFileHeader fileHeader = {};
    BMPInfoHeader infoHeader = {};

    uint8* tmpPixels = LoadBmpData(filepath, fileHeader, infoHeader);
    if (!tmpPixels)
    {
        LogErr(LOG, "can't load bitmap image data");
        return false;
    }

    if (!CreateEmpty(infoHeader.width, infoHeader.height, infoHeader.bitCount))
    {
        LogErr(LOG, "can't create an empty space for bmp image: %s", filepath);
        SafeDeleteArr(tmpPixels);
        return false;
    }


    // initialize the position in the final image data buffer
    uint k = 0;

    const uint width           = infoHeader.width;
    const uint height          = infoHeader.height;
    const uint bytesPerPixel   = infoHeader.bitCount >> 3;

    const uint bytesInScanline = width * bytesPerPixel;
    const uint padding         = (width % 2 != 0) * (bytesPerPixel == 3);


    // read the image data into the final pixels array
    for (uint j = 0; j < height; ++j)
    {
        // bitmaps are upside down so load bottom to top into the array
        const uint rowIdx = (height - 1 - j) * bytesInScanline;

        for (uint i = 0; i < width; ++i)
        {
            const uint baseIdx = rowIdx + i * bytesPerPixel;

            // swap the R and B values to get RGB since the bitmap color format is in BGR
            pixels_[baseIdx + 0] = tmpPixels[k + 2];
            pixels_[baseIdx + 1] = tmpPixels[k + 1];
            pixels_[baseIdx + 2] = tmpPixels[k + 0];

            k += bytesPerPixel;
        }

        // compensate for the extra byte at end of each line in 
        // non-divide by 2 bitmaps (eg. 257x257) with 24 bpp 
        k += padding;
    }

    // release temp buffer
    SafeDeleteArr(tmpPixels);

    SetName(filepath);

    return true;
}

//---------------------------------------------------------
// Desc:   to minimize memory usage we load grayscale bmp images
//         in a specific way: just get only R-channel of each pixel
//         and set bits per pixel (bpp) value == 8 for this image instance
//---------------------------------------------------------
bool Image::LoadGrayscaleBMP(const char* filepath)
{
    size_t        retCode = 0;
    BMPFileHeader fileHeader = {};
    BMPInfoHeader infoHeader = {};
    const uint8   bpp = 8;

    uint8* tmpPixels = LoadBmpData(filepath, fileHeader, infoHeader);
    if (!tmpPixels)
    {
        LogErr(LOG, "can't load bitmap image data");
        return false;
    }

    if (!CreateEmpty(infoHeader.width, infoHeader.height, bpp))
    {
        LogErr(LOG, "can't create an empty space for bmp image: %s", filepath);
        SafeDeleteArr(tmpPixels);
        return false;
    }

    // initialize the position in the image data buffer
    int k = 0;
    
    const int width         = infoHeader.width;
    const int height        = infoHeader.height;

    const int bytesPerPixel = infoHeader.bitCount >> 3;
    const int padding       = (width % 2 != 0) * (bytesPerPixel == 3);

    // read the image data into the final pixels array
    for (int j = 0; j < height; ++j)
    {
        // bitmaps are upside down so load bottom to top into the array
        const int rowIdx = (height - 1 - j) * width;

        for (int i = 0; i < width; ++i)
        {
            // get the grey scale pixel value from the bitmap image data at this location
            pixels_[rowIdx + i] = tmpPixels[k];

            // increment the bitmap image data idx (skip G and B channels, and also alpha if bpp == 32)
            k += bytesPerPixel;
        }

        // compensate for the extra byte at end of each line in 
        // non-divide by 2 bitmaps (eg. 257x257) with 24 bpp 
        k += padding;
    }

    // release temp buffer
    SafeDeleteArr(tmpPixels);

    SetName(filepath);

    return true;
}

//---------------------------------------------------------
// Desc:  load .RAW file as a grayscale 8-bit image
//---------------------------------------------------------
bool Image::LoadRAW(const char* filepath)
{
    if (!filepath || filepath[0] == '\0')
    {
        LogErr(LOG, "empty filepath");
        return false;
    }


    int fileSize = 0;

    if (!LoadFileRAW(filepath, &pixels_, fileSize))
    {
        LogErr(LOG, "can't load raw-file as grayscale 8-bit image: %s", filepath);
        return false;
    }

    // we expect square image which is grayscale with bits per pixel (bpp) == 8
    const uint dimension = (uint)sqrtf((float)fileSize);
    width_  = dimension;
    height_ = dimension;
    bpp_    = 8;

    SetName(filepath);

    return true;
}

#if 0

//--------------------------------------------------------------
// Desc:   targa (TGA) image loader
// Args:   - filepath:  a path to image file
//--------------------------------------------------------------
bool Image::LoadTGA(const char* filepath)
{
    TGAInfoHeader tgaInfo;
    FILE*  pFile = nullptr;
    uint8* data = nullptr;
    int    fileSize = 0;

    
    if (!filepath || filepath[0] == '\0')
    {
        LogErr(LOG, "empty filepath");
        return false;
    }

    pFile = fopen(filepath, "rb");
    if (!pFile)
    {
        LogErr(LOG, "can't open a file for reading in binary mode: %s", filepath);
        return false;
    }

    // get file length
    fseek(pFile, 0, SEEK_END);
    fileSize = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    // allocate memory for the file content
    data = new uint8[fileSize]{0};

    // read in file content
    if (fread(data, 1, fileSize, pFile) != (unsigned)fileSize)
    {
        LogErr(LOG, "can't read in pixels data: %s", filepath);
        SafeDeleteArr(data);
        fclose(pFile);
        return false;
    }

    // load TGA info header data
    uint8* fileData = data;
    memcpy(&tgaInfo, fileData, sizeof(TGAInfoHeader));

    const uint width  = (uint)tgaInfo.width;
    const uint height = (uint)tgaInfo.height;
    const uint bpp    = (uint)tgaInfo.bitsPerPixel;

    // create an empty image
    CreateEmpty(width, height, bpp);

    // skip the header and move to the actual pixels data
    fileData += sizeof(TGAInfoHeader);

    // copy pixels raw data into buffer
    //uint imageSize = width*height*(bpp/8);
    //memcpy(pixels_, fileData, imageSize);

    // close the file
    fclose(pFile);

    // define if tga image is compressed or uncompressed
    switch (tgaInfo.imageType)
    {
        case UncompressedRgb:
        {
            LoadUncompressedTGA(tgaInfo, fileData);
            break;
        }
        case RleRgb:
        {
            // if we want to read in RGB image
            if (tgaInfo.BytesPerPixel() == 3)
            {
                LoadCompressedTGA24(fileData, tgaInfo.width, tgaInfo.height);
            }
            // we want to read in RGBA image
            else
            {
                LoadCompressedTGA32(fileData, tgaInfo.width, tgaInfo.height);
            }
            break;
        }
        default:
        {
            LogErr(LOG, "unsupported TGA image type: %d", tgaInfo.imageType);
        }
    } // switch


    // release the temp data buffer
    SafeDeleteArr(data);

    return true;
}

//--------------------------------------------------------------
// Desc:   read in RLE compressed TGA image data with 24 bits per pixel
// Args:   - data:           pixels raw data
//         - width, height:  dimensions of image
//--------------------------------------------------------------
void Image::LoadCompressedTGA24(uint8*& data, const uint width, const uint height)
{
    assert(data);
    assert(width > 0 && height > 0);

    constexpr uint bytesPerPixel = 3;
    uint           currByte = 0;
    uint8          colorBuf[bytesPerPixel]{ 0 };

    uint expectBytes = width * height * bytesPerPixel;
    uint actualBytes = 0;

    // check if the pixels buffer is valid
    if (!pixels_)
    {
        LogErr(LOG, "you have to alloc the pixels buffer first!");
        return;
    }

    for (uint y = 0; y < height; ++y)
    {
        for (uint x = 0; x < width;)
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

                    actualBytes += 3;
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

                    actualBytes += 3;
                }
            }
        }
    }

    SetConsoleColor(CYAN);
    LogMsg("tga: expect bytes: %d", expectBytes);
    LogMsg("tga: actual bytes: %d", actualBytes);
    SetConsoleColor(RESET);
}

//--------------------------------------------------------------
// Desc:   read in RLE compressed TGA image data with 32 bits per pixel
// Args:   - data:           pixels raw data
//         - width, height:  dimensions of image
//--------------------------------------------------------------
void Image::LoadCompressedTGA32(uint8*& data, const uint width, const uint height)
{
    assert(data);
    assert(width > 0 && height > 0);

    constexpr uint bytesPerPixel = 4;
    uint           currByte = 0;
    uint8          colorBuf[bytesPerPixel]{ 0 };


    // check if the pixels buffer is valid
    if (!pixels_)
    {
        LogErr(LOG, "you have to allocate the pixels buffer first!");
        return;
    }

    for (uint y = 0; y < height; ++y)
    {
        for (uint x = 0; x < width;)
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

// --------------------------------------------------------
// Desc:   private HELPER load an uncompressed targa (TGA) image
// --------------------------------------------------------
void Image::LoadUncompressedTGA(const TGAInfoHeader& header, uint8*& data)
{
    assert(data);

    const uint numPixels      = header.width * header.height;
    const uint bytesPerPixel  = header.BytesPerPixel();
    const uint imgSizeInBytes = numPixels * bytesPerPixel;

    assert(numPixels > 0);
    assert(bytesPerPixel > 0);

    // check if the pixels buffer is valid
    if (!pixels_)
    {
        LogErr(LOG, "you have to allocate the pixels buffer first!");
        return;
    }

    // copy and swap R and B channels
    for (uint i = 0; i < imgSizeInBytes; i += bytesPerPixel)
    {
        pixels_[i+0] = data[i+2];
        pixels_[i+1] = data[i+1];
        pixels_[i+2] = data[i+0];
    }
}

#endif


//==================================================================================
// 
//                           SPECIFIC SAVERS
// 
//==================================================================================

// --------------------------------------------------------
// Desc:   save the current image info to a BMP file.
// Args:   - filename:  where to save
// --------------------------------------------------------
bool Image::SaveAsBMP(const char* filepath) const
{
    BMPFileHeader  bitmapFileHeader;
    BMPInfoHeader  bitmapInfoHeader;

    const uint bytesPerPixel = 3;
    const uint headersOffset = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
    
    uint bytesInScanline   = (width_ * bytesPerPixel);
   
    // compensate for the extra bytes at end of each line (to make boundary 4 bytes aligned)
    const uint overFour    = (bytesInScanline & 3);
    const uint paddingSize = (overFour == 0) ? 0 : 4-overFour ;
    const uint imageSize   = height_ * (width_ * bytesPerPixel + paddingSize);

    // account compensation
    bytesInScanline += paddingSize;


    // check input params
    if (!filepath || filepath[0] == '\0')
    {
        LogErr(LOG, "empty filename");
        return false;
    }

    // open a file that we can write to
    FILE* pFile = fopen(filepath, "wb");
    if (!pFile)
    {
        LogErr(LOG, "can't open a file to save BMP image: %s", filepath);
        return false;
    }

    // define the BMP file header
    bitmapFileHeader.type            = 0x4D42;// BITMAP_ID;
    bitmapFileHeader.size            = headersOffset + imageSize;
    bitmapFileHeader.reserved1       = 0;
    bitmapFileHeader.reserved2       = 0;
    bitmapFileHeader.offBits         = headersOffset;
  
    // define the BMP info header
    bitmapInfoHeader.size            = sizeof(BMPInfoHeader);
    bitmapInfoHeader.width           = width_;
    bitmapInfoHeader.height          = height_;
    bitmapInfoHeader.planes          = 1;
    bitmapInfoHeader.bitCount        = 24;
    bitmapInfoHeader.compression     = 0;          // no compression
    bitmapInfoHeader.sizeImage       = imageSize;
    bitmapInfoHeader.xPixelsPerMeter = 0;
    bitmapInfoHeader.yPixelsPerMeter = 0;
    bitmapInfoHeader.colorUsed       = 0;
    bitmapInfoHeader.colorImportant  = 0;


    //
    // copy pixels into temp buffer
    //
    uint8* bmpPixels = new uint8[imageSize];
    memset(bmpPixels, 0, imageSize);


    // if the original image is grayscale
    if (bpp_ == 8)
    {
        int k = 0;
        const int width  = width_;
        const int height = height_;

        for (int j = 0; j < height; ++j)
        {
            // bitmaps are upside down so load bottom to top into the array
            const int rowIdx = (height - 1 - j) * bytesInScanline;

            for (int i = 0; i < width; ++i)
            {
                bmpPixels[rowIdx + i*3 + 0] = pixels_[k];
                bmpPixels[rowIdx + i*3 + 1] = pixels_[k];
                bmpPixels[rowIdx + i*3 + 2] = pixels_[k];
                k++;
            }
        }
    }

    // if original image is already 24 bits per pixel
    else if (bpp_ == 24)
    {
        uint k = 0;
        const uint height = height_;
        const uint width  = width_;

        for (uint j = 0; j < height; ++j)
        {
            // bitmaps are upside down so load bottom to top into the array
            uint rowIdx = (height - 1 - j) * bytesInScanline;

            for (uint i = 0; i < width; ++i)
            {
                uint base = rowIdx + i*3;

                // swap the image bit order (RGB to BGR)
                bmpPixels[base + 0] = pixels_[k + 2];
                bmpPixels[base + 1] = pixels_[k + 1];
                bmpPixels[base + 2] = pixels_[k + 0];

                k += 3;
            }
        }
    }

    // convert original 32 bits image into 24 bits image
    else
    {
        uint k = 0;
        const uint height = height_;
        const uint width = width_;

        for (uint j = 0; j < height; ++j)
        {
            // bitmaps are upside down so load bottom to top into the array
            uint rowIdx = (height - 1 - j) * bytesInScanline;

            for (uint i = 0; i < width; ++i)
            {
                uint base = rowIdx + i*3;

                // swap the image bit order (RGB to BGR)
                bmpPixels[base + 0] = pixels_[k + 2];
                bmpPixels[base + 1] = pixels_[k + 1];
                bmpPixels[base + 2] = pixels_[k + 0];

                k += 4;
            }
        }
    }


    // write the .bmp file header to the file
    fwrite(&bitmapFileHeader, 1, sizeof(BMPFileHeader), pFile);

    // write the .bmp file info header to the file
    fwrite(&bitmapInfoHeader, 1, sizeof(BMPInfoHeader), pFile);

    // now write the actual image data
    fwrite(bmpPixels, 1, imageSize, pFile);

    // release the pixels temp buffer
    SafeDeleteArr(bmpPixels);

    // the file has been successfully saved
    fclose(pFile);

    return true;
}

//---------------------------------------------------------
// Desc:  save a grayscale image as RAW
//---------------------------------------------------------
bool Image::SaveAsRAW(const char* filepath) const
{
    if (!filepath || filepath[0] == '\0')
    {
        LogErr(LOG, "empty filepath");
        return false;
    }

    if (bpp_ != 8)
    {
        LogErr(LOG, "you can save as RAW only 8-bits grayscale images (img name: %s, into file: %s)", name_, filepath);
        return false;
    }

    if (!SaveFileRAW(filepath, pixels_, width_ * height_))
    {
        LogErr(LOG, "can't save image (%s) as RAW file: %s", name_, filepath);
        return false;
    }

    return true;
}

//---------------------------------------------------------
// Desc:  save input data as a grayscale BMP
// Args:  - filepath:  where to store
//        - data:      array of pixels in range 0-255
//        - w, h:      width and height of image
// 
// Ret:   true if everything is ok
//---------------------------------------------------------
bool Image::SaveRawAsGrayBmp(const char* filepath, const uint8* data, uint w, uint h)
{
    if (!filepath || filepath[0] == '\0')
    {
        LogErr(LOG, "empty filepath");
        return false;
    }
    if (!data)
    {
        LogErr(LOG, "empty input data");
        return false;
    }
    if (w == 0 || h == 0)
    {
        LogErr(LOG, "wrong dimensions (w: %u, h: %u), it must be > 0", w, h);
        return false;
    }

    Image img;
    img.pixels_ = (uint8*)data;
    img.width_  = w;
    img.height_ = h;
    img.bpp_    = 8;

    // save input raw data as image
    if (!img.SaveAsBMP(filepath))
    {
        LogErr(LOG, "can't save data as grayscaled BMP: %s", filepath);
        img.pixels_ = nullptr;
        return false;
    }

    // prevent destruction of input data
    img.pixels_ = nullptr;

    return true;
}


//---------------------------------------------------------
// Desc:  setup a name for this image (usually it is a path to this image)
//---------------------------------------------------------
void Image::SetName(const char* name)
{
    if (!name || name[0] == '\0')
    {
        LogErr(LOG, "empty name");
        return;
    }

    size_t maxLen = sizeof(name_);
    size_t len = strlen(name);

    if (len > maxLen) 
        len = maxLen;

    strncpy(name_, name, len);
}
