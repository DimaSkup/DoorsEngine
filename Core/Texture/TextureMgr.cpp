// =================================================================================
// Filename:      TextureMgr.cpp
// Description:   a manager for work with textures: initialization of
//                ALL the textures, getting it and releasing;
// Created:       06.06.23
// =================================================================================
#include <CoreCommon/pch.h>
#include "../Render/d3dclass.h"
#include "TextureMgr.h"
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
    CAssert::NotNullptr(pDevice, "ptr to the device == nullptr");
    pDevice_ = pDevice;

    // create and store a couple of default textures
    //LoadFromFile(g_TexDirPath + "notexture.dds");
    //AddDefaultTex("unloaded",          { pDevice, Colors::UnloadedTextureColor });
    //AddDefaultTex("unhandled_texture", { pDevice, Colors::UnhandledTextureColor });
}

// --------------------------------------------------------
// Desc:   convert input image raw data into 32 bits per pixel data
// Args:   - data:      actual pixels data (one element per channel)
//         - width:     the texture width
//         - height:    the texture height
//         - bpp:       bits per pixel (24 or 8)
// Out:    - outData:   array of converted data
// --------------------------------------------------------
void ConvertInto32bits(
    const uint8* data,
    const uint width,
    const uint height,
    const int bpp,
    cvector<uint8>& outData)
{
    const uint bytesPerPixel = 4;
    const uint numPixels     = width * height;
    const uint sizeInBytes   = numPixels * bytesPerPixel;

    // alloc memory for the output 32 bits image data
    outData.resize(sizeInBytes, 0);

    // convert 24 bits => 32 bits
    if (bpp == 24)
    {
        for (int i = 0, i1 = 0, i2 = 0; i < (int)numPixels; i++, i1 = i * 4, i2 = i * 3)
        {
            // convert from RGB to RGBA
            outData[i1 + 0] = data[i2 + 0];        // R
            outData[i1 + 1] = data[i2 + 1];        // G
            outData[i1 + 2] = data[i2 + 2];        // B
            outData[i1 + 3] = 255;                 // A (255 because we use uint8)
        }
    }
    // convert 8 bits => 32 bits (it will be a grayscale image)
    else if (bpp == 8)
    {
        for (int i = 0, i1 = 0, i2 = 0; i < (int)numPixels; i++, i1 = i * 4, i2++)
        {
            // convert from RGB to RGBA
            outData[i1 + 0] = data[i2 + 0];        // R
            outData[i1 + 1] = data[i2 + 0];        // G
            outData[i1 + 2] = data[i2 + 0];        // B
            outData[i1 + 3] = 255;                 // A (255 because we use uint8)
        }
    }
    else
    {
        LogErr(LOG, "wrong number of bits per pixel (expected: 8 or 24; received: %d)", bpp);
    }
}

// --------------------------------------------------------
// Desc:   create a new DirectX texture with the image's input raw data
// Args:   - name:      a name for texture identification
//         - data:      actual pixels data (one element per channel)
//         - width:     the texture width
//         - height:    the texture height
//         - bpp:       bits per pixel (8, 24 or 32)
//         - mipMapped: defines if we will generate mipmaps of not
// Ret:    an ID of created texture (for details look at TextureMgr)
// --------------------------------------------------------
TexID TextureMgr::CreateTextureFromRawData(
    const char* name,
    const uint8* data,
    const uint width,
    const uint height,
    const int bpp,
    const bool mipMapped)
{
    uint8* initData = nullptr;
    cvector<uint8> convertedData;

    try
    {
        // check input params
        CAssert::True(!StrHelper::IsEmpty(name),          "input name is empty");
        CAssert::True(data,                               "input ptr to image data array == nullptr");
        CAssert::True(bpp == 8 || bpp == 24 || bpp == 32, "input number of bits per pixel must be equal to 24 or 32");

        // if we want to create mipmaps
        if (mipMapped)
        {
            CAssert::True(IsPow2(width),  "input width must be a power of 2");
            CAssert::True(IsPow2(height), "input height must be a power of 2");
        }
        // create a texture with only one mipmap
        else
        {
            CAssert::True(width & height,  "input width and height must be > 0");
            CAssert::True(width == height, "input width != height (for square texture)");
        }


        // we already have a 32 bits image
        if (bpp == 32)
        {
            initData = (uint8*)data;
        }
        // we need to convert from 8 or 24 bits into 32 bits image data
        else
        {
            ConvertInto32bits(data, width, height, bpp, convertedData);
            initData = convertedData.data();
        }

        // create a DirectX texture
        Texture texture(g_pDevice, name, initData, width, height, mipMapped);

        // move texture into the textures manager and return an ID of the texture
        return Add(name, std::move(texture));
    }
    catch (std::bad_alloc& e)
    {
        LogErr(e.what());
        LogErr(LOG, "can't allocate memory for the texture: %s", name);
        return INVALID_TEXTURE_ID;
    }
    catch (EngineException& e)
    {
        LogErr(e);
        return INVALID_TEXTURE_ID;
    }
}

// --------------------------------------------------------
// Desc:   recreate a DirectX texture with the image's input raw data
//         (release the previous data and init with new)
// Args:   - name:      a name for texture identification
//         - data:      actual pixels data (one element per channel)
//         - width:     the texture width
//         - height:    the texture height
//         - bpp:       bits per pixel (24 or 32)
//         - mipMapped: defines if we will generate mipmaps of not
//         - inOutTex:  texture object which will be filled with new pixel data
// --------------------------------------------------------
void TextureMgr::RecreateTextureFromRawData(
    const char* name,
    const uint8* data,
    const uint width,
    const uint height,
    const int bpp,
    const bool mipMapped,
    Texture& inOutTex)
{
    uint8* initData = nullptr;
    cvector<uint8> convertedData;
   
    try
    {
        // check input params
        CAssert::True(!StrHelper::IsEmpty(name),          "input name is empty");
        CAssert::True(data,                               "input ptr to image data array == nullptr");
        CAssert::True(IsPow2(width),                      "input width must be a power of 2");
        CAssert::True(IsPow2(height),                     "input height must be a power of 2");
        CAssert::True(bpp == 8 || bpp == 24 || bpp == 32, "input number of bits per pixel must be equal to 24 or 32");

        // we already have a 32 bits image
        if (bpp == 32)
        {
            initData = (uint8*)data;
        }
        // we need to convert from 8 or 24 bits into 32 bits image data
        else
        {
            ConvertInto32bits(data, width, height, bpp, convertedData);
            initData = convertedData.data();
        }

        // get idx of this texture in the texture manager
        const TexID id  = GetTexIdByName(inOutTex.GetName().c_str());
        const index idx = ids_.get_idx(id);

        // init texture with raw data
        if (!inOutTex.Initialize(g_pDevice, name, initData, width, height, mipMapped))
        {
            sprintf(g_String, "can't initialize texture: %s", name);
            throw EngineException(g_String);
        }

        // update shader resource view by this texture
        shaderResourceViews_[idx] = inOutTex.GetTextureResourceView();
    }
    catch (std::bad_alloc& e)
    {
        LogErr(e.what());
        LogErr(LOG, "can't allocate memory for the terrain texture: %s", name);
    }
    catch (EngineException& e)
    {
        LogErr(e);
    }
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

    if (ids_[idx] != id)
    {
        LogErr(LOG, "there is no texture by ID: %ud", id);
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
        CAssert::NotEmpty(name.empty(), "a texture name (path) cannot be empty");

        // if there is already a texture by such name we just return its ID
        const bool isNotUniqueName = (names_.find(name) != -1);

        if (isNotUniqueName)
            return GetTexIdByName(name);


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
        CAssert::True(!IsNameEmpty(name), "a texture name cannot be empty");

        TexID id = INVALID_TEXTURE_ID;

        // if there is already a texture with such name we just return its ID
        id = GetTexIdByName(name);
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

//---------------------------------------------------------
// Desc:  load a texture from file and create a texture resource with it
// Args:  - dirPath:     a directory with textures relatively to the project working directory
//        - texturePath: path to texture relatively to the dirPath
// Ret:   an ID to the loaded texture
//---------------------------------------------------------
TexID TextureMgr::LoadFromFile(const char* dirPath, const char* texturePath)
{
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
    memset(g_String, 0, LOG_BUF_SIZE);
    strcat(g_String, dirPath);
    strcat(g_String, texturePath);
    return LoadFromFile(g_String);
}

//---------------------------------------------------------
// Desc:  load a texture from file and create a texture resource with it
// Args:  - path:  full path to the texture file
// Ret:   an ID to the loaded texture
//---------------------------------------------------------
TexID TextureMgr::LoadFromFile(const char* path)
{
    try
    {
        if ((path == nullptr) || (path[0] == '\0'))
        {
            LogErr("input path to texture is empty!");
            return INVALID_TEXTURE_ID;
        }

#if DEBUG || _DEBUG
        if (!FileSys::Exists(path))
            return INVALID_TEXTURE_ID;
#endif

        // if there is already such a texture we just return its ID
        TexID id = GetTexIdByName(path);
        if (id != 0)
            return id;


        // else we create a new texture from file
        id = GenID();

        char name[64]{'\0'};
        FileSys::GetFileStem(path, name);

        ids_.push_back(id);
        names_.push_back(name);
        textures_.push_back(std::move(Texture(pDevice_, path, name)));
        shaderResourceViews_.push_back(textures_.back().GetTextureResourceView());

        // check if we successfully created this texture
        bool isSuccess = (textures_.back().GetTextureResourceView() != nullptr);
        
        return (isSuccess) ? id : INVALID_TEXTURE_ID;	
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr(LOG, "can't load a texture from file: %s", path);
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
        CAssert::True(!IsNameEmpty(textureObjName), "input name for the texture object is empty");
        CAssert::True(texturePaths,                 "input ptr to arr of filenames == nullptr");
        CAssert::True(numTextures > 0,              "input number of texture must be > 0");

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
    TexID id = GetTexIdByName(g_String);

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
TexID TextureMgr::GetTexIdByName(const char* inName)
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

    CAssert::True(inNames != nullptr, "input ptr to names arr == nullptr");
    CAssert::True(numNames > 0,       "input number of names must be > 0");

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

    CAssert::True(texIDs != nullptr, "input ptr to textures IDs arr == nullptr");
    CAssert::True(numTex > 0, "input number of textures must be > 0");

    // get idxs to textures IDs which != 0
    idxsToNotZero_.resize(numTex);
    int pos = 0;

    for (index idx = 0; idx < numTex; ++idx)
    {
        idxsToNotZero_[pos] = idx;
        pos += bool(texIDs[idx]);
    }

    idxsToNotZero_.resize(pos + 1);

    // ---------------------------------------------

    // get SRVs
    ID3D11ShaderResourceView* unloadedTexSRV = shaderResourceViews_[0];
    outSRVs.resize(numTex, unloadedTexSRV);

    const size numPreparedTextures = std::ssize(idxsToNotZero_);
    texIds_.resize(numPreparedTextures);
    tempIdxs_.resize(numPreparedTextures);
    preparedSRVs_.resize(numPreparedTextures);

    for (index i = 0; const index idx : idxsToNotZero_)
        texIds_[i++] = texIDs[idx];

    // get idxs of searched textures ids
    ids_.get_idxs(texIds_.data(), numPreparedTextures, tempIdxs_);

    for (int i = 0; const index idx : tempIdxs_)
        preparedSRVs_[i++] = shaderResourceViews_[idx];

    for (index i = 0; const index idx : idxsToNotZero_)
        outSRVs[idx] = preparedSRVs_[i++];
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
