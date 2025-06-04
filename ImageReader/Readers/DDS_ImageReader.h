// *********************************************************************************
// Filename:     DDS_ImageReader.h
// Description:  textures loader/initializer of the .dds format;
// 
// *********************************************************************************
#pragma once

#include <d3d11.h>

namespace ImgReader
{

class DDS_ImageReader
{
public:
	DDS_ImageReader() {}

	bool LoadTextureFromFile(
		const char* filePath,
		ID3D11Device* pDevice,
		ID3D11Resource** ppTexture,
		ID3D11ShaderResourceView** ppTextureView,
		uint32_t& texWidth,
        uint32_t& texHeight);
};


} // namespace ImgReader
