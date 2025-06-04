#include "../Common/pch.h"
#include "DDS_ImageReader.h"
#include <d3dx11tex.h>

#pragma warning (disable : 4996)

namespace ImgReader
{

bool DDS_ImageReader::LoadTextureFromFile(
    const char* filePath,
    ID3D11Device* pDevice,
    ID3D11Resource** ppTexture,
    ID3D11ShaderResourceView** ppTextureView,
    uint32_t& texWidth,
    uint32_t& texHeight)
{
    // this function loads a DDS texture from the file by filePath
    // and initializes input parameters: texture resource, shader resource view,
    // width and height of the texture;


    HRESULT hr = S_OK;

    D3DX11_IMAGE_LOAD_INFO loadInfo;
    loadInfo.MipLevels = 0;


    //std::wstring wStr(filePath, filePath + strlen(filePath));
    wchar_t wStr[256]{ L'\0' };
    StrHelper::StrToWide(filePath, wStr);


    // create a shader resource view from the texture file
    hr = D3DX11CreateShaderResourceViewFromFile(pDevice,
        wStr,   // src file path
        &loadInfo,            // ptr load info
        nullptr,              // ptr pump
        ppTextureView,        // pp shader resource view
        nullptr);             // pHresult

    if (FAILED(hr))
    {
        sprintf(g_String, "can't load a DDS texture from the file: err during D3DX11CreateShaderResourceViewFromFile(): %s", filePath);
        LogErr(g_String);
        return false;
    }
       
    // initialize a texture resource using the shader resource view
    (*ppTextureView)->GetResource(ppTexture);


    // load information about the texture
    D3DX11_IMAGE_INFO imageInfo;
    hr = D3DX11GetImageInfoFromFile(wStr, nullptr, &imageInfo, nullptr);

    if (FAILED(hr))
    {
        sprintf(g_String, "can't load a DDS texture from the file: err during D3DX11GetImageInfoFromFile(): %s", filePath);
        LogErr(g_String);
        return false;
    }

    // initialize the texture width and height values
    texWidth = imageInfo.Width;
    texHeight = imageInfo.Height;

    return true;
}


} // namespace ImgReader
