// *********************************************************************************
// Filename:     DDS_ImageReader.h
// Description:  textures loader/initializer of the .dds format;
// 
// *********************************************************************************
#pragma once

#include <d3d11.h>


namespace Img
{

class DDS_ImageReader
{
public:
    static bool LoadTextureFromFile(
        const char* filePath,
        ID3D11Device* pDevice,
        ID3D11Resource** ppTexture,
        ID3D11ShaderResourceView** ppTextureView,
        UINT& texWidth,
        UINT& texHeight);
};


} // namespace ImgReader
