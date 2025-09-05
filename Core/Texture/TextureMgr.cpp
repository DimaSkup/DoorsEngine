// =================================================================================
// Filename:      TextureMgr.cpp
// Description:   a manager for work with textures: initialization of
//                ALL the textures, getting it and releasing;
// Created:       06.06.23
// =================================================================================
#include <CoreCommon/pch.h>
#include <Render/d3dclass.h>    // for using global pointers to DX11 device and context
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
        Texture texture(Render::g_pDevice, name, initData, width, height, mipMapped);

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
        if (!inOutTex.Initialize(Render::g_pDevice, name, initData, width, height, mipMapped))
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

//---------------------------------------------------------
// Desc:  set a name of the texture by id
// Args:  - id:      texture identifier
//        - inName:  new name for the texture
//---------------------------------------------------------
void TextureMgr::SetTexName(const TexID id, const char* inName)
{

    if (StrHelper::IsEmpty(inName))
    {
        LogErr("input name is empty");
        return;
    }

    const index idx = ids_.get_idx(id);

    if (ids_[idx] != id)
    {
        LogErr(LOG, "there is no texture by ID: %ld", id);
        return;
    }

    // length of texture name cannot be bigger than MAX_LENGTH_TEXTURE_NAME
    size_t sz = strlen(inName);
    sz = (sz > MAX_LENGTH_TEXTURE_NAME) ? MAX_LENGTH_TEXTURE_NAME : sz;  

    // update name
    names_[idx] = inName;
    textures_[idx].SetName(inName);
}

//---------------------------------------------------------
// Desc:   add a new texture and return its generated ID
// Args:   - name:  a name for the new texture
//         - tex:   a texture obj which will be moved into the manager
// Ret:    identifier of added texture
//---------------------------------------------------------
TexID TextureMgr::Add(const char* name, Texture&& tex)
{
    try
    {
        if (StrHelper::IsEmpty(name))
        {
            LogErr(LOG, "input texture name is empty");
            return INVALID_TEXTURE_ID;
        }

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
            LogErr(LOG, "input path to texture is empty!");
            return INVALID_TEXTURE_ID;
        }

        // if there is already such a texture we just return its ID
        TexID id = GetTexIdByName(path);
        if (id != 0)
            return id;


        // else we create a new texture from file
        char name[64]{ '\0' };
        FileSys::GetFileStem(path, name);

        Texture tex(pDevice_, path, name);

        id = GenID();
        ids_.push_back(id);
        names_.push_back(name);
        textures_.push_back(std::move(tex));
        shaderResourceViews_.push_back(textures_.back().GetTextureResourceView());
        
        return id;	
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr(LOG, "can't load a texture from file: %s", path);
        return INVALID_TEXTURE_ID;
    }
}

//---------------------------------------------------------
// Desc:   reinit a texture object by id with a new texture resource
//         (load another texture from file)
// Args:   - id:    texture identifier
//         - path:  path to texture file
//---------------------------------------------------------
bool TextureMgr::ReloadFromFile(const TexID id, const char* path)
{
    try
    {
        if ((path == nullptr) || (path[0] == '\0'))
        {
            LogErr(LOG, "input path to texture is empty!");
            return false;
        }

        // find an index to texture data and check if it is valid
        const index idx = ids_.get_idx(id);
        
        if (ids_[idx] != id)
        {
            LogErr(LOG, "there is no texture by id: %" PRIu32, id);
            return false;
        }

        // release all the prev data
        Texture& tex = textures_[id];
        tex.Release();

        // generate a new name for texture
        char name[64]{ '\0' };
        FileSys::GetFileStem(path, name);

        // reinit
        tex.LoadFromFile(Render::g_pDevice, path);
        tex.SetName(name);

        // update data
        names_[idx] = name;
        shaderResourceViews_[idx] = tex.GetTextureResourceView();

        return true;
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr(LOG, "can't load a texture from file: %s", path);
        return false;
    }
}

//---------------------------------------------------------
// Desc:   load 6 textures from directory by directoryPath
//         and create a single cube map texture from them
// Args:   - name:   just a name for this texture (can be used when search for this texture)
//         - params: initial parameters for cubemap
// Ret:    identifier of created texture or 0 if we failed
//---------------------------------------------------------
TexID TextureMgr::CreateCubeMap(const char* name, const CubeMapInitParams& params)
{
    // if there is already such a texture we just return its ID
    TexID id = GetTexIdByName(name);

    if (id != INVALID_TEXTURE_ID)
        return id;

    // create a cubemap texture
    Texture cubeMap;
    if (!cubeMap.CreateCubeMap(name, params))
    {
        cubeMap.Release();
        return INVALID_TEXTURE_ID;
    }

    id = GenID();

    // store cubemap into the manager
    ids_.push_back(id);
    names_.push_back(name);
    textures_.push_back(std::move(cubeMap));
    shaderResourceViews_.push_back(textures_.back().GetTextureResourceView());

    return id;
}

///////////////////////////////////////////////////////////

TexID TextureMgr::LoadTextureArray(
    const char* name,
    const std::string* texturePaths,
    const size numTextures,
    const DXGI_FORMAT format)
{
    try
    {
        CAssert::True(!StrHelper::IsEmpty(name), "input name for the texture object is empty");
        CAssert::True(texturePaths,              "input ptr to arr of filenames == nullptr");
        CAssert::True(numTextures > 0,           "input number of texture must be > 0");

        // check if files exist
        for (index i = 0; i < numTextures; ++i)
        {
            if (!FileSys::Exists(texturePaths[0].c_str()))
                return INVALID_TEXTURE_ID;
        }

        // create a texture array object
        Texture texArr(pDevice_, name, texturePaths, (int)numTextures, format);

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

//---------------------------------------------------------
// Desc:   create a texture filled with input color
// Args:   - color:  a color which will be used to fill in the whole texture
//---------------------------------------------------------
TexID TextureMgr::CreateWithColor(const Color& color)
{
    // generate a name for this texture
    sprintf(g_String, "color_texture_%d_%d_%d", color.GetR(), color.GetG(), color.GetB());
    TexID id = GetTexIdByName(g_String);

    return (id != INVALID_TEXTURE_ID) ? id : Add(g_String, Texture(pDevice_, color));
}

//---------------------------------------------------------
// return a texture by input id or return an unloaded texture (id: 0) if there is no texture by such ID
//---------------------------------------------------------
Texture& TextureMgr::GetTexByID(const TexID id)
{
    const index idx = ids_.get_idx(id);
    const bool exist = (ids_[idx] == id);

    return textures_[idx * exist];
}

//---------------------------------------------------------
// Desc:   return a ptr to the texture by ID or nullptr if there is no such a texture
//---------------------------------------------------------
Texture* TextureMgr::GetTexPtrByID(const TexID id)
{
    const index idx = ids_.get_idx(id);
    const bool exist = (ids_[idx] == id);

    return &textures_[idx * exist];
}

//---------------------------------------------------------
// Desc:   return a ptr to the texture by name or nullptr if there is no such a texture
//---------------------------------------------------------
Texture* TextureMgr::GetTexPtrByName(const char* name)
{
    if (StrHelper::IsEmpty(name))
    {
        LogErr("input name of texture is empty!");
        return nullptr;
    }

    const index idx = names_.find(name);
    return (idx != -1) ? &textures_[idx] : &textures_[0];
}

//---------------------------------------------------------
// Desc:    return a texture ID of texture object by input name;
//         if there is no such a textures we return 0;
//---------------------------------------------------------
TexID TextureMgr::GetTexIdByName(const char* inName)
{
    for (index i = 0; i < names_.size(); ++i)
    {
        if (strcmp(names_[i].c_str(), inName) == 0)
            return ids_[i];
    }

    return INVALID_TEXTURE_ID;
}

//---------------------------------------------------------
// Desc:   return a texture ID by input idx; if idx is invalid we return 0
//---------------------------------------------------------
TexID TextureMgr::GetTexIdByIdx(const index idx) const
{
    const bool isValidIdx = (idx >= 0 && idx < ids_.size());
    return (isValidIdx) ? ids_[idx] : INVALID_TEXTURE_ID;
}

//---------------------------------------------------------
// Desc:   get shader resource view of texture by input ID
//---------------------------------------------------------
ID3D11ShaderResourceView* TextureMgr::GetTexViewsById(const TexID texID)
{
    const index idx  = ids_.get_idx(texID);

    // if there is no texture by ID we return shader resource view by idx == 0
    return shaderResourceViews_[idx * (ids_[idx] == texID)];
}

//---------------------------------------------------------
// Desc:    get SRV (shader resource view) of each input texture by its ID
// Args:    - texIds:  textures identifiers
//          - numTex:  how many textures we want to get
//          - outSRVs: output arr of shader resource views
//
// NOTE:    size of outSRVs arr MUST be equal to numTex
//---------------------------------------------------------
void TextureMgr::GetTexViewsByIds(
    const TexID* texIDs,
    const size numTex,
    ID3D11ShaderResourceView** outSRVs)
{
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

    // get shader resource views
    const vsize numPreparedTextures = idxsToNotZero_.size();
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
