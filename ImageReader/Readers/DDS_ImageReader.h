// *********************************************************************************
// Filename:     DDS_ImageReader.h
// Description:  textures loader/initializer of the .dds format;
// 
// *********************************************************************************
#pragma once

#include "../Common/Types.h"
#include <d3d11.h>

namespace ImgReader
{

class DDS_ImageReader final
{
public:
	DDS_ImageReader() {}

	bool LoadTextureFromFile(
		const char* filePath,
		ID3D11Device* pDevice,
		ID3D11Resource** ppTexture,
		ID3D11ShaderResourceView** ppTextureView,
		u32& texWidth,
		u32& texHeight);
};


} // namespace ImgReader
