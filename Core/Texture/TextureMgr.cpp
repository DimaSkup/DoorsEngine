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
#include <CoreCommon/FileSystem.h>

#include "ImageReader.h"


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
    //AddDefaultTex("unloaded",          { pDevice, Colors::UnloadedTextureColor });
    //AddDefaultTex("unhandled_texture", { pDevice, Colors::UnhandledTextureColor });
}

///////////////////////////////////////////////////////////

void TextureMgr::SetTexName(const TexID id, const char* inName)
{
    // update a name of the texture by id

    if (IsNameEmpty(inName))
    {
        LogErr("input name is empty");
        return;
    }

    const index idx = ids_.get_idx(id);
    const bool exist = (ids_[idx] == id);

    if (!exist)
    {
        sprintf(g_String, "there is no texture by ID: %ud", id);
        LogErr(g_String);
        return;
    }

    // length of texture name cannot be bigger than MAX_LENGTH_TEXTURE_NAME
    size_t sz = strlen(inName);
    sz = (sz > MAX_LENGTH_TEXTURE_NAME) ? MAX_LENGTH_TEXTURE_NAME : sz;  

    // update name
    names_[idx] = inName;
    //strncpy(names_[idx].name, inName, sz);

    textures_[idx].SetName(inName);
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
        LogErr(e);
        throw EngineException("can't add a texture by name: " + name);
    }
}
#endif

///////////////////////////////////////////////////////////

TexID TextureMgr::Add(const char* name, Texture&& tex)
{
    // add a new texture by name and return its generated ID
    try
    {
        Assert::True(!IsNameEmpty(name), "a texture name cannot be empty");

        TexID id = INVALID_TEXTURE_ID;

        // if there is already a texture with such name we just return its ID
        id = GetIDByName(name);
        if (id != 0)
            return id;


        // else we add a new texture
        id = GenID();

        ids_.push_back(id);
        names_.push_back(name);

        textures_.push_back(std::move(tex));
        shaderResourceViews_.push_back(textures_.back().GetTextureResourceView());

        // return an ID of the added texture
        return id;
    }
    catch (EngineException& e)
    {
        LogErr(e);
        sprintf(g_String, "can't add a texture by name: %s", name);
        throw EngineException(g_String);
    }
}

///////////////////////////////////////////////////////////

TexID TextureMgr::LoadFromFile(const char* dirPath, const char* texturePath)
{
    // return an ID to the texture which is loaded from the file by input path
    //
    // input: dirPath     -- directory relatively to the project working directory
    //        texturePath -- path to texture relatively to the dirPath


    // check input params
    if ((dirPath == nullptr) || (dirPath[0] == '\0'))
    {
        LogErr("input path to directory is empty!");
        return INVALID_TEXTURE_ID;
    }

    if ((texturePath == nullptr) || (texturePath[0] == '\0'))
    {
        LogErr("input path to texture is empty!");
        return INVALID_TEXTURE_ID;
    }

    // create a full path to the texture (relatively to the project working directory) and load this texture
    sprintf(g_String, "%s%s", dirPath, texturePath);
    return LoadFromFile(g_String);
}

///////////////////////////////////////////////////////////

TexID TextureMgr::LoadFromFile(const char* path)
{
    // return an ID to the texture which is loaded from the file by input path

    try
    {
#if DEBUG || _DEBUG
        if (!FileSys::Exists(path))
            return INVALID_TEXTURE_ID;
#endif

        TexID id = INVALID_TEXTURE_ID;

        // if there is already such a texture we just return its ID
        id = GetIDByName(path);
        if (id != 0)
            return id;


        // else we create a new texture from file
        id = GenID();

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
        sprintf(g_String, "can't create a texture from file: %s", path);
        LogErr(e);
        return INVALID_TEXTURE_ID;
    }
}

///////////////////////////////////////////////////////////

TexID TextureMgr::LoadTextureArray(
    const char* textureObjName,
    const std::string* texturePaths,
    const size numTextures,
    const DXGI_FORMAT format)
{
    try
    {
        Assert::True(!IsNameEmpty(textureObjName), "input name for the texture object is empty");
        Assert::True(texturePaths,                 "input ptr to arr of filenames == nullptr");
        Assert::True(numTextures > 0,              "input number of texture must be > 0");

        // check if files exist
        for (index i = 0; i < numTextures; ++i)
        {
            if (!FileSys::Exists(texturePaths[0].c_str()))
                return INVALID_TEXTURE_ID;
        }

        // create a texture array object
        Texture texArr(pDevice_, textureObjName, texturePaths, (int)numTextures, format);

        const TexID id = GenID();

        // store data into the texture manager
        ids_.push_back(id);
        names_.push_back(texArr.GetName());
        shaderResourceViews_.push_back(texArr.GetTextureResourceView());
        textures_.push_back(std::move(texArr));

        return id;
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        LogErr("can't create texture 2D array");
        return INVALID_TEXTURE_ID;
    }
}

///////////////////////////////////////////////////////////

TexID TextureMgr::CreateWithColor(const Color& color)
{
    // if there is already a color texture by such ID we just return an ID to it;
    // or in another case we create this texture and return an ID to this new texture;
    
    // generate a name for this texture
    sprintf(g_String, "color_texture_%d_%d_%d", color.GetR(), color.GetG(), color.GetB());
    TexID id = GetIDByName(g_String);

    return (id != INVALID_TEXTURE_ID) ? id : Add(g_String, Texture(pDevice_, color));
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

Texture* TextureMgr::GetTexPtrByName(const char* name)
{
    // return a ptr to the texture by name or nullptr if there is no such a texture

    if (IsNameEmpty(name))
    {
        LogErr("input name of texture is empty!");
        return nullptr;
    }

    const index idx = names_.find(name);
    return (idx != -1) ? &textures_[idx] : &textures_[0];
}


// =================================================================================
// Getters: get texture ID / textures IDs array by name/names
// =================================================================================
TexID TextureMgr::GetIDByName(const char* inName)
{
    // return an ID of texture object by input name;
    // if there is no such a textures we return 0;

    for (index i = 0; i < names_.size(); ++i)
    {
        if (strcmp(names_[i].c_str(), inName) == 0)
            return ids_[i];
    }

    // if we didn't find any ID by name we return 0
    return INVALID_TEXTURE_ID;
}

///////////////////////////////////////////////////////////

TexID TextureMgr::GetTexIdByIdx(const index idx) const
{
    // return ID by input idx; if idx is invalid we return 0

    const bool isValidIdx = (idx >= 0 && idx < ids_.size());
    return (isValidIdx) ? ids_[idx] : INVALID_TEXTURE_ID;
}

///////////////////////////////////////////////////////////
#if 0
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
#endif

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
void TextureMgr::AddDefaultTex(const char* name, Texture&& tex)
{
    // add some default texture into the TextureMgr and
    // set for it a specified id

    ids_.push_back(GenID());
    names_.push_back(name);
    textures_.push_back(std::move(tex));
    shaderResourceViews_.push_back(textures_.back().GetTextureResourceView());
}

} // namespace Core
