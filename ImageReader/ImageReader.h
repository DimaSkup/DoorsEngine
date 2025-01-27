#pragma once

#include "Common/Types.h"

#include <string>
#include <d3d11.h>
#include <list>
//#include <dxgitype.h>
//#include <dxgi.h>

namespace ImgReader
{

class ImageReader
{
public:
	struct DXTextureData
	{
		DXTextureData(
			const std::string& path,
			ID3D11Resource** ppTex,
			ID3D11ShaderResourceView** ppTexView) 
			:
			filePath(path),
			ppTexture(ppTex),
			ppTextureView(ppTexView) {}

		std::string filePath;
		ID3D11Resource** ppTexture = nullptr;
		ID3D11ShaderResourceView** ppTextureView = nullptr;
		UINT textureWidth = 0;
		UINT textureHeight = 0;
	};

public:
	ImageReader() {};

	void SetupLogger(FILE* pFile, std::list<std::string>* pMsgsList);

	void LoadTextureFromFile(
		ID3D11Device* pDevice,
		DXTextureData& texData);

	void LoadTextureFromMemory(
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