// =================================================================================
// Filename:    ImageReader.cpp
// Description: constains implementation of functional for the ImageReader class;
// =================================================================================
#include "ImageReader.h"

#include "Common/StringHelper.h"
#include "Common/LIB_Exception.h"
#include "Common/log.h"
#include "Common/Assert.h"

// image readers for different types
#include "Readers/DDS_ImageReader.h"
#include "Readers/TARGA_ImageReader.h"
#include "Readers/WICTextureLoader11.h"
#include "Readers/BMP_Image.h"

#include <d3dx11tex.h>
#include <filesystem>


namespace fs = std::filesystem;


namespace ImgReader
{

void ImageReader::SetupLogger(FILE* pFile, std::list<std::string>* pMsgsList)
{
	// setup a file for writing log msgs into it;
	// also setup a list which will be filled with log messages;
	Log::Setup(pFile, pMsgsList);
	Log::Debug("logger is setup successfully");
}



// =================================================================================
// 
//                            PUBLIC METHODS
// 
// =================================================================================


void ImageReader::LoadTextureFromFile(ID3D11Device* pDevice, DXTextureData& texData)
{
	// load a texture data from file by texData.filePath;
	// and fill in the DXTextureData struct with loaded data

	try
	{	
		CheckInputParams(texData);

		const fs::path path = texData.filePath;
		const std::string texExt = path.extension().string();

		if ((texExt == ".png") || (texExt == ".jpg") || (texExt == ".jpeg"))
		{
			LoadPNGTexture(pDevice, texData);
		}
		else if (texExt == ".dds")
		{
			LoadDDSTexture(pDevice, texData);
		}
		else if (texExt == ".tga")
		{
			LoadTGATexture(pDevice, texData);
		}
		else if (texExt == ".bmp")
		{
			LoadBMPTexture(pDevice, texData);
		}
		else
		{
			throw LIB_Exception("UNKNOWN IMAGE EXTENSION");
		}
	}
	catch (LIB_Exception & e)
	{
		const std::string errMgs{ "can't load a texture from file: " + texData.filePath };

		Log::Error(e);
		Log::Error(errMgs);
		throw LIB_Exception(errMgs);
	}
}

///////////////////////////////////////////////////////////

void ImageReader::LoadTextureFromMemory(
	ID3D11Device* pDevice,
	const uint8_t* pData,
	const size_t size,
	DXTextureData& outTexData)
{

	try
	{
		CheckInputParams(outTexData);
		Assert::True((bool)pDevice && (bool)pData && (size > 0), "some of input params are invalid");

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
	}
	catch (LIB_Exception & e)
	{
		const std::string errMsg = "can't load texture's data from memory";
		Log::Error(e);
		Log::Error(errMsg);
		throw LIB_Exception(errMsg);

	}
}



// =================================================================================
// 
//                            PRIVATE METHODS
// 
// =================================================================================

void ImageReader::CheckInputParams(const DXTextureData& data)
{
	Assert::True(
		(!data.filePath.empty()) && 
		data.ppTexture && 
		data.ppTextureView, "some of input params are invalid");
}

///////////////////////////////////////////////////////////

void ImageReader::LoadPNGTexture(ID3D11Device* pDevice, DXTextureData& data)
{
	const std::wstring wFilePath{ StringHelper::StringToWide(data.filePath) };

#if 0
	const HRESULT hr = DirectX::CreateWICTextureFromFile(
		pDevice,
		wFilePath.c_str(),
		data.ppTexture,
		data.ppTextureView,
		0);

#else
	ID3D11DeviceContext* pContext = nullptr;
	pDevice->GetImmediateContext(&pContext);

	const HRESULT hr = DirectX::CreateWICTextureFromFileEx(
		pDevice,
		pContext,                                              // pass the context to make possible auto-gen of mipmaps
		wFilePath.c_str(),
		0,                                                     // max size
		D3D11_USAGE_DEFAULT, 
		D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
		0,                                                     // cpuAccessFlags
		D3D11_RESOURCE_MISC_GENERATE_MIPS,                     // generate mipmaps
		DirectX::WIC_LOADER_DEFAULT,
		data.ppTexture,
		data.ppTextureView);
#endif
	Assert::NotFailed(hr, "can't create a PNG texture from file: " + data.filePath);

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