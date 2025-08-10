#pragma once

#include <log.h>
#include <stdint.h>
#include <d3d11.h>
#include <stdio.h>


#pragma warning (disable : 4996)


namespace Img
{

struct DXTextureData
{
    DXTextureData(
        const char* path,
        ID3D11Resource** ppTex,
        ID3D11ShaderResourceView** ppTexView)
        :
        ppTexture(ppTex),
        ppTextureView(ppTexView)
    {
        if (!path || path[0] == '\0')
            LogErr(LOG, "input path is empty!");

        else
            strcpy(filePath, path);
    }

    char                        filePath[256]{ '\0' };
    ID3D11Resource**            ppTexture       = nullptr;
    ID3D11ShaderResourceView**  ppTextureView   = nullptr;
    UINT                        textureWidth    = 0;
    UINT                        textureHeight   = 0;
};

///////////////////////////////////////////////////////////

class ImageReader
{
public:
    ImageReader() {};

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
    bool LoadPNGTexture(ID3D11Device* pDevice, DXTextureData& data);
    bool LoadDDSTexture(ID3D11Device* pDevice, DXTextureData& data);
    bool LoadTGATexture(ID3D11Device* pDevice, DXTextureData& data);
    bool LoadBMPTexture(ID3D11Device* pDevice, DXTextureData& data);
};

} // namespace ImgReader
