// =================================================================================
// Filename:    ImageReader.cpp
// Description: constains implementation of functional for the ImageReader class;
// =================================================================================
#include "ImageReader.h"

#include "Common/log.h"
#include "Common/LIB_Exception.h"
#include "Common/FileSystem.h"
#include "Common/Assert.h"
#include "Common/StrHelper.h"

// image readers for different types
#include "Readers/DDS_ImageReader.h"
#include "Readers/TARGA_ImageReader.h"
#include "Readers/WICTextureLoader11.h"
#include "Readers/BMP_Image.h"

#include <d3dx11tex.h>


namespace ImgReader
{

// =================================================================================
//                            PUBLIC METHODS
// =================================================================================
bool ImageReader::LoadTextureFromFile(
    ID3D11Device* pDevice,
    DXTextureData& texData)
{
	// load a texture data from file by texData.filePath;
	// and fill in the DXTextureData struct with loaded data

	try
	{	
		CheckInputParams(texData);

        // get texture extension
        char ext[8]{'\0'};
        FileSys::GetFileExt(texData.filePath, ext);


		if ((strcmp(ext, ".png")  == 0) ||
            (strcmp(ext, ".jpg")  == 0) ||
            (strcmp(ext, ".jpeg") == 0))
		{
			LoadPNGTexture(pDevice, texData);
		}
		else if (strcmp(ext, ".dds") == 0)
		{
			LoadDDSTexture(pDevice, texData);
		}
		else if (strcmp(ext, ".tga") == 0)
		{
			LoadTGATexture(pDevice, texData);
		}
		else if (strcmp(ext, ".bmp") == 0)
		{
			LoadBMPTexture(pDevice, texData);
		}
		else
		{
			sprintf(g_String, "unknown image extension: %s", ext);
            LogErr(g_String);
            return false;
		}

        return true;
	}
	catch (LIB_Exception & e)
	{
        LogErr(e);
        sprintf(g_String, "can't load a texture from file: %s", texData.filePath);
		LogErr(g_String);
        return false;
	}
}

///////////////////////////////////////////////////////////

bool ImageReader::LoadTextureFromMemory(
	ID3D11Device* pDevice,
	const uint8_t* pData,
	const size_t size,
	DXTextureData& outTexData)
{
	try
	{
		CheckInputParams(outTexData);
        Assert::True(pData != nullptr, "input ptr to texture raw data");
        Assert::True(size > 0,         "input size of texture data must be > 0");

		ID3D11DeviceContext* pContext = nullptr;
		pDevice->GetImmediateContext(&pContext);

		HRESULT hr = DirectX::CreateWICTextureFromMemory(
			pDevice,
			pContext,                                     // pass the context to make possible auto-gen of mipmaps
			pData,
			size,
			outTexData.ppTexture,
			outTexData.ppTextureView);

		Assert::NotFailed(hr, "can't create a texture from memory");

		// initialize the texture width and height values
		D3D11_TEXTURE2D_DESC desc;
		ID3D11Texture2D* pTex = (ID3D11Texture2D*)(*outTexData.ppTexture);
		pTex->GetDesc(&desc);
		
		outTexData.textureWidth = desc.Width;
		outTexData.textureHeight = desc.Height;

        return true;
	}
	catch (LIB_Exception& e)
	{
		LogErr(e);
		LogErr("can't load texture's data from memory");
        return false;
	}
}


// =================================================================================
//                            PRIVATE METHODS
// =================================================================================
void ImageReader::CheckInputParams(const DXTextureData& data)
{
	Assert::True(
		(data.filePath[0] != '\0') &&
		data.ppTexture && 
		data.ppTextureView, "some of input params are invalid");
}

///////////////////////////////////////////////////////////

void ImageReader::LoadPNGTexture(ID3D11Device* pDevice, DXTextureData& data)
{
    wchar_t wFilePath[256]{ L'\0' };
    StrHelper::StrToWide(data.filePath, wFilePath);

	ID3D11DeviceContext* pContext = nullptr;
	pDevice->GetImmediateContext(&pContext);

	const HRESULT hr = DirectX::CreateWICTextureFromFileEx(
		pDevice,
		pContext,                                              // pass the context to make possible auto-gen of mipmaps
		wFilePath,
		0,                                                     // max size
		D3D11_USAGE_DEFAULT, 
		D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
		0,                                                     // cpuAccessFlags
		D3D11_RESOURCE_MISC_GENERATE_MIPS,                     // generate mipmaps
		DirectX::WIC_LOADER_DEFAULT,
		data.ppTexture,
		data.ppTextureView);

    if (FAILED(hr))
    {
        sprintf(g_String, "can't create a PNG texture from file: %s", data.filePath);
        LogErr(g_String);
    }

	// initialize the texture width and height values
	D3D11_TEXTURE2D_DESC desc;
	ID3D11Texture2D* pTex = (ID3D11Texture2D*)(*data.ppTexture);
	pTex->GetDesc(&desc);

	data.textureWidth = desc.Width;
	data.textureHeight = desc.Height;
}

///////////////////////////////////////////////////////////

void ImageReader::LoadDDSTexture(ID3D11Device* pDevice, DXTextureData& data)
{
	DDS_ImageReader reader;

	reader.LoadTextureFromFile(
		data.filePath,
		pDevice,
		data.ppTexture,
		data.ppTextureView,
		data.textureWidth,
		data.textureHeight);
}

///////////////////////////////////////////////////////////

void ImageReader::LoadTGATexture(ID3D11Device* pDevice, DXTextureData& data)
{
	TARGA_ImageReader reader;

	reader.LoadTextureFromFile(
		data.filePath,
		pDevice,
		data.ppTexture,
		data.ppTextureView,
		data.textureWidth,
		data.textureHeight);
}

///////////////////////////////////////////////////////////

void ImageReader::LoadBMPTexture(ID3D11Device* pDevice, DXTextureData& data)
{
	BMP_Image reader;

	reader.LoadTextureFromFile(
		data.filePath,
		pDevice,
		data.ppTexture,
		data.ppTextureView,
		data.textureWidth,
		data.textureHeight);
}


} // namespace ImgReader
