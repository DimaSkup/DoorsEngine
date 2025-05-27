// =================================================================================
// Filename:  Image.h
// Desc:      wrapper over the .bmp and .tga image/texture formats
//
// Created:   23.05.2025  by DimaSkup
// =================================================================================
#pragma once
#include <CoreCommon/Types.h>

// ========================================================
// Constants/typedefs
// ========================================================
constexpr int BITMAP_ID = 0x4D42;


// ========================================================
// Structures
// ========================================================
struct TGAInfoHeader
{
    uint8 header[6];
    uint  bytesPerPixel;
    uint  imageSize;
    uint  temp;
    uint  type;
    uint  height;
    uint  width;
    uint  bpp;
};

#pragma pack( push, 1 )

struct BMPFileHeader
{
    ushort type;
    uint   size;
    ushort reserved1;
    ushort reserved2;
    uint   offBits;
};

#pragma pack( pop )

struct BMPInfoHeader
{
    uint    size;
    long    width;
    long    height;
    ushort  planes;
    ushort  bitCount;
    uint    compression;
    uint    sizeImage;
    long    xPixelsPerMeter;
    long    yPixelsPerMeter;
    uint    colorUsed;
    uint    colorImportant;
};


// ========================================================
// Globals
// ========================================================
extern uint8 g_UTGAcompare[12];    // uncompressed TGA
extern uint8 g_CTGAcompare[12];    // compressed TGA


// ========================================================
// Class
// ========================================================
class Image
{
public:
    bool Create(const uint width, const uint height, const uint bpp);

    bool LoadData(const char* filename);
    bool Load    (const char* filename, const bool mipmapped = false);
    bool SaveBMP (const char* filename) const;

    void Unload();
    

    // ----------------------------------------------------
    // Desc:   get the color (RGB triplet) from a texture pixel
    // Args:   - x, y: position to get color from
    //         - red, green, blue: place to store the RGB
    //           values that are extracted from the texture
    // ----------------------------------------------------
    inline void GetColor(const uint x, const uint y, uint8& red, uint8& green, uint8& blue) const
    {
        if ((x < width_) && (y < height_))
        {
            const uint bpp = bpp_ / 8;
            const uint idx = ((y*height_) + x) * bpp;

            red   = pData_[idx + 0];
            green = pData_[idx + 1];
            blue  = pData_[idx + 2];
        }
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
            const uint bpp = bpp_ / 8;
            const uint idx = ((y*height_) + x) * bpp;

            pData_[idx + 0] = red;
            pData_[idx + 1] = green;
            pData_[idx + 2] = blue;
        }
    }

    // ----------------------------------------------------

    inline uint8* GetData()   const { return pData_; }    // get a pointer to the image's data buffer
    inline uint   GetWidth()  const { return width_; }    // get the width of texture
    inline uint   GetHeight() const { return height_; }   // get the height of texture
    inline uint   GetBPP()    const { return bpp_; }      // get the number of bits per pixel
    inline uint   GetID()     const { return id_; }       // get the identifying of ID of texture

    inline void   SetID(const uint id) { id_ = id; }      // set the texture's identifying ID

    inline bool   IsLoaded()  const { return isLoaded_; } // find out if the texture has been loaded or not


private:
    bool LoadBMP(void);

    bool LoadCompressedTGA(void);
    bool LoadUncompressedTGA(void);

private:
    uint8*  pData_ = nullptr;
    uint    width_ = 0;
    uint    height_ = 0;
    uint    bpp_ = 0;
    uint    id_ = 0;

    bool    isLoaded_ = false;
};
