// *********************************************************************************
// Filename:      textureclass.h
// Description:   Encapsulates the loading, unloading, and accessing
//                of a single texture resource. For each texture 
//                needed an object of this class to be instantiated.
//
// Revising:      09.04.22
// *********************************************************************************
#pragma once


#include "TextureTypes.h"
#include "../Render/Color.h"

#include <Types.h>
#include <assimp/material.h>
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

///////////////////////////////////////////////////////////

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
    Texture(ID3D11Device* pDevice, const char* filePath);	

    // for multiple textures to create a Texture2DArray
    Texture(
        ID3D11Device* pDevice,
        const char* name,            // a name for this texture object
        const std::string* texturesNames,
        const size numTextures,
        const DXGI_FORMAT format);

    // make 1x1 texture with single color
    Texture(ID3D11Device* pDevice, const Color& color);

    // make width_x_height texture with color data
    Texture(
        ID3D11Device* pDevice, 
        const Color* pColorData,
        const UINT width,
        const UINT height);

    // a constructor for loading embedded compressed textures 
    Texture(
        ID3D11Device* pDevice,
        const char* name,
        const uint8_t* pData,
        const UINT width,
        const UINT height,
        const bool mipmapped);

    ~Texture();

    // --------------------------------------------------------------------------------

    bool Initialize(
        ID3D11Device* pDevice,
        const char* name,
        const uint8* data,
        const uint width,
        const uint height,
        const bool mipMapped);

    void Release();

    // deep copy
    void Copy(Texture& src);
    void Copy(ID3D11Resource* const pSrcTexResource);


    inline ID3D11Resource*           GetResource()                                { return pTexture_; }
    inline ID3D11ShaderResourceView* GetTextureResourceView()               const { return pTextureView_; }
    inline ID3D11ShaderResourceView* const* GetTextureResourceViewAddress() const { return &pTextureView_; }

    inline const std::string&        GetName()                              const { return name_; }
    inline UINT                      GetWidth()                             const { return width_; }
    inline UINT                      GetHeight()                            const { return height_; }

    POINT GetTextureSize();

    inline void SetName(const std::string& newPath) { name_ = newPath; } 


private:
  
    void LoadFromFile(
        ID3D11Device* pDevice, 
        const char* filePath);

    void Initialize1x1ColorTexture(
        ID3D11Device* pDevice, 
        const Color & colorData);

    void InitializeColorTexture(
        ID3D11Device* pDevice, 
        const Color* pColorData, 
        const UINT width, 
        const UINT height);

private:
    std::string               name_ {"no_tex_name"};
    ID3D11Resource*           pTexture_ = nullptr;
    ID3D11ShaderResourceView* pTextureView_ = nullptr;   
    UINT                      width_ = 30;                                   
    UINT                      height_ = 30;
};

} // namespace Core
