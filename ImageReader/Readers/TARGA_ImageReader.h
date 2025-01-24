// *********************************************************************************
// Filename:     TARGA_ImageReader.h
// Description:  textures loader/initializer of the .tga format;
// 
// *********************************************************************************
#pragma once

#include "../Common/Types.h"
#include <d3d11.h>
#include <string>


namespace ImgReader
{


class TARGA_ImageReader final
{

public:
	TARGA_ImageReader() {};

	void LoadTextureFromFile(
		const std::string & filePath,
		ID3D11Device* pDevice,
		ID3D11Resource** ppTexture,
		ID3D11ShaderResourceView** ppTextureView,
		UINT& texWidth,
		UINT& texHeight);
};

} // namespace ImgReader