// =================================================================================
// Filename:  Image.h
// Desc:      wrapper over the .bmp and .tga image/texture formats
//
// Created:   23.05.2025  by DimaSkup
// =================================================================================
#pragma once

#include <stdint.h>
#include <assert.h>

// some typedefs
using uint8  = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint   = unsigned int;
using ushort = unsigned short;


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
    int     width;              // width of the image
    int     height;             // height of the image
    ushort  planes;             // number of colour planes
    ushort  bitCount;           // bits per pixel
    uint    compression;        // compression byte
    uint    sizeImage;          // image size in bytes
    int     xPixelsPerMeter;    
    int     yPixelsPerMeter;
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
    Image();
    Image(const char* filepath);
    ~Image();

    bool CreateEmpty(const uint width, const uint height, const uint bpp);
    void Shutdown();

  
    bool Load            (const char* filepath);
    bool LoadRgbBMP      (const char* filepath);
    bool LoadGrayscaleBMP(const char* filepath);
    bool LoadRAW         (const char* filepath);

    bool Save            (const char* filepath) const;
    bool SaveAsBMP       (const char* filepath) const;
    bool SaveAsRAW       (const char* filepath) const;

    static bool SaveRawAsGrayBmp(const char* path, const uint8* data, uint w, uint h);


    // get pixel from RGB / grayscale image
    void  GetPixelColor(const uint x, const uint y, uint8& r, uint8& g, uint8& b)           const;
    void  GetPixelColor(const uint x, const uint y, uint8& r, uint8& g, uint8& b, uint8& a) const;


    uint8 GetPixelRed  (const uint x, const uint y) const;
    uint8 GetPixelGreen(const uint x, const uint y) const;
    uint8 GetPixelBlue (const uint x, const uint y) const;
    uint8 GetPixelGray (const uint x, const uint y) const;



    // set pixel for RGB / grayscale image
    void SetPixelColor(const uint x, const uint y, const uint8 r, const uint8 g, const uint8 b);
    void SetPixelGray (const uint x, const uint y, const uint8 grayscale);


    // ----------------------------------------------------
    // inline getters
    // ----------------------------------------------------
    inline uint8* GetPixels() const { return pixels_; }   // get a pointer to the image's data buffer
    inline uint   GetWidth()  const { return width_; }    // get the width of texture
    inline uint   GetHeight() const { return height_; }   // get the height of texture
    inline uint   GetBPP()    const { return bpp_; }      // get the number of bits per pixel
    inline uint   GetId()     const { return id_; }       // get the identifying of ID of texture

    inline bool   IsLoaded()  const { return isLoaded_; } // find out if the texture has been loaded or not


    // ----------------------------------------------------
    // inline setters
    // ----------------------------------------------------

    // set the texture's identifying ID (for detail watch: TextureMgr class)
    inline void   SetId    (const uint id)  { id_ = id; }      
    inline void   SetWidth (const uint w)   { width_ = w; }
    inline void   SetHeight(const uint h)   { height_ = h; }
    inline void   SetBPP   (const uint bpp) { bpp_ = bpp; }
    

private:
    void SetName(const char* name);

    void GetImageExt(const char* path, char* outExt) const;

    void LoadUncompressedTGA(const TGAInfoHeader& header, uint8*& pixelsData);
    void LoadCompressedTGA24(uint8*& pixelsData, const uint width, const uint height);
    void LoadCompressedTGA32(uint8*& pixelsData, const uint width, const uint height);

private:
    uint32 id_            = 0;
    uint32 width_         = 0;
    uint32 height_        = 0;
    uint32 bytesPerPixel_ = 0;
    uint32 bpp_ = 0;           // bits per pixel
    uint8* pixels_ = nullptr;
    char   name_[64]{ '\0' };
    bool   isLoaded_ = false;
};

//==================================================================================
// INLINE FUNCTIONS
//==================================================================================

// ----------------------------------------------------
// Desc:   setup pixel for a grayscale image (when bpp == 8)
// Args:   - grayscale: pixel value (white color intensity from 0 to 255)
//         - x, y:  position to set value
// ----------------------------------------------------
inline void Image::SetPixelGray(const uint x, const uint y, const uint8 grayscale)
{
    assert(bpp_ == 8);
    pixels_[(y * width_) + x] = grayscale;
}

// ----------------------------------------------------
// Desc:  return values of specific channel of pixel by coord x,y
// ----------------------------------------------------
inline uint8 Image::GetPixelRed(const uint x, const uint y) const
{
    assert((bpp_ == 24 || bpp_ == 32) && (x < width_ && y < height_));

    return pixels_[((y * height_) + x) * (bpp_ >> 3)];
}

inline uint8 Image::GetPixelGreen(const uint x, const uint y) const
{
    assert((bpp_ == 24 || bpp_ == 32) && (x < width_ && y < height_));

    return pixels_[((y * height_) + x) * (bpp_ >> 3) + 1];
}

inline uint8 Image::GetPixelBlue(const uint x, const uint y) const
{
    assert((bpp_ == 24 || bpp_ == 32) && (x < width_ && y < height_));

    return pixels_[((y * height_) + x) * (bpp_ >> 3) + 2];
}

// ----------------------------------------------------
// Desc:   get gray pixel value
// Ret:    grayscale value from 0 to 255
// ----------------------------------------------------
inline uint8 Image::GetPixelGray(const uint x, const uint y) const
{
    assert(bpp_ == 8);

#if (_DEBUG || DEBUG)
    const uint imageSize = width_ * height_;
    const uint idx = (y * width_) + x;

    return pixels_[idx * (idx < imageSize)];
#else
    return pixels_[(y * width_) + x];
#endif
}
