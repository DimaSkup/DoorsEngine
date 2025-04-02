// =================================================================================
// Filename:      TextureMgr.cpp
// Description:   a manager for work with textures: initialization of
//                ALL the textures, getting it and releasing;
// Created:       06.06.23
// =================================================================================
#include "TextureMgr.h"

#include <CoreCommon/FileSystemPaths.h>
#include <CoreCommon/MemHelpers.h>
#include <CoreCommon/log.h>
#include <CoreCommon/Assert.h>
#include <CoreCommon/StringHelper.h>

#include "ImageReader.h"

#include <stdexcept>
#include <filesystem>

namespace fs = std::filesystem;



namespace Core
{

// init a global instance of the texture manager
TextureMgr g_TextureMgr;

// initialize a static pointer to this class instance
TextureMgr* TextureMgr::pInstance_ = nullptr;

// we use this value as ID for each created/added texture
TexID TextureMgr::lastTexID_ = 0;


///////////////////////////////////////////////////////////

TextureMgr::TextureMgr()
{
    if (pInstance_ == nullptr)
    {
        pInstance_ = this;	

        // reserve some memory ahead
        constexpr size reserveForTexCount = 128;
        ids_.reserve(reserveForTexCount);
        names_.reserve(reserveForTexCount);
        textures_.reserve(reserveForTexCount);
        shaderResourceViews_.reserve(reserveForTexCount);

        // setup the static logger in the image reader module
        ImgReader::ImageReader imgReader;
        imgReader.SetupLogger(Log::GetFilePtr(), &Log::GetLogMsgsList());
    }
    else
    {
        throw EngineException("you can't have more that only one instance of this class");
    }
}

///////////////////////////////////////////////////////////

TextureMgr::~TextureMgr()
{
    ids_.clear();
    names_.clear();
    textures_.clear();   
    shaderResourceViews_.clear();
    
    pInstance_ = nullptr;
}


// =================================================================================
// Public API: initialization/adding/loading/creation
// =================================================================================
void TextureMgr::Initialize(ID3D11Device* pDevice)
{
    // check input params
    Assert::NotNullptr(pDevice, "ptr to the device == nullptr");
    pDevice_ = pDevice;

    // create and store a couple of default textures
    //LoadFromFile(g_TexDirPath + "notexture.dds");
    AddDefaultTex("unloaded", { pDevice, Colors::UnloadedTextureColor });
    AddDefaultTex("unhandled_texture", { pDevice, Colors::UnhandledTextureColor });
}

///////////////////////////////////////////////////////////

#if 0
TexID TextureMgr::Add(const TexName& name, Texture& tex)
{
    // add a new texture by name and return its generated ID
    try
    {
        Assert::NotEmpty(name.empty(), "a texture name (path) cannot be empty");

        // if there is already a texture by such name we just return its ID
        const bool isNotUniqueName = (names_.find(name) != -1);

        if (isNotUniqueName)
            return GetIDByName(name);


        // else we add a new texture
        const TexID id = GenID();

        ids_.push_back(id);
        names_.push_back(name);
        textures_.push_back(std::move(tex));
        shaderResourceViews_.push_back(textures_.back().GetTextureResourceView());

        // return an id of the added texture
        return id;
    }
    catch (EngineException& e)
    {
        Log::Error(e);
        throw EngineException("can't add a texture by name: " + name);
    }
}
#endif

///////////////////////////////////////////////////////////

TexID TextureMgr::Add(const TexName& name, Texture&& tex)
{
    // add a new texture by name and return its generated ID
    try
    {
        Assert::NotEmpty(name.empty(), "a texture name (path) cannot be empty");

        // if there is already a texture with such name we just search and return its ID
        const bool isNotUniqueName = (names_.find(name) != -1);

        if (isNotUniqueName)
            return GetIDByName(name);

        // else we add a new texture
        const TexID id = GenID();

        ids_.push_back(id);
        names_.push_back(name);
        textures_.push_back(std::move(tex));
        shaderResourceViews_.push_back(textures_.back().GetTextureResourceView());

        // return an ID of the added texture
        return id;
    }
    catch (EngineException& e)
    {
        Log::Error(e);
        throw EngineException("can't add a texture by name: " + name);
    }
}

///////////////////////////////////////////////////////////

TexID TextureMgr::LoadFromFile(const TexPath& path)
{
    // return an ID to the texture which is loaded from the file by input path

    try
    {
        const fs::path texPath = path;

        // check if such texture file exists
        if (!fs::exists(texPath))
        {
            Log::Error("there is no texture by path: " + path);
            return INVALID_TEXTURE_ID;
        }

        // if there is such a texture we just return its ID
        if (names_.find(path) != -1)
            return GetIDByName(path);


        // else we create a new texture from file
        const TexID id = GenID();

        ids_.push_back(id);
        names_.push_back(path);
        textures_.push_back(std::move(Texture(pDevice_, path)));
        shaderResourceViews_.push_back(textures_.back().GetTextureResourceView());

        // check if we successfully created this texture
        bool isSuccess = (textures_.back().GetTextureResourceView() != nullptr);
        
        return (isSuccess) ? id : INVALID_TEXTURE_ID;	
    }
    catch (EngineException& e)
    {
        Log::Error(e);
        throw EngineException("can't create a texture from file: " + path);
    }
}

///////////////////////////////////////////////////////////

TexID TextureMgr::LoadTextureArray(
    const std::string* textureNames,
    const size numTextures,
    const DXGI_FORMAT format)
{
    try
    {
        Assert::True(textureNames != nullptr, "input ptr to arr of filenames == nullptr");
        Assert::True(numTextures > 0,         "input number of texture must be > 0");

        // check if files exist
        for (index i = 0; i < numTextures; ++i)
        {
            // if such texture file doesn't exist we return an of invalid texture ID
            if (!fs::exists(fs::path(textureNames[i])))
            {
                Log::Error("there is no texture by path: " + textureNames[i]);
                return INVALID_TEXTURE_ID;  
            }
        }

        const TexID id = GenID();
      
        // create a texture array object
        Texture texArr(
            pDevice_,
            "texture_array" + std::to_string(id),
            textureNames,
            (int)numTextures,
            format);


        // store data into the texture manager
        ids_.push_back(id);
        names_.push_back(texArr.GetName());
        shaderResourceViews_.push_back(texArr.GetTextureResourceView());
        textures_.push_back(std::move(texArr));

        return id;
    }
    catch (EngineException& e)
    {
        Log::Error(e, true);
        throw EngineException("can't create texture 2D array");
    }
}

///////////////////////////////////////////////////////////

TexID TextureMgr::CreateWithColor(const Color& color)
{
    // if there is already a color texture by such ID we just return an ID to it;
    // or in another case we create this texture and return an ID to this new texture;
    
    // generate a name for this texture
    const std::string name = std::format("color_texture_{}_{}_{}", color.GetR(), color.GetG(), color.GetB());
    TexID id = GetIDByName(name);

    return (id != INVALID_TEXTURE_ID) ? id : Add(name, Texture(pDevice_, color));
}


// =================================================================================
// Getters: get texture reference or texture pointer by ID/name
// =================================================================================
Texture& TextureMgr::GetTexByID(const TexID id)
{
    // return a texture by input id or return an unloaded texture (id: 0) if there is no texture by such ID
    const index idx = ids_.get_idx(id);
    const bool exist = (ids_[idx] == id);

    return textures_[idx * exist];
}

///////////////////////////////////////////////////////////

Texture* TextureMgr::GetTexPtrByID(const TexID id)
{
    // return a ptr to the texture by ID or nullptr if there is no such a texture
    const index idx = ids_.get_idx(id);
    const bool exist = (ids_[idx] == id);

    return &textures_[idx * exist];
}

///////////////////////////////////////////////////////////

Texture* TextureMgr::GetTexPtrByName(const TexName& name)
{
    // return a ptr to the texture by name or nullptr if there is no such a texture
    const index idx = names_.find(name);
    return (idx != -1) ? &textures_[idx] : nullptr;
}


// =================================================================================
// Getters: get texture ID / textures IDs array by name/names
// =================================================================================
TexID TextureMgr::GetIDByName(const TexName& name)
{
    // return an ID of texture object by input name;
    // if there is no such a textures we return 0;

    const index idx = names_.find(name);
    return (idx != -1) ? ids_[idx] : INVALID_TEXTURE_ID;
}

///////////////////////////////////////////////////////////

void TextureMgr::GetIDsByNames(
    const TexName* inNames,
    const size numNames,
    cvector<TexID>& outIDs)
{
    // get textures IDs by its names

    Assert::True(inNames != nullptr, "input ptr to names arr == nullptr");
    Assert::True(numNames > 0,       "input number of names must be > 0");

    // get idxs by names
    cvector<index> idxs(numNames);

    for (index i = 0; i < numNames; ++i)
        idxs[i] = names_.find(inNames[i]);

    // get IDs by idxs
    outIDs.resize(numNames);

    for (int i = 0; const index idx : idxs)
    {
        const bool exist = (idx != -1);
        outIDs[i++] = ids_[idx * (size)exist];
    }
}

///////////////////////////////////////////////////////////

ID3D11ShaderResourceView* TextureMgr::GetSRVByTexID(const TexID texID)
{
    const index idx  = ids_.get_idx(texID);
    const bool exist = (ids_[idx] == texID);  // check if we found the proper idx

    return shaderResourceViews_[idx * exist];
}

///////////////////////////////////////////////////////////

void TextureMgr::GetSRVsByTexIDs(
    const TexID* texIDs,
    const size numTex,
    cvector<ID3D11ShaderResourceView*>& outSRVs)
{
    // here get SRV (shader resource view) of each input texture by its ID

    Assert::True(texIDs != nullptr, "input ptr to textures IDs arr == nullptr");
    Assert::True(numTex > 0,        "input number of textures must be > 0");

    // get idxs to textures IDs which != 0
    cvector<index> idxsToNotZero(numTex);
    int pos = 0;

    for (index idx = 0; idx < numTex; ++idx)
    {
        idxsToNotZero[pos] = idx;
        pos += bool(texIDs[idx]);
    }

    idxsToNotZero.resize(pos + 1);

    // ---------------------------------------------

    // get SRVs
    ID3D11ShaderResourceView* unloadedTexSRV = shaderResourceViews_[0];
    outSRVs.resize(numTex, unloadedTexSRV);

    const size numPreparedTextures = std::ssize(idxsToNotZero);
    cvector<TexID> ids(numPreparedTextures);
    cvector<index> idxs(numPreparedTextures);
    cvector<SRV*>  preparedSRVs(numPreparedTextures);
    
    for (index i = 0; const index idx : idxsToNotZero)
        ids[i++] = texIDs[idx];

    // get idxs of searched textures ids
    ids_.get_idxs(ids.data(), numPreparedTextures, idxs);

    for (int i = 0; const index idx : idxs)
        preparedSRVs[i++] = shaderResourceViews_[idx];

    for (index i = 0; const index idx : idxsToNotZero)
        outSRVs[idx] = preparedSRVs[i++];
}

///////////////////////////////////////////////////////////

#if 0
void TextureMgr::GetAllTexturesPathsWithinDirectory(
    const std::string& pathToDir,
    std::vector<TexPath>& outPaths)
{
    // get an array of paths to textures in the directory by pathToDir

    const std::string extentions[4]{".dds", ".tga", ".png", ".bmp"};

    // go through each file in the directory
    for (int i = 0; const fs::directory_entry& entry : fs::directory_iterator(pathToDir))
    {
        const char* ext = entry.path().extension().string().c_str();

        // check if we have a valid extention
        bool isValidExt = false;
        isValidExt |= strcmp(ext, extentions[0].c_str());
        isValidExt |= strcmp(ext, extentions[1].c_str());
        isValidExt |= strcmp(ext, extentions[2].c_str());
        isValidExt |= strcmp(ext, extentions[3].c_str());

        
        if (isValidExt)
        {
            const std::string& path = pathToDir[i];
            std::replace(path.begin(), path.end(), '\\', '/');  // in the pass change from '\\' into '/' symbol

            outPaths.emplace_back(path);
        }

        ++i;
    }
}
#endif


// =================================================================================
//                              PRIVATE HELPERS
// =================================================================================
void TextureMgr::AddDefaultTex(const TexName& name, Texture&& tex)
{
    // add some default texture into the TextureMgr and
    // set for it a specified id

    try
    {
        Assert::True(!name.empty(), "input name is empty");

        const bool isUniqueName = !names_.has_value(name);
        Assert::True(isUniqueName, "input name is not unique: " + name);


        const TexID id = GenID();

        ids_.push_back(id);
        names_.push_back(name);
        textures_.push_back(std::move(tex));
        shaderResourceViews_.push_back(textures_.back().GetTextureResourceView());
    }
    catch (EngineException& e)
    {
        Log::Error(e);
        throw EngineException("can't add a texture by name: " + name);
    }
}

} // namespace Core
