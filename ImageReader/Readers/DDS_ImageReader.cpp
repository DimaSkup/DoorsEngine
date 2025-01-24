#include "DDS_ImageReader.h"

#include "../Common/StringHelper.h"
#include "../Common/log.h"
#include "../Common/Assert.h"

#include "DDSTextureLoader11.h"

#include <d3dx11tex.h>

namespace ImgReader
{


void DDS_ImageReader::LoadTextureFromFile(
	const std::string & filePath,
	ID3D11Device* pDevice,
	ID3D11Resource** ppTexture,
	ID3D11ShaderResourceView** ppTextureView,
	u32& texWidth,
	u32& texHeight)
{
	// this function loads a DDS texture from the file by filePath
	// and initializes input parameters: texture resource, shader resource view,
	// width and height of the texture;


	try
	{
		HRESULT hr = S_OK;
		const std::wstring wFilePath{ StringHelper::StringToWide(filePath) };

		D3DX11_IMAGE_LOAD_INFO loadInfo;
		loadInfo.MipLevels = 0;

		// create a shader resource view from the texture file
		hr = D3DX11CreateShaderResourceViewFromFile(pDevice,
			wFilePath.c_str(),   // src file path
			&loadInfo,             // ptr load info
			nullptr,             // ptr pump
			ppTextureView,       // pp shader resource view
			nullptr);            // pHresult
		Assert::NotFailed(hr, "err during D3DX11CreateShaderResourceViewFromFile() for a file: " + filePath);

		// initialize a texture resource using the shader resource view
		(*ppTextureView)->GetResource(ppTexture);


		// load information about the texture
		D3DX11_IMAGE_INFO imageInfo;
		hr = D3DX11GetImageInfoFromFile(wFilePath.c_str(), nullptr, &imageInfo, nullptr);
		Assert::NotFailed(hr, "err during D3DX11GetImageInfoFromFile() for a file : " + filePath);

		// initialize the texture width and height values
		texWidth = imageInfo.Width;
		texHeight = imageInfo.Height;
	}
	catch (LIB_Exception& e)
	{
		Log::Error(e);
		Log::Error("can't load a DDS texture from the file: " + filePath);
	}
}


} // namespace ImgReader