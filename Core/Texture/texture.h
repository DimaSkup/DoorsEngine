// *********************************************************************************
// Filename:      textureclass.h
// Description:   Encapsulates the loading, unloading, and accessing
//                of a single texture resource. For each texture 
//                needed an object of this class to be instantiated.
//
// Revising:      09.04.22
// *********************************************************************************
#pragma once
#include "enum_texture_types.h"
#include <types.h>
#include "../Render/Color.h"
#include <d3d11.h>


namespace Core
{

enum class TexStoreType
{
    Invalid,
    None,
    EmbeddedIndexCompressed,
    EmbeddedIndexNonCompressed,
    EmbeddedCompressed,
    EmbeddedNonCompressed,
    Disk
};

//---------------------------------------------------------
// params needed for cubemap texture initialization
//---------------------------------------------------------
struct CubeMapInitParams
{
    // path to directory with textures for cubemap
    char directory[64]{'\0'};

    // 6 textures, max 32 chars for each name:
    //    0 - positive X
    //    1 - negative X
    //    2 - positive Y
    //    3 - negative Y
    //    4 - positive Z
    //    5 - negative Z
    char texNames[6][32]{'\0'};
};

//---------------------------------------------------------
// Texture class
//---------------------------------------------------------
class Texture
{
public:
    struct TexDimensions
    {
        UINT width = 0;
        UINT height = 0;
    };

public:
    Texture();

    // move constructor/assignment
    Texture(Texture&& rhs) noexcept;
    Texture& operator=(Texture&& rhs) noexcept;

    // restrict shallow copying
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    // a constructor for loading textures from the disk
    Texture(const char* fullFilePath, const char* name);

    // for multiple textures to create a Texture2DArray
    Texture(
        const char* name,
        const std::string* texturesNames,
        const size numTextures,
        const DXGI_FORMAT format);

    // make 1x1 texture with single color
    Texture(const Color& color);

    // make width_x_height texture with color data
    Texture(
        const Color* pColorData,
        const UINT width,
        const UINT height);

    // a constructor for loading embedded compressed textures 
    Texture(
        const char* name,
        const uint8_t* pData,
        const UINT width,
        const UINT height,
        const bool mipmapped);

    ~Texture();

    // -----------------------------------------------------------------------------

    bool Init(
        const char* name,
        const uint8* data,
        const uint width,
        const uint height,
        const bool mipMapped);

    void LoadFromFile(const char* filePath);

    void Release();

    // deep copy
    void Copy(Texture& src);
    void Copy(ID3D11Resource* const pSrcTexResource);

    bool CreateCubeMap(const char* name, const CubeMapInitParams& params);

    ID3D11Resource*           GetResource(void);
    ID3D11ShaderResourceView* GetResourceView(void) const;
    ID3D11ShaderResourceView* const* GetResourceViewAddr(void) const;

    const std::string&        GetName(void) const;
    UINT                      GetWidth(void) const;
    UINT                      GetHeight(void) const;

    void SetName(const char* name);

private:
    void Init1x1ColorTexture(const Color& data);
    void InitColorTexture   (const Color* pData, const UINT width, const UINT height);

private:
    std::string               name_ {"no_tex_name"};
    ID3D11Resource*           pTexture_ = nullptr;
    ID3D11ShaderResourceView* pTextureView_ = nullptr;   
    UINT                      width_ = 30;                                   
    UINT                      height_ = 30;
};


//---------------------------------------------------------
// inline methods
//---------------------------------------------------------

inline ID3D11Resource* Texture::GetResource(void)
{ 
	return pTexture_; 
}

inline ID3D11ShaderResourceView* Texture::GetResourceView(void) const
{ 
	return pTextureView_; 
}

inline ID3D11ShaderResourceView* const* Texture::GetResourceViewAddr(void) const
{ 
	return &pTextureView_; 
}

// get a name of this texture
inline const std::string& Texture::GetName(void) const 
{ 
	return name_; 
}

// return width of this texture
inline UINT Texture::GetWidth(void) const 
{ 
	return width_; 
}

// return height of this texture
inline UINT Texture::GetHeight(void) const 
{ 
	return height_; 
}


} // namespace Core
