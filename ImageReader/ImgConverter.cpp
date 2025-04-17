#include "ImgConverter.h"

#include "Common/Assert.h"
#include "Common/log.h"

#pragma warning (disable : 4996)

using namespace DirectX;

namespace ImgReader
{
    
#if 0
// if we already have some uncompressed format
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_R8_UNORM:
        case DXGI_FORMAT_R8_SNORM:
        case DXGI_FORMAT_R8G8_UNORM:
        case DXGI_FORMAT_R8G8_SNORM:
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
#endif


// *********************************************************************************
//                            PUBLIC METHODS
// *********************************************************************************

bool ImgConverter::LoadFromFile(const fs::path& filepath, ScratchImage& outImage)
{
    // load image data from file into the input ScratchImage

    if (!fs::exists(filepath))
    {
        sprintf(g_String, "there is no image/texture file: %s", filepath.string().c_str());
        LogErr(g_String);
        return false;
    }

    const wchar_t* path = filepath.wstring().c_str();
    HRESULT hr = LoadFromWICFile(path, WIC_FLAGS_NONE, nullptr, outImage);

    if (FAILED(hr))
    {
        sprintf(g_String, "can't load image/texture from file: %s", filepath.string().c_str());
        LogErr(g_String);
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////

void ImgConverter::LoadFromMemory(
    ID3D11Device* pDevice,
    ID3D11DeviceContext* pContext,
    ID3D11Resource* pTexture,
    ScratchImage& image)
{
    HRESULT hr = CaptureTexture(pDevice, pContext, pTexture, image);
    Assert::NotFailed(hr, "can't capture a texture");
}

///////////////////////////////////////////////////////////

void ImgConverter::GenDstImgPath(const fs::path& srcPath, fs::path& dstPath)
{
    // generate a path to a destination .dds texture

    dstPath = "";
    dstPath += srcPath.parent_path();
    dstPath += "/";
    dstPath += srcPath.stem();
    dstPath += ".dds";
}

///////////////////////////////////////////////////////////

void ImgConverter::Convert(
    const ScratchImage& srcImage,
    const DXGI_FORMAT dstFormat,
    const ConvertOptions& opts,
    ScratchImage& dstImage)
{
    // Convert an image from one pixel format to another dstFormat.

    // This function does not operate directly on block compressed images.
    // See Decompress and Compress.
    // Also: This function cannot operate directly on a planar format image.
    // See ConvertToSinglePlane of DirectXTex

    const DXGI_FORMAT srcFormat = srcImage.GetMetadata().format;

    if (srcFormat == dstFormat) 
        return;

    sprintf(g_String, "convert format (from -> to): %d => %d", srcFormat, dstFormat);
    LogDbg(g_String);


    const bool isSrcCompressed = DirectX::IsCompressed(srcFormat);
    const bool isDstCompressed = DirectX::IsCompressed(dstFormat);

    if (isSrcCompressed || isDstCompressed)
    {
        sprintf(g_String, "can't handle compressed format (src or dst): %d => %d", srcFormat, dstFormat);
        throw LIB_Exception(g_String);
    }

    // uncompressed => uncompressed
    HRESULT hr = ConvertEx(
        srcImage.GetImages(), 
        srcImage.GetImageCount(),
        srcImage.GetMetadata(),
        dstFormat,
        opts, dstImage);
    Assert::NotFailed(hr, "can't convert");
}

///////////////////////////////////////////////////////////

void ImgConverter::Decompress(
    const ScratchImage& srcImage,
    const DXGI_FORMAT dstFormat,
    ScratchImage& dstImage)
{
    // decompress: from compressed into uncompressed dstFormat

    const DXGI_FORMAT srcFormat = srcImage.GetMetadata().format;

    sprintf(g_String, "decompress image (from -> to): %d => %d", srcFormat, dstFormat);
    LogDbg(g_String);


    const bool isSrcCompressed = DirectX::IsCompressed(srcFormat);
    const bool isDstCompressed = DirectX::IsCompressed(dstFormat);

    if (!isSrcCompressed || isDstCompressed)
    {
        sprintf(g_String, "wrong format params: %d => %d", srcFormat, dstFormat);
        throw LIB_Exception(g_String);
    }

    // compressed => uncompressed
    HRESULT hr = DirectX::Decompress(
        srcImage.GetImages(),
        srcImage.GetImageCount(),
        srcImage.GetMetadata(),
        dstFormat, 
        dstImage);
    Assert::NotFailed(hr, "can't decompress");
}

///////////////////////////////////////////////////////////

void ImgConverter::Compress(
    const ScratchImage& srcImg,
    const DXGI_FORMAT dstFormat,
    const CompressOptions opts,
    ScratchImage& dstImg)
{
    // compress: from uncompressed into compressed dstFormat

    HRESULT hr = S_OK;
    const DXGI_FORMAT srcFormat = srcImg.GetMetadata().format;

    bool isSrcCompressed = DirectX::IsCompressed(srcFormat);
    bool isDstCompressed = DirectX::IsCompressed(dstFormat);
    
    // COMPRESS: uncompressed => compressed
    if (!isSrcCompressed && isDstCompressed)
    {
        sprintf(g_String, "compress image: %d => %d", srcFormat, dstFormat);
        LogDbg(g_String);

        hr = CompressEx(
            srcImg.GetImages(),
            srcImg.GetImageCount(),
            srcImg.GetMetadata(),
            dstFormat,
            opts,
            dstImg);
        Assert::NotFailed(hr, "can't compress");
    }
    // RECOMPRESS: compressed => compressed
    else if (isSrcCompressed && isDstCompressed) 
    {
        sprintf(g_String, "recompress image: %d => %d", srcFormat, dstFormat);
        LogDbg(g_String);

        ScratchImage tempImg;

        hr = DirectX::Decompress(*srcImg.GetImages(), DXGI_FORMAT_UNKNOWN, tempImg);
        Assert::NotFailed(hr, "recompress image: can't decompress");

        hr = CompressEx(
            tempImg.GetImages(),
            tempImg.GetImageCount(),
            tempImg.GetMetadata(),
            dstFormat,
            opts,
            dstImg);
        Assert::NotFailed(hr, "recompress image: can't compress");
    }
}

///////////////////////////////////////////////////////////

ScratchImage ImgConverter::GenMipMaps(
    ScratchImage& srcImage,
    const TEX_FILTER_FLAGS filter)
{
    // generate a full mipmaps chain for 2D dimension textures
    // (if there is not mipmaps before);
    // 
    // return: mipChain
    // 
    // NOTE_1: for 3D dimension textures (a.k.a. volume maps), 
    //         see GenerateMipMaps3D of DirectXTex;
    //
    // NOTE_2: also this func can't operate directly on a planar format images.
    //         See ConvertToSinglePlane of DirectXTex;


    const TexMetadata& metadata = srcImage.GetMetadata();

    // check if the input img format is proper
    Assert::True(IsValid(metadata.format), "the format of input img is invalid");
    Assert::True(!IsPlanar(metadata.format), "a planar format isn't supported");

    // generate mip-maps if necessary
    if (metadata.mipLevels == 1 &&
        metadata.dimension == TEX_DIMENSION_TEXTURE2D)
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

            Assert::NotFailed(hr, "can't generate mipmaps");
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

            Assert::NotFailed(hr, "can't generate mipmaps");
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

///////////////////////////////////////////////////////////

void ImgConverter::ProcessImage(
    const ScratchImage& srcImage,
    const DXGI_FORMAT dstFormat,
    ScratchImage& dstImage)
{
    // convert/decompress/compress image if necessary;
    // by input srcFormat and dstFormat we define if we need any process;

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

///////////////////////////////////////////////////////////

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
        sprintf(g_String, "can't save image into dds file: %s", dstPath.string().c_str());
        LogErr(g_String);
        return false;
    }

    return true;
}

} // namespace ImgReader
