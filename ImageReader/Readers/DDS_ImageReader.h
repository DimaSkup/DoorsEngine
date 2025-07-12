// *********************************************************************************
// Filename:     DDS_ImageReader.h
// Description:  textures loader/initializer of the .dds format;
// 
// *********************************************************************************
#pragma once

#include <d3d11.h>
#include <MemHelpers.h>
#include <d3dx11tex.h>

namespace Img
{

class DDS_ImageReader
{
public:
    //---------------------------------------------------------
    // Desc:   a STATIC method to load a DDS texture from the file by filePath
    //         and initializes input parameters: texture resource, shader resource view,
    //         width and height of the texture;
    // Args:   - filePath:      path to image file
    //         - pDevice:       a ptr to DirectX11 device
    // Out:    - ppTexture:     a double pointer to DirectX11 texture resource
    //         - ppTextureView: a double pointer to DirectX11 shader resource view
    //         - texWidth:      texture width 
    //         - texHeight:     texture height
    // Ret:    true: if we managed to load in a texture and create resources
    //---------------------------------------------------------
    static bool LoadTextureFromFile(
        const char* filePath,
        ID3D11Device* pDevice,
        ID3D11Resource** ppTexture,
        ID3D11ShaderResourceView** ppTextureView,
        uint32_t& texWidth,
        uint32_t& texHeight)
    {
        HRESULT hr = S_OK;
        D3DX11_IMAGE_LOAD_INFO loadInfo;
        loadInfo.MipLevels = 0;              // max level of mipmaps           

        wchar_t wStr[256]{ L'\0' };
        StrHelper::StrToWide(filePath, wStr);


        // create a shader resource view from the texture file
        hr = D3DX11CreateShaderResourceViewFromFile(pDevice,
            wStr,                 // src file path
            &loadInfo,            // ptr load info
            nullptr,              // ptr pump
            ppTextureView,        // pp shader resource view
            nullptr);             // pHresult

        if (FAILED(hr))
        {
            LogErr(LOG, "can't load a DDS texture from the file: %s", filePath);
            return false;
        }

        // initialize a texture resource using the shader resource view
        (*ppTextureView)->GetResource(ppTexture);

        // load information about the texture
        D3DX11_IMAGE_INFO imageInfo;
        hr = D3DX11GetImageInfoFromFile(wStr, nullptr, &imageInfo, nullptr);

        if (FAILED(hr))
        {
            LogErr(LOG, "can't load a DDS texture from the file: %s", filePath);
            SafeRelease(ppTexture);
            SafeRelease(ppTextureView);
            return false;
        }

        // initialize the texture width and height values
        texWidth  = imageInfo.Width;
        texHeight = imageInfo.Height;

        return true;
    }
};


} // namespace ImgReader
