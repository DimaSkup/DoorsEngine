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

#include <CoreCommon/cvector.h>
#include "textureclass.h"

#include <d3d11.h>
#include <d3dx11tex.h>
#include <windows.h>
#include <string>


namespace Core
{

class TextureMgr
{
public:
    // typedefs and constants
    using SRV = ID3D11ShaderResourceView;
    const TexID TEX_ID_UNLOADED = 0;
    const TexID TEX_ID_UNHANDLED = 1;

public:
    TextureMgr();
    ~TextureMgr();

    // restrict shallow copying
    TextureMgr(const TextureMgr&) = delete;
    TextureMgr& operator=(const TextureMgr&) = delete;

    // public creation API
    void Initialize(ID3D11Device* pDevice);

    TexID Add(const TexName& name, Texture& tex);
    TexID Add(const TexName& name, Texture&& tex);
    
    TexID LoadFromFile(const TexPath& path);

    TexID LoadTextureArray(
        const std::string* textureNames,
        const size numTextures,
        const DXGI_FORMAT format);

    // TODO: FIX IT
    //void LoadFromFile(const std::vector<TexPath>& texPaths, std::vector<TexID>& outTexIDs);

    TexID CreateWithColor(const Color& textureColor);


    // public query API
    Texture& GetTexByID     (const TexID id);
    Texture* GetTexPtrByID  (const TexID id);
    Texture* GetTexPtrByName(const TexName& name);

    TexID GetIDByName (const TexName& name);
    TexID GetTexIdByIdx(const index idx) const;
    void GetIDsByNames(const TexName* names, const size numNames, cvector<TexID>& outIDs);

    ID3D11ShaderResourceView* GetSRVByTexID  (const TexID texID);
    void GetSRVsByTexIDs(const TexID* texIDs, const size numTex, cvector<ID3D11ShaderResourceView*>& outSRVs);

    inline const ID3D11ShaderResourceView** GetAllShaderResourceViews()       { return (const ID3D11ShaderResourceView**)shaderResourceViews_.data(); }
    inline size                             GetNumShaderResourceViews() const { return shaderResourceViews_.size(); }
#if 0
    void GetAllTexturesPathsWithinDirectory(
        const std::string& pathToDir,
        cvector<TexPath>& outPathsToTextures);
#endif

private:
    void AddDefaultTex(const TexName& name, Texture&& tex);

    inline int GenID() { return lastTexID_++; }

private:
    static TexID        lastTexID_;
    static TextureMgr*  pInstance_;
    ID3D11Device*       pDevice_ = nullptr;

    cvector<TexID>      ids_;                 // SORTED array of unique IDs
    cvector<SRV*>       shaderResourceViews_;
    cvector<TexName>    names_;               // name (there can be path) which is used for searching of texture
    cvector<Texture>    textures_;
};


// =================================================================================
// Declare a global instance of the texture manager
// =================================================================================
extern TextureMgr g_TextureMgr;

} // namespace Core
