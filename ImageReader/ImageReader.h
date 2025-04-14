#pragma once

#include "Common/Types.h"
#include <d3d11.h>
#include <stdio.h>

#pragma warning (disable : 4996)


namespace ImgReader
{

struct DXTextureData
{
    DXTextureData(
        const char* path,
        ID3D11Resource** ppTex,
        ID3D11ShaderResourceView** ppTexView) :
        ppTexture(ppTex),
        ppTextureView(ppTexView)
    {
        strcpy(filePath, path);
    }

    char filePath[256]{ '\0' };
    ID3D11Resource** ppTexture = nullptr;
    ID3D11ShaderResourceView** ppTextureView = nullptr;
    UINT textureWidth = 0;
    UINT textureHeight = 0;
};

///////////////////////////////////////////////////////////

class ImageReader
{
public:
    ImageReader() {};

    void SetupLogger(FILE* pFile);

    bool LoadTextureFromFile(
        ID3D11Device* pDevice,
        DXTextureData& texData);

    bool LoadTextureFromMemory(
        ID3D11Device* pDevice,
        const uint8_t* pData,
        const size_t size,
        DXTextureData& outTexData);

private:
    void CheckInputParams(const DXTextureData& data);
    void LoadPNGTexture(ID3D11Device* pDevice, DXTextureData& data);
    void LoadDDSTexture(ID3D11Device* pDevice, DXTextureData& data);
    void LoadTGATexture(ID3D11Device* pDevice, DXTextureData& data);
    void LoadBMPTexture(ID3D11Device* pDevice, DXTextureData& data);
};

} // namespace ImgReader
