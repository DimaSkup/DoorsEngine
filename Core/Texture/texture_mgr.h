// =================================================================================
// Filename:      TextureMgr.h
// Description:   a manager for work with textures: 
//                when we ask for the texture for the first time we initialize it from 
//                the file and store it in the manager so later it'll be faster to just copy it
//                but not to read it from the file anew.
//                 
// Created:       06.06.23
// =================================================================================
#pragma once

#include "texture.h"

#include <cvector.h>
#include <d3d11.h>
//#include <d3dx11tex.h>
#include <string>


namespace Core
{

class TextureMgr
{
public:
    // typedefs and constants
    using SRV = ID3D11ShaderResourceView;
    //const TexID TEX_ID_UNLOADED = 0;
    //const TexID TEX_ID_UNHANDLED = 1;

public:
    TextureMgr();
    ~TextureMgr();

    // restrict shallow copying
    TextureMgr(const TextureMgr&) = delete;
    TextureMgr& operator=(const TextureMgr&) = delete;

    void PrintDump() const;

    // public creation API
    bool Init(const char* texturesCfg);

    TexID CreateTextureFromRawData(
        const char* name,
        const uint8* data,
        const uint width,
        const uint height,
        const int bpp,
        const bool mipMapped);

    void RecreateTextureFromRawData(
        const char* name,
        const uint8* data,
        const uint width,
        const uint height,
        const int bpp,
        const bool mipMapped,
        Texture& inOutTex);

    TexID LoadTextureArray(
        const char* textureObjName,
        const std::string* texturePaths,
        const size numTextures,
        const DXGI_FORMAT format);

    bool        ReloadFromFile(const TexID id, const char* path);
    TexID       Add          (const char* name, Texture&& tex);
    TexID       LoadFromFile (const char* name, const char* path);
    TexID       CreateCubeMap(const char* name, const CubeMapInitParams& params);

    TexID       CreateWithColor(const Color& textureColor);
    

    Texture&    GetTexByID     (const TexID id);
    Texture*    GetTexPtrByID  (const TexID id);
    Texture&    GetTexByName   (const char* name);
    Texture*    GetTexPtrByName(const char* name);
    void        SetTexName     (const TexID id, const char* inName);

    const char* GetTexTypeName(const uint texType) const;
    const char**GetTexTypesNames()                 const;
    const uint  GetNumTexTypesNames()              const;
    

    TexID       GetTexIdByName (const char* name);
    TexID       GetTexIdByIdx  (const index idx) const;

    SRV*        GetTexViewsById(const TexID texID);


    void GetTexViewsByIds(
        const TexID* texIDs,
        const size numTex,
        ID3D11ShaderResourceView** outSRVs);

    const ID3D11ShaderResourceView** GetAllShaderResourceViews();
    size                             GetNumShaderResourceViews() const;

private:
    int  GenID          (void);
    bool IsIdxValid     (const index idx) const;
    bool IsTexNameUnique(const char* texName) const;

private:
    static TexID         lastTexID_;

    cvector<TexID>       ids_;                 // SORTED array of unique IDs
    cvector<SRV*>        shaderResourceViews_;
    cvector<std::string> names_;               // name which is used for searching of texture
    cvector<Texture>     textures_;


    // transient arrays are used in GetSRVsByTexIDs()
    cvector<index> idxsToNotZero_;
    cvector<TexID> texIds_;
    cvector<index> tempIdxs_;
    cvector<SRV*>  preparedSRVs_;
};


// =================================================================================
// Declare a global instance of the texture manager
// =================================================================================
extern TextureMgr g_TextureMgr;



//---------------------------------------------------------
// Desc:  return a ptr to array of all the currently allocated shader resource views
//---------------------------------------------------------
inline const ID3D11ShaderResourceView** TextureMgr::GetAllShaderResourceViews()
{
    return (const ID3D11ShaderResourceView**)shaderResourceViews_.data();
}

//---------------------------------------------------------
// Desc:  return a number of all the currently allocated shader resource views
//---------------------------------------------------------
inline size TextureMgr::GetNumShaderResourceViews() const
{
    return shaderResourceViews_.size();
}

//-----------------------------------------------------
// Desc:  generate a values which will be an ID for new texture
//-----------------------------------------------------
inline int TextureMgr::GenID()
{
    return lastTexID_++;
}

//-----------------------------------------------------
// Desc:  check if input index of texture is in proper range
//-----------------------------------------------------
inline bool TextureMgr::IsIdxValid(const index idx) const
{
    return (idx >= 0 && idx < ids_.size());
}

} // namespace Core
