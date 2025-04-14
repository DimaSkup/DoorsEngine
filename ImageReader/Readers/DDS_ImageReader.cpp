#include "DDS_ImageReader.h"

#include "../Common/log.h"
#include "DDSTextureLoader11.h"
#include <d3dx11tex.h>

#pragma warning (disable : 4996)

namespace ImgReader
{


bool DDS_ImageReader::LoadTextureFromFile(
    const char* filePath,
    ID3D11Device* pDevice,
    ID3D11Resource** ppTexture,
    ID3D11ShaderResourceView** ppTextureView,
    u32& texWidth,
    u32& texHeight)
{
    // this function loads a DDS texture from the file by filePath
    // and initializes input parameters: texture resource, shader resource view,
    // width and height of the texture;


    HRESULT hr = S_OK;

    D3DX11_IMAGE_LOAD_INFO loadInfo;
    loadInfo.MipLevels = 0;

    // create a shader resource view from the texture file
    hr = D3DX11CreateShaderResourceViewFromFile(pDevice,
        (wchar_t*)filePath,   // src file path
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
    hr = D3DX11GetImageInfoFromFile((wchar_t*)filePath, nullptr, &imageInfo, nullptr);

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
