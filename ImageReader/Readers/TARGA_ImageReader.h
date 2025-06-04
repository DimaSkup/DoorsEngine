// =================================================================================
// Filename:     TARGA_ImageReader.h
// Description:  textures loader/initializer of the .tga format;
// 
// =================================================================================
#pragma once

#include <d3d11.h>

namespace ImgReader
{

class TARGA_ImageReader
{

public:
    TARGA_ImageReader() {};

    bool LoadTextureFromFile(
        const char* filePath,
        ID3D11Device* pDevice,
        ID3D11Resource** ppTexture,
        ID3D11ShaderResourceView** ppTextureView,
        UINT& texWidth,
        UINT& texHeight);
};

} // namespace ImgReader
