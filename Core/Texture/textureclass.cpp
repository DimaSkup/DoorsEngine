// =================================================================================
// Filename: textureclass.cpp
// =================================================================================
#include "textureclass.h"

#include "ImageReader.h"
#include "../../ImageReader/Common/LIB_Exception.h"

#include <CoreCommon/Log.h>
#include <CoreCommon/MemHelpers.h>
#include <CoreCommon/Assert.h>
#include <CoreCommon/FileSystem.h>
#include <CoreCommon/StrHelper.h>
#include <CoreCommon/cvector.h>

#include <D3DX11tex.h>


#pragma warning (disable : 4996)


namespace Core
{

// =================================================================================
// Helpers to create a Texture 2D Array
// =================================================================================
bool CreateTexturesFromFiles(
    ID3D11Device* pDevice,
    const std::string* texturesNames,
    const size numTextures,
    const DXGI_FORMAT format,
    cvector<ID3D11Texture2D*>& outTextures)
{
    // load the texture elements individually from file. These texture won't
    // be used by the GPU (0 bind flags), they are just used to load the image 
    // data from file. We use the STAGING usage so the CPU can read the resource.

    wchar_t wSrcFilepath[256]{ L'\0' };

    outTextures.resize(numTextures, nullptr);

    for (index i = 0; i < numTextures; ++i)
    {
#if DEBUG || _DEBUG
        if (!FileSys::Exists(texturesNames[i].c_str()))
        {
            sprintf(g_String, "there is no texture file: %s", texturesNames[i].c_str());
            LogErr(g_String);
            return false;
        }
#endif

        D3DX11_IMAGE_LOAD_INFO loadInfo;
        loadInfo.Width          = D3DX11_FROM_FILE;
        loadInfo.Height         = D3DX11_FROM_FILE;
        loadInfo.Depth          = D3DX11_FROM_FILE;
        loadInfo.FirstMipLevel  = 0;
        loadInfo.MipLevels      = D3DX11_FROM_FILE;
        loadInfo.Usage          = D3D11_USAGE_STAGING;
        loadInfo.BindFlags      = 0;
        loadInfo.CpuAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
        loadInfo.MiscFlags      = 0;
        loadInfo.Format         = format;
        loadInfo.Filter         = D3DX11_DEFAULT;
        loadInfo.MipFilter      = D3DX11_DEFAULT;
        loadInfo.pSrcInfo       = 0;

        // convert str => wide str
        StrHelper::StrToWide(texturesNames[i].c_str(), wSrcFilepath);

        HRESULT hr = D3DX11CreateTextureFromFile(
            pDevice,
            wSrcFilepath,
            &loadInfo,
            nullptr,
            (ID3D11Resource**)&outTextures[i],
            nullptr);

        if (FAILED(hr))
        {
            // if we got any error we release all the previously created textures
            for (index idx = 0; idx < i; ++idx)
                SafeRelease(&outTextures[idx]);

            sprintf(g_String, "can't create a texture from file: %s", texturesNames[i].c_str());
            LogErr(g_String);
            return false;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////

bool FillTextureArray(
    ID3D11Device* pDevice,
    const size numTextures,
    const D3D11_TEXTURE2D_DESC& texElemDesc,
    const cvector<ID3D11Texture2D*>& srcTextures,
    ID3D11Texture2D* textureArr)
{
    // copy individual texture element into texture array

    ID3D11DeviceContext* pContext = nullptr;
    pDevice->GetImmediateContext(&pContext);

    // for each texture element...
    for (int texElem = 0; texElem < (int)numTextures; ++texElem)
    {
        // for each mipmap level...
        for (UINT mipLevel = 0; mipLevel < texElemDesc.MipLevels; ++mipLevel)
        {
            D3D11_MAPPED_SUBRESOURCE mappedTex2D;

            // map individual texture
            HRESULT hr = pContext->Map(srcTextures[texElem], mipLevel, D3D11_MAP_READ, 0, &mappedTex2D);
            if (FAILED(hr))
            {
                sprintf(g_String, "can't map a texture by idx: %d", texElem);
                LogErr(g_String);
                return false;
            }

            // update texture 2D array
            pContext->UpdateSubresource(
                textureArr,                   // dst resource
                D3D11CalcSubresource(         // compute idx identifying the subresource we are updating in the dst resource
                    mipLevel,
                    (UINT)texElem,
                    texElemDesc.MipLevels),
                nullptr,                      // pDstBox - a ptr to a D3D11_BOX instance that specifies the volume in the destination subresource we are updating; specify null to update the entire subresource
                mappedTex2D.pData,            // ptr to the src data
                mappedTex2D.RowPitch,         // byte size of one row of the src data
                mappedTex2D.DepthPitch);      // byte size of one depth slice of the src data

            pContext->Unmap(srcTextures[texElem], mipLevel);
        }
    }
    return true;
}

///////////////////////////////////////////////////////////

void InitTexture2dArr(
    ID3D11Device* pDevice,
    D3D11_TEXTURE2D_DESC& texElemDesc,
    D3D11_TEXTURE2D_DESC& texArrayDesc,
    const cvector<ID3D11Texture2D*>& srcTextures,
    ID3D11Texture2D** textureArr)
{
    // Create the texture array. Each element in the texture array
    // has the same format/dimensions/mip levels number

    texArrayDesc.Width              = texElemDesc.Width;
    texArrayDesc.Height             = texElemDesc.Height;
    texArrayDesc.MipLevels          = texElemDesc.MipLevels;
    texArrayDesc.ArraySize          = (UINT)srcTextures.size();
    texArrayDesc.Format             = texElemDesc.Format;
    texArrayDesc.SampleDesc.Count   = 1;
    texArrayDesc.SampleDesc.Quality = 0;
    texArrayDesc.Usage              = D3D11_USAGE_DEFAULT;
    texArrayDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
    texArrayDesc.CPUAccessFlags     = 0;
    texArrayDesc.MiscFlags          = 0;

    HRESULT hr = pDevice->CreateTexture2D(&texArrayDesc, nullptr, textureArr);
    Assert::NotFailed(hr, "can't create a texture array");

    // fill in the texture 2D array with data of separate textures
    bool res = FillTextureArray(pDevice, srcTextures.size(), texElemDesc, srcTextures, *textureArr);
    Assert::True(res, "can't fill in the texture 2D with data");
}


// =================================================================================
// CONSTRUCTORS / DESTRUCTOR
// =================================================================================
Texture::Texture()
{
}

///////////////////////////////////////////////////////////

Texture::Texture(ID3D11Device* pDevice, const char* filePath) :
    name_(filePath)
{
    // create and initialize a texture from a file by filePath
    try
    {
        LoadFromFile(pDevice, filePath);
    }
    catch (EngineException & e)
    {
        sprintf(g_String, "can't create a texture from file: %s", filePath);
        LogErr(e);
        LogErr(g_String);
    }
}

///////////////////////////////////////////////////////////

Texture::Texture(
    ID3D11Device* pDevice,
    const char* name,            // a name for this texture object
    const std::string* texturesNames,
    const size numTextures,
    const DXGI_FORMAT format)
{
    // create a TEXTURE 2D ARRAY and associated view from a series of textures on disk

    cvector<ID3D11Texture2D*> srcTextures;
    ID3D11Texture2D* textureArr = nullptr;
    ID3D11ShaderResourceView* texArrSRV = 0;

    try
    {
        Assert::True((name != nullptr) && (name[0] != '\0'), "input name for the texture object is empty");
        Assert::True(texturesNames != nullptr,               "input arr of paths to textures == nullptr");
        Assert::True(numTextures > 0,                        "input number of textures filenames must be > 0");
        
        bool res = CreateTexturesFromFiles(pDevice, texturesNames, numTextures, format, srcTextures);
        Assert::True(res, "can't create individual textures from files");


        D3D11_TEXTURE2D_DESC texElemDesc;
        D3D11_TEXTURE2D_DESC texArrayDesc;

        srcTextures[0]->GetDesc(&texElemDesc);
        InitTexture2dArr(pDevice, texElemDesc, texArrayDesc, srcTextures, &textureArr);


        // Create a resource view to the texture array
        D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
        viewDesc.Format                         = texArrayDesc.Format;
        viewDesc.ViewDimension                  = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        viewDesc.Texture2DArray.MostDetailedMip = 0;
        viewDesc.Texture2DArray.MipLevels       = texArrayDesc.MipLevels;
        viewDesc.Texture2DArray.FirstArraySlice = 0;
        viewDesc.Texture2DArray.ArraySize       = (UINT)numTextures;

        HRESULT hr = pDevice->CreateShaderResourceView(textureArr, &viewDesc, &texArrSRV);
        Assert::NotFailed(hr, "can't create a shader resource view for texture array");


        // cleanup
        for (index i = 0; i < numTextures; ++i)
            SafeRelease(&srcTextures[i]);

        // assignment
        pTexture_     = textureArr;
        pTextureView_ = texArrSRV;
        width_        = texArrayDesc.Width;
        height_       = texArrayDesc.Height;
        name_         = name;
    }
    catch (EngineException& e)
    {
        for (index i = 0; i < numTextures; ++i)
            SafeRelease(&srcTextures[i]);

        SafeRelease(&textureArr);
        SafeRelease(&texArrSRV);

        LogErr(e);
    }
}

///////////////////////////////////////////////////////////

Texture::Texture(ID3D11Device* pDevice, const Color & color)
{
    // create a 1x1 texture with input color value
    try
    {
        Initialize1x1ColorTexture(pDevice, color);
    }
    catch (EngineException& e)
    {
        LogErr(e);
        throw EngineException("can't create a texture by input color data");
    }
}

///////////////////////////////////////////////////////////

Texture::Texture(
    ID3D11Device* pDevice, 
    const Color* pColorData, 
    const UINT width,
    const UINT height)
{
    // create a width_x_height texture with input color data
    try
    {
        Assert::NotNullptr(pColorData,       "the input ptr to color data == nullptr");
        Assert::True((bool)(width & height), "texture dimensions must be greater that zero");

        InitializeColorTexture(pDevice, pColorData, width, height);
    }
    catch (EngineException& e)
    {
        LogErr(e);
        throw EngineException("can't create a texture by given color data");
    }	
}

///////////////////////////////////////////////////////////

Texture::Texture(
    ID3D11Device* pDevice,
    const char* name,
    const uint8_t* pData,
    const size_t size)
{
    // a constructor for loading embedded compressed textures;
    // 
    // source texture can be compressed or embedded so we firstly load its 
    // content into the memory and then passed here ptr to this loaded data;
    try
    {
        Assert::True((name != nullptr) && (name[0] != '\0'), "input name for the texture obj is empty");
        Assert::True(pData != nullptr,                       "input ptr to texture data == nullptr");
        Assert::True(size > 0,                               "size of input texture data must be > 0");

        ImgReader::ImageReader   imageReader;
        ImgReader::DXTextureData texData(name, &pTexture_, &pTextureView_);

        HRESULT hr = S_OK;

    const float area = (float)(size / 4);
    const UINT width = (UINT)sqrt(area);

    ID3D11Texture2D* p2DTexture = nullptr;
    D3D11_TEXTURE2D_DESC textureDesc;
    D3D11_SUBRESOURCE_DATA initialData{};

    // setup description for this texture
    textureDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.Width              = width;
    textureDesc.Height             = width;
    textureDesc.ArraySize          = 1;
    textureDesc.MipLevels          = 1;
    textureDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.Usage              = D3D11_USAGE_DEFAULT;
    textureDesc.CPUAccessFlags     = 0;
    textureDesc.SampleDesc.Count   = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.MiscFlags          = 0;

    // setup initial data for this texture
    initialData.pSysMem = pData;
    initialData.SysMemPitch = textureDesc.Width * sizeof(uint8_t)*4;

    // create a new 2D texture
    hr = pDevice->CreateTexture2D(&textureDesc, &initialData, &p2DTexture);
    Assert::NotFailed(hr, "Failed to initialize texture from color data");

    // store a ptr to the 2D texture 
    //pTexture_ = static_cast<ID3D11Texture2D*>(p2DTexture);
    pTexture_ = p2DTexture;

    // setup description for a shader resource view (SRV)
    CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D, textureDesc.Format);

    // create a new SRV from texture
    hr = pDevice->CreateShaderResourceView(pTexture_, &srvDesc, &pTextureView_);
    Assert::NotFailed(hr, "Failed to create shader resource view from texture generated from color data");


#if 0
         hr = D3DX11CreateTextureFromMemory(
            pDevice,
            (void*)pData,
            size,
            nullptr,
            nullptr,
            &pTexture_,
            nullptr);
        Assert::NotFailed(hr, "can't create a texture from memory");

        D3D11_TEXTURE2D_DESC textureDesc;
        ((ID3D11Texture2D*)pTexture_)->GetDesc(&textureDesc);

        // setup description for a shader resource view (SRV)
        CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D, textureDesc.Format);


        hr = pDevice->CreateShaderResourceView(pTexture_, &srvDesc, &pTextureView_);
        Assert::NotFailed(hr, "can't create a shader resource view");

        

        // initialize the texture width and height values
        D3D11_TEXTURE2D_DESC desc;
        ID3D11Texture2D* pTex = (ID3D11Texture2D*)(pTexture_);
        pTex->GetDesc(&desc);

        width_ = desc.Width;
        height_ = desc.Height;
#endif
#if 0
        bool result = imageReader.LoadTextureFromMemory(pDevice, pData, size, texData);

        if (!result)
        {
            sprintf(g_String, "can't load a texture from memory");
            LogErr(g_String);

            // if we didn't manage to load a texture from the memory
            // we create a 1x1 color texture for this texture object
            Initialize1x1ColorTexture(pDevice, Colors::UnloadedTextureColor);

            width_ = 1;
            height_ = 1;

            return;
        }
#endif
        width_ = texData.textureWidth;
        height_ = texData.textureHeight;
        name_ = name;
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr("can't create an embedded compressed texture");

        Initialize1x1ColorTexture(pDevice, Colors::UnloadedTextureColor);
    }
}

///////////////////////////////////////////////////////////

Texture::Texture(Texture&& rhs) noexcept :
    name_        (std::exchange(rhs.name_, "")),
    pTexture_    (std::exchange(rhs.pTexture_, nullptr)),
    pTextureView_(std::exchange(rhs.pTextureView_, nullptr)),
    width_       (rhs.width_),
    height_      (rhs.height_)
{
    // move constructor
}

///////////////////////////////////////////////////////////

Texture& Texture::operator=(Texture&& rhs) noexcept
{
    // move assignment
    if (this != &rhs)
    {
        Clear();                    // lifetime of *this ends
        std::construct_at(this, std::move(rhs));
    }

    return *this;
}

///////////////////////////////////////////////////////////

Texture::~Texture()
{
    Clear();
}


// =================================================================================
// Public API
// =================================================================================
void Texture::Copy(Texture& src)
{
    this->Copy(src.pTexture_);

    // copy common data of the texture instance
    name_   = src.name_;
    width_  = src.width_;
    height_ = src.height_;
}

///////////////////////////////////////////////////////////

void Texture::Copy(ID3D11Resource* const pSrcTexResource)
{
    // deep copy: execute copying of the src texture into the current;
    // note:      we discard all the previous data of the current texture;

    // guard self assignment
    if (this->pTexture_ == pSrcTexResource)
        return;

    // clear the previous data
    Clear();

    ID3D11Device* pDevice = nullptr;
    ID3D11DeviceContext* pContext = nullptr;

    // get ptrs to the device and device context
    pSrcTexResource->GetDevice(&pDevice);
    pDevice->GetImmediateContext(&pContext);


    // ------------------------------------------
    
    HRESULT                  hr = S_OK;
    uint32_t*                pixelsData  = nullptr;
    ID3D11Texture2D*         pSrcTexture = static_cast<ID3D11Texture2D*>(pSrcTexResource);
    ID3D11Texture2D*         pDstTexture = nullptr;
    ID3D11Texture2D*         pTextureBuf = nullptr;
    D3D11_TEXTURE2D_DESC     textureBufDesc;
    D3D11_TEXTURE2D_DESC     dstTextureDesc;
    D3D11_SUBRESOURCE_DATA   initialData;
    D3D11_MAPPED_SUBRESOURCE mappedSubresource;

    // create a staging texture for resource copying
    pSrcTexture->GetDesc(&textureBufDesc);
    textureBufDesc.BindFlags      = 0;
    textureBufDesc.Usage          = D3D11_USAGE_STAGING;
    textureBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

    hr = pDevice->CreateTexture2D(&textureBufDesc, nullptr, &pTextureBuf);
    Assert::NotFailed(hr, "Failed to create a staging texture");

    // copy the data from the source texture
    pContext->CopyResource(pTextureBuf, pSrcTexture);

    // map the staging buffer
    hr = pContext->Map(pTextureBuf, 0, D3D11_MAP_READ, 0, &mappedSubresource);
    Assert::NotFailed(hr, "can't map the staging buffer");

    // copy src texture data into the temporal buffer
    const int numPixels = textureBufDesc.Width * textureBufDesc.Height;
    pixelsData = new uint32_t[numPixels];
    memcpy(pixelsData, mappedSubresource.pData, sizeof(uint32_t) * numPixels);

    pContext->Unmap(pTextureBuf, 0);


    // ----------------------------------------------------

    // setup initial data for a dst texture
    initialData.pSysMem = pixelsData;
    initialData.SysMemPitch = mappedSubresource.RowPitch;

    // create a dst texture with the same params as the src one
    pSrcTexture->GetDesc(&dstTextureDesc);
    dstTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    hr = pDevice->CreateTexture2D(&dstTextureDesc, &initialData, &pDstTexture);
    Assert::NotFailed(hr, "Failed to create a dst texture");

    // store a ptr to the 2D texture 
    pTexture_ = pDstTexture;


    // define which view dimension we need to use
    UINT sdCount   = dstTextureDesc.SampleDesc.Count;
    UINT sdQuality = dstTextureDesc.SampleDesc.Quality;
    bool no4xMSAA  = ((sdCount == 1) && (sdQuality == 0));

    // create a new shader resource view (SRV) for the dst texture
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format                    = dstTextureDesc.Format;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels       = dstTextureDesc.MipLevels;
    srvDesc.ViewDimension             = (no4xMSAA) ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D_SRV_DIMENSION_TEXTURE2DMS;

    hr = pDevice->CreateShaderResourceView(pTexture_, &srvDesc, &pTextureView_);
    Assert::NotFailed(hr, "Failed to create shader resource view (SRV)");


    // ----------------------------------------------------

    // copy common data of the src texture
    name_   = "copy_without_path";
    width_  = dstTextureDesc.Width;
    height_ = dstTextureDesc.Height;

    // after all clear all the temp data
    SafeDeleteArr(pixelsData);
    SafeRelease(&pTextureBuf);
}

///////////////////////////////////////////////////////////

POINT Texture::GetTextureSize()
{
    ID3D11Texture2D* p2DTexture = static_cast<ID3D11Texture2D*>(pTexture_);
    D3D11_TEXTURE2D_DESC desc;
    
    p2DTexture->GetDesc(&desc);
    
    return { (LONG)desc.Width, (LONG)desc.Height };
}


// =================================================================================
// Private API
// =================================================================================
void Texture::Clear()
{
    // clear the texture data and release resources
    width_ = 0;
    height_ = 0;
    name_.clear();
    SafeRelease(&pTexture_);
    SafeRelease(&pTextureView_);
}

///////////////////////////////////////////////////////////

void Texture::LoadFromFile(ID3D11Device* pDevice, const char* filePath)
{
    // load a texture from file by input path and store texture obj into the manager;
    try
    {
        Assert::True((filePath != nullptr) && (filePath[0] != '\0'), "input path to texture is empty");

        ImgReader::ImageReader imageReader;
        ImgReader::DXTextureData data(filePath, &pTexture_, &pTextureView_);

        bool result = imageReader.LoadTextureFromFile(pDevice, data);

        if (!result)
        {
            sprintf(g_String, "can't load a texture from file: %s", filePath);
            LogErr(g_String);

            // if we didn't manage to initialize a texture from the file 
            // we create a 1x1 color texture for this texture object
            Initialize1x1ColorTexture(pDevice, Colors::UnloadedTextureColor);

            width_  = 1;
            height_ = 1;

            return;
        }

        name_ = filePath;
        width_ = data.textureWidth;
        height_ = data.textureHeight;
    }
    catch (EngineException & e)
    {
        LogErr(e);
        throw EngineException("can't initialize a texture from file");
    }
}


///////////////////////////////////////////////////////////

void Texture::Initialize1x1ColorTexture(ID3D11Device* pDevice, const Color & colorData)
{
    InitializeColorTexture(pDevice, &colorData, 1, 1);
}

///////////////////////////////////////////////////////////

void Texture::InitializeColorTexture(
    ID3D11Device* pDevice,
    const Color* pColorData,
    const UINT width,
    const UINT height)
{
    // Initialize a color texture using input color data (pColorData) and
    // the input width/height

    width_ = width;
    height_ = height;

    ID3D11Texture2D* p2DTexture = nullptr;
    D3D11_TEXTURE2D_DESC textureDesc;
    D3D11_SUBRESOURCE_DATA initialData{};

    // setup description for this texture
    textureDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.Width              = width;
    textureDesc.Height             = height;
    textureDesc.ArraySize          = 1;
    textureDesc.MipLevels          = 0;
    textureDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.Usage              = D3D11_USAGE_DEFAULT;
    textureDesc.CPUAccessFlags     = 0;
    textureDesc.SampleDesc.Count   = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.MiscFlags          = 0;

    // setup initial data for this texture
    initialData.pSysMem = pColorData;
    initialData.SysMemPitch = width * sizeof(Color);

    // create a new 2D texture
    HRESULT hr = pDevice->CreateTexture2D(&textureDesc, &initialData, &p2DTexture);
    Assert::NotFailed(hr, "Failed to initialize texture from color data");

    // store a ptr to the 2D texture 
    //pTexture_ = static_cast<ID3D11Texture2D*>(p2DTexture);
    pTexture_ = p2DTexture;

    // setup description for a shader resource view (SRV)
    CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D, textureDesc.Format);

    // create a new SRV from texture
    hr = pDevice->CreateShaderResourceView(pTexture_, &srvDesc, &pTextureView_);
    Assert::NotFailed(hr, "Failed to create shader resource view from texture generated from color data");
}

} // namespace Core
