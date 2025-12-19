#include "ImgConverter.h"

#include <CAssert.h>
#include <log.h>
#include <EngineException.h>
#include <FileSystem.h>
#include <StrHelper.h>

#pragma warning (disable : 4996)
using namespace DirectX;


namespace Img
{

//---------------------------------------------------------
// Desc:  load image data from file by filepath into the ScratchImage
//---------------------------------------------------------
bool ImgConverter::LoadFromFile(const char* filepath, ScratchImage& outImage)
{
    if (StrHelper::IsEmpty(filepath))
    {
        LogErr(LOG, "there is no image/texture file: %s", filepath);
        return false;
    }

    if (strlen(filepath) > 128)
    {
        LogErr(LOG, "wstring buffer overflow");
        return false;
    }


    HRESULT hr = S_OK;
    wchar_t wPath[128]{L'\0'};
    char    ext[8]{'\0'};           // extension

    StrHelper::StrToWide(filepath, wPath);
    FileSys::GetFileExt(filepath, ext);

    // try to load a dds texture
    if (strcmp(ext, ".dds") == 0)
    {
        hr = LoadFromDDSFile(wPath, DDS_FLAGS_NONE, nullptr, outImage);
    }

    // try to load a png/jpg/jpeg texture
    else if ((strcmp(ext, ".png") == 0) || (strcmp(ext, ".jpg") == 0) || (strcmp(ext, ".jpeg") == 0))
    {
        hr = LoadFromWICFile(wPath, WIC_FLAGS_NONE, nullptr, outImage);
    }

    else
    {
        LogErr(LOG, "unsupported image format: %s,  for file: %s", ext, filepath);
        return false;
    }
    
    if (FAILED(hr))
    {
        LogErr(LOG, "can't load image/texture from file: %s", filepath);
        return false;
    }

    return true;
}

// --------------------------------------------------------
// Desc:   create a ScratchImage loading data from
//         the input texture resource
// Args:   - pDevice:  a ptr to the DX11 device
//         - pContext: a ptr to the DX11 device context
//         - pTexture: a ptr to the DX11 resource (in this case - texture)
//         - image:    output image
// --------------------------------------------------------
void ImgConverter::LoadFromMemory(
    ID3D11Device* pDevice,
    ID3D11DeviceContext* pContext,
    ID3D11Resource* pTexture,
    ScratchImage& image)
{
    HRESULT hr = CaptureTexture(pDevice, pContext, pTexture, image);
    CAssert::NotFailed(hr, "can't capture a texture");
}

// --------------------------------------------------------
// Desc:  create a 2D texture resource by input
//        raw texture pixels data, input metadata and flags
// Args:  - ID3D11Device:   a pointer to the DX11 device
//        - image:          raw image data and params
//        - metadata:       image metadata (width/heigh/mipLevels/etc)
//        - usage:          identified expected resource use during rendering
//        - bindFlags:      identifies how to bind a resource to the pipeline
//        - cpuAccessFlags: specifies the types of CPU access allowed for a resource
//        - miscFlags:      identifies options for resources
//        - genMips:        flag which defines if we will generate mipmaps or not
//        - ppOutResource:  output texture resource
// --------------------------------------------------------
HRESULT ImgConverter::CreateTexture2dEx(
    ID3D11Device* pDevice,
    const Image& image,
    const TexMetadata& metadata,
    const D3D11_USAGE usage,
    const UINT bindFlags,
    const UINT cpuAccessFlags,
    const UINT miscFlags,
    const bool genMips,
    ID3D11Resource** ppOutResource)
{
    HRESULT      hr     = S_OK;
    size_t       levels = (genMips) ? 0 : 1;   //  0 indicates creating a full mipmap chain down to 1x1
    ScratchImage srcImage;

    srcImage.InitializeFromImage(image);

    // we want to generate mip maps for the image
    if (genMips)
    {
        ScratchImage mipChain;

        hr = DirectX::GenerateMipMaps(
            srcImage.GetImages(),
            srcImage.GetImageCount(),
            srcImage.GetMetadata(),
            TEX_FILTER_DEFAULT,
            0,
            mipChain);

        if (FAILED(hr))
        {
            LogErr("can't generate mipmaps");
            return hr;
        }

        hr = DirectX::CreateTexture(
            pDevice,
            mipChain.GetImages(),
            mipChain.GetImageCount(),
            mipChain.GetMetadata(),
            ppOutResource);

#if 0
        hr = DirectX::CreateTextureEx(
            pDevice,
            mipChain.GetImages(),
            mipChain.GetImageCount(),                    
            mipChain.GetMetadata(),
            usage,
            bindFlags,
            cpuAccessFlags,
            miscFlags,
            CREATETEX_FLAGS::CREATETEX_DEFAULT,
            ppOutResource);
#endif

        if (FAILED(hr))
            LogErr("can't create texture from input raw data");
    }

    // else: we want to create an image without mipmaps
    else
    {
        hr = DirectX::CreateTextureEx(
            pDevice,
            srcImage.GetImages(),
            srcImage.GetImageCount(),
            srcImage.GetMetadata(),
            usage,
            bindFlags,
            cpuAccessFlags,
            miscFlags,
            CREATETEX_FLAGS::CREATETEX_DEFAULT,
            ppOutResource);

        if (FAILED(hr))
            LogErr("can't create texture from input raw data");
    }

    return hr;
}

//---------------------------------------------------------
// Desc:  generate a path to a destination .dds texture
//---------------------------------------------------------
void ImgConverter::GenDstImgPath(const fs::path& srcPath, fs::path& dstPath)
{
    dstPath = "";
    dstPath += srcPath.parent_path();
    dstPath += "/";
    dstPath += srcPath.stem();
    dstPath += ".dds";
}

//---------------------------------------------------------
// Desc:  convert an image from one pixel format to another dstFormat.
//
//        This function does not operate directly on block compressed images.
//        See Decompress and Compress.
//        Also: This function cannot operate directly on a planar format image.
//        See ConvertToSinglePlane of DirectXTex
//---------------------------------------------------------
void ImgConverter::Convert(
    const ScratchImage& srcImage,
    const DXGI_FORMAT dstFormat,
    const ConvertOptions& opts,
    ScratchImage& dstImage)
{
    

    const DXGI_FORMAT srcFormat = srcImage.GetMetadata().format;

    if (srcFormat == dstFormat) 
        return;

    LogDbg(LOG, "convert format (from -> to): %d => %d", srcFormat, dstFormat);

    const bool isSrcCompressed = DirectX::IsCompressed(srcFormat);
    const bool isDstCompressed = DirectX::IsCompressed(dstFormat);

    if (isSrcCompressed || isDstCompressed)
    {
        sprintf(g_String, "can't handle compressed format (src or dst): %d => %d", srcFormat, dstFormat);
        throw EngineException(g_String);
    }

    // uncompressed => uncompressed
    HRESULT hr = ConvertEx(
        srcImage.GetImages(), 
        srcImage.GetImageCount(),
        srcImage.GetMetadata(),
        dstFormat,
        opts, dstImage);
    CAssert::NotFailed(hr, "can't convert");
}

//---------------------------------------------------------
// Desc:  decompress image from compressed into uncompressed format
// Args:  - srcImage:   source image
//        - dstFormat:  destination image format
//        - dstImage:   destination image
//---------------------------------------------------------
void ImgConverter::Decompress(
    const ScratchImage& srcImage,
    const DXGI_FORMAT dstFormat,
    ScratchImage& dstImage)
{
    const DXGI_FORMAT srcFormat = srcImage.GetMetadata().format;

    LogDbg(LOG, "decompress image (from -> to): %d => %d", srcFormat, dstFormat);


    const bool isSrcCompressed = DirectX::IsCompressed(srcFormat);
    const bool isDstCompressed = DirectX::IsCompressed(dstFormat);

    if (!isSrcCompressed || isDstCompressed)
    {
        sprintf(g_String, "wrong format params: %d => %d", srcFormat, dstFormat);
        throw EngineException(g_String);
    }

    // compressed => uncompressed
    HRESULT hr = DirectX::Decompress(
        srcImage.GetImages(),
        srcImage.GetImageCount(),
        srcImage.GetMetadata(),
        dstFormat, 
        dstImage);
    CAssert::NotFailed(hr, "can't decompress");
}

//---------------------------------------------------------
// Desc:   compress/recompress image
// Args:   - srcImg:     source image
//         - dstFormat:  destination image format (must to be a type for compression)
//         - opts:
//         - dstImg:     destination image
//---------------------------------------------------------
void ImgConverter::Compress(
    const ScratchImage& srcImg,
    const DXGI_FORMAT dstFormat,
    const CompressOptions opts,
    ScratchImage& dstImg)
{
    HRESULT hr = S_OK;
    const DXGI_FORMAT srcFormat = srcImg.GetMetadata().format;

    bool isSrcCompressed = DirectX::IsCompressed(srcFormat);
    bool isDstCompressed = DirectX::IsCompressed(dstFormat);
    
    // COMPRESS: uncompressed => compressed
    if (!isSrcCompressed && isDstCompressed)
    {
        LogDbg(LOG, "compress image: %d => %d", srcFormat, dstFormat);

        hr = CompressEx(
            srcImg.GetImages(),
            srcImg.GetImageCount(),
            srcImg.GetMetadata(),
            dstFormat,
            opts,
            dstImg);
        CAssert::NotFailed(hr, "can't compress");
    }
    // RECOMPRESS: compressed => compressed
    else if (isSrcCompressed && isDstCompressed) 
    {
        LogDbg(LOG, "recompress image: %d => %d", srcFormat, dstFormat);

        ScratchImage tempImg;

        hr = DirectX::Decompress(*srcImg.GetImages(), DXGI_FORMAT_UNKNOWN, tempImg);
        CAssert::NotFailed(hr, "recompress image: can't decompress");

        hr = CompressEx(
            tempImg.GetImages(),
            tempImg.GetImageCount(),
            tempImg.GetMetadata(),
            dstFormat,
            opts,
            dstImg);
        CAssert::NotFailed(hr, "recompress image: can't compress");
    }
}

//---------------------------------------------------------
// generate a full mipmaps chain for 2D textures
// 
// return: mipChain
// 
// NOTE_1: for 3D dimension textures (a.k.a. volume maps), 
//         see GenerateMipMaps3D of DirectXTex;
//
// NOTE_2: also this func can't operate directly on a planar format images.
//         See ConvertToSinglePlane of DirectXTex;
//---------------------------------------------------------
ScratchImage ImgConverter::GenMipMaps(
    ScratchImage& srcImage,
    const TEX_FILTER_FLAGS filter)
{
    const TexMetadata& metadata = srcImage.GetMetadata();

    // check if the input img format is proper
    CAssert::True(IsValid(metadata.format), "the format of input img is invalid");
    CAssert::True(!IsPlanar(metadata.format), "a planar format isn't supported");

    // generate mip-maps if necessary
    if (metadata.dimension == TEX_DIMENSION_TEXTURE2D)
    {
        size_t levels = 0;       //  0 indicates creating a full mipmap chain down to 1x1
        ScratchImage mipChain;
        HRESULT hr = S_OK;

        // if input img is compressed we have to decompress it first;
        // and then generate mipmaps for it
        if (IsCompressed(metadata.format))
        {
            ScratchImage decompressedImage;

            Decompress(srcImage, DXGI_FORMAT_R8G8B8A8_UNORM, decompressedImage);

            hr = GenerateMipMaps(
                decompressedImage.GetImages(),
                decompressedImage.GetImageCount(),
                decompressedImage.GetMetadata(),
                filter,
                levels,
                mipChain);

            CAssert::NotFailed(hr, "can't generate mipmaps");
        }
        // else input img is already uncompressed
        else
        {
            hr = GenerateMipMaps(
                srcImage.GetImages(),
                srcImage.GetImageCount(),
                metadata,
                filter,
                levels,
                mipChain);

            CAssert::NotFailed(hr, "can't generate mipmaps");
        }

        // return a generated mipChain
        return mipChain;
    }

    // input src image already has mipmaps
    else
    {
        return ScratchImage(std::move(srcImage));
    }
}

//---------------------------------------------------------
// Desc:  convert/decompress/compress image if necessary (according to dstFormat);
//        by input srcFormat and dstFormat we define if we need any process;
// Args:  - srcImage:  source image
//        - dstFormat: destination image format for processing
//                     (if the same as the source image we do nothing)
//        - dstImage:  dentination image
//---------------------------------------------------------
void ImgConverter::ProcessImage(
    const ScratchImage& srcImage,
    const DXGI_FORMAT dstFormat,
    ScratchImage& dstImage)
{
    const bool isSrcCompressed = IsCompressed(srcImage.GetMetadata().format);
    const bool isDstCompressed = IsCompressed(dstFormat);

    // compress / recompress image
    if (isDstCompressed)
    {
        CompressOptions copts = {};
        copts.flags         = TEX_COMPRESS_DEFAULT;
        copts.threshold     = TEX_THRESHOLD_DEFAULT;
        copts.alphaWeight   = TEX_ALPHA_WEIGHT_DEFAULT;

        Compress(srcImage, dstFormat, copts, dstImage);
    }
    // we have uncompressed src format so define how we want to process img
    else
    {
        // uncompressed => uncompressed
        if (!isSrcCompressed && !isDstCompressed)
        {
            ConvertOptions opts;
            opts.filter     = TEX_FILTER_DEFAULT;
            opts.threshold  = TEX_THRESHOLD_DEFAULT;

            Convert(srcImage, dstFormat, opts, dstImage);
        }
        // compressed => uncompressed
        else
        {
            Decompress(srcImage, dstFormat, dstImage);
        }
    }
}

//---------------------------------------------------------
// Desc:  save input image into .dds file
//---------------------------------------------------------
bool ImgConverter::SaveToFile(
    const DirectX::ScratchImage& image,
    const DDS_FLAGS flags,
    const fs::path& dstPath)
{
    HRESULT hr = SaveToDDSFile(
        image.GetImages(),
        image.GetImageCount(),
        image.GetMetadata(),
        flags,
        dstPath.wstring().c_str());

    if (FAILED(hr))
    {
        LogErr(LOG, "can't save image into dds file: %s", dstPath.string().c_str());
        return false;
    }

    return true;
}

} // namespace ImgReader
