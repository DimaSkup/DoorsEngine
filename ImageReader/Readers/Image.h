// =================================================================================
// Filename:  Image.h
// Desc:      wrapper over the .bmp and .tga image/texture formats
//
// Created:   23.05.2025  by DimaSkup
// =================================================================================
#pragma once

#include <Types.h>


namespace ImgReader
{

// ========================================================
// enums / structures
// ========================================================
enum eTgaImageType
{
    NoImage             = 0,
    UncompressedIndexed = 1,
    UncompressedRgb     = 2,
    UncompressedGray    = 3,
    RleIndexed          = 9,
    RleRgb              = 10,  // Runlength encoded RGB images
    RleGray             = 11,  // Compressed, black and white images
};

///////////////////////////////////////////////////////////

#pragma pack( push, 1 )

struct TGAInfoHeader
{
    uint8   idLength;
    uint8   colormapType;
    uint8   imageType;
    uint16  colormapOrigin;
    uint16  colormapLength;
    uint8   colormapDepth;
    uint16  xOrigin;
    uint16  yOrigin;
    uint16  width;
    uint16  height;
    uint8   bitsPerPixel;
    uint8   imageDescriptor;

    inline bool IsRGB() const
    {
        return (imageType == UncompressedRgb || imageType == RleRgb);
    }

    // currently we support only RGB images (uncompressed/RLE)
    inline bool IsSupported() const
    {
        return IsRGB() && (bitsPerPixel == 24 || bitsPerPixel == 32);
    }

    inline int BytesPerPixel() const
    {
        return bitsPerPixel / 8;
    }
};

///////////////////////////////////////////////////////////

struct BMPFileHeader
{
    ushort  type;               // magic identifier
    uint    size;               // file size in bytes
    ushort  reserved1;
    ushort  reserved2;
    uint    offBits;            // offset to image data, bytes
};

///////////////////////////////////////////////////////////

struct BMPInfoHeader
{
    uint    size;               // header size in bytes
    long    width;              // width of the image
    long    height;             // height of the image
    ushort  planes;             // number of colour planes
    ushort  bitCount;           // bits per pixel
    uint    compression;        // compression byte
    uint    sizeImage;          // image size in bytes
    long    xPixelsPerMeter;    
    long    yPixelsPerMeter;
    uint    colorUsed;          // number of colors
    uint    colorImportant;     // important colors
};
#pragma pack( pop )


// ========================================================
// Class
// ========================================================
class Image
{
public:
    ~Image();

    bool Create(const uint width, const uint height, const uint bpp);
  
    bool LoadData        (const char* filename);
    bool Load            (const char* filename, const bool mipmapped = false);
    bool LoadGrayscaleBMP(const char* filename);

    bool Save    (const char* filename);
    bool SaveBMP (const char* filename) const;

    static bool SaveRawAsGrayBmp(
        const char* filename,
        const uint8* data,
        const int width,
        const int height);

    void Unload();

    // ----------------------------------------------------
    // Desc:   get the color (RGB triplet) from a texture pixel
    // Args:   - x, y: position to get color from
    //         - red, green, blue: place to store the RGB
    //           values that are extracted from the texture
    // ----------------------------------------------------
    inline void GetColor(const uint x, const uint y, uint8& red, uint8& green, uint8& blue) const
    {
        //if ((x < width_) && (y < height_))
        //{
            const uint idx = ((y*height_) + x) * bytesPerPixel_;

            red   = pixels_[idx + 0];
            green = pixels_[idx + 1];
            blue  = pixels_[idx + 2];
        //}
    }

    // ----------------------------------------------------
    // Desc:   set the color (RGB triplet) for a texture pixel
    // Args:   - x, y: position to set color at
    //         - red, green, blue: color to set at pixel
    // ----------------------------------------------------
    inline void SetColor(const uint x, const uint y, const uint8 red, const uint8 green, const uint8 blue)
    {
        if ((x < width_) && (y < height_))
        {
            const uint idx = ((y*height_) + x) * bytesPerPixel_;

            pixels_[idx + 0] = red;
            pixels_[idx + 1] = green;
            pixels_[idx + 2] = blue;
        }
    }

    // ----------------------------------------------------
    // Desc:   setup pixel for a grayscale image (when bpp == 8)
    // Args:   - pixel: pixel value (white color intensity from 0 to 255)
    //         - x, y:  position to set value
    // ----------------------------------------------------
    inline void SetPixelGray(const uint8 pixel, const int x, const int y)
    {
        pixels_[(y * width_) + x] = pixel;
    }

    // ----------------------------------------------------
    // Desc:   get gray pixel value
    // Ret:    uint8 value which is a white color intensity from 0 to 255
    // ----------------------------------------------------
    inline uint8 GetPixelGray(const int x, const int y) const
    {
        return pixels_[(y * width_) + x];
    }

    // ----------------------------------------------------

    inline uint8* GetData()   const { return pixels_; }   // get a pointer to the image's data buffer
    inline uint   GetWidth()  const { return width_; }    // get the width of texture
    inline uint   GetHeight() const { return height_; }   // get the height of texture
    inline uint   GetBPP()    const { return bpp_; }      // get the number of bits per pixel
    inline uint   GetID()     const { return id_; }       // get the identifying of ID of texture


    inline void   SetID(const uint id) { id_ = id; }      // set the texture's identifying ID
    inline bool   IsLoaded()  const { return isLoaded_; } // find out if the texture has been loaded or not


private:
    bool LoadRgbBMP(const char* filename);
    bool LoadTGA            (const char* filename);

    void LoadUncompressedTGA(const TGAInfoHeader& header, uint8*& pixelsData);
    void LoadCompressedTGA24(uint8*& pixelsData, const int width, const int height);
    void LoadCompressedTGA32(uint8*& pixelsData, const int width, const int height);

private:
    char    name_[64]{'\0'};
    uint    id_         = 0;
    uint    width_      = 0;
    uint    height_     = 0;
    uint    bpp_        = 0;           // bits per pixel
    uint    bytesPerPixel_ = 0;
    uint8*  pixels_     = nullptr;
    bool    isLoaded_   = false;
};

} // namespace Core
