#include "TARGA_ImageReader.h"

#include "../Common/log.h"
#include "../Common/Assert.h"

#include <vector>

namespace ImgReader
{

// we define the Targa file header structure here to 
// make reading in the data easier (for .tga format)
struct TargaHeader
{
	UCHAR data1[12]{ '\0' };
	USHORT width = 0;
	USHORT height = 0;
	UCHAR bpp{ '\0' };
	UCHAR data2{ '\0' };
};

///////////////////////////////////////////////////////////

// predefinition of the helper function
void LoadTarga32Bit(
	const std::string& filePath,
	std::vector<UCHAR>& targaDataArr,   // raw image data
	UINT& textureWidth,
	UINT& textureHeight);

///////////////////////////////////////////////////////////////////////////////////////////
//
//                             PUBLIC FUNCTIONS
//
///////////////////////////////////////////////////////////////////////////////////////////

void TARGA_ImageReader::LoadTextureFromFile(
	const std::string & filePath,
	ID3D11Device* pDevice,
	ID3D11Resource** ppTexture,
	ID3D11ShaderResourceView** ppTextureView,
	UINT& texWidth,
	UINT& texHeight)
{
	// this function loads a TARGA texture from the file by filePath
	// and initializes input parameters: texture resource, shader resource view,
	// width and height of the texture;

	try
	{
		Assert::NotEmpty(filePath.empty(), "a path to targa texture is empty");

		HRESULT hr = S_OK;
		bool result = false;

		UINT rowPitch = 0;
		const UINT bytesOfPixel = 4;

		D3D11_TEXTURE2D_DESC textureDesc;
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

		ID3D11Texture2D* p2DTexture = nullptr;
		ID3D11DeviceContext* pDeviceContext = nullptr;

		// holds the raw Targa data read straight in from the file
		std::vector<UCHAR> targaData;

		// ----------------------------------------------------- //

		// load the targa image data into memory (into the targaDataArr array) 
		LoadTarga32Bit(filePath, targaData, texWidth, texHeight);

		// next we need to setup our description of the DirectX texture that we will load
		// the Targa data into. We use the height and width from the Targa image data, and 
		// set the format to be a 32-bit RGBA texture. We set the SampleDesc to default.
		// Then we set the Usage to D3D11_USAGE_DEFAULT which is better performing memory.
		// And finally, we set the MipLevels, BindFlags, and MiscFlags to the settings 
		// required for Mipmapped textures. Once the description is complete, we call
		// CreateTexture2D() to create an empty texture for us. The next step will be to 
		// copy the Targa data into that empty texture.

		textureDesc.Width = texWidth;   // we've gotten width/height in the LoadTarga32Bit function
		textureDesc.Height = texHeight;
		textureDesc.MipLevels = 0;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;


		// create the empty texture
		hr = pDevice->CreateTexture2D(&textureDesc, nullptr, &p2DTexture);
		Assert::NotFailed(hr, "can't create an empty 2D texture: " + filePath);

		// set the row pitch of the targa image data
		rowPitch = (texWidth * bytesOfPixel) * sizeof(UCHAR);

		// get the device context
		pDevice->GetImmediateContext(&pDeviceContext);

		// copy the targa image data into the texture
		pDeviceContext->UpdateSubresource(p2DTexture, 0, nullptr, targaData.data(), rowPitch, 0);

		targaData.clear();

		// setup the shader resource view description
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = -1;

		// after the texture is loaded, we create a shader resource view which allows us to have
		// a pointer to set the texture in shaders.
		hr = pDevice->CreateShaderResourceView(p2DTexture, &srvDesc, ppTextureView);
		Assert::NotFailed(hr, "can't create the shader resource view: " + filePath);

		// generate mipmaps for this texture
		pDeviceContext->GenerateMips(*ppTextureView);

		// store a ptr to the 2D texture 
		*ppTexture = static_cast<ID3D11Texture2D*>(p2DTexture);
	}
	catch (LIB_Exception& e)
	{
		Log::Error(e);
		Log::Error("can't load a targa texture");
	}
}




///////////////////////////////////////////////////////////////////////////////////////////
//
//                        PRIVATE HELPER FUNCTIONS
//
///////////////////////////////////////////////////////////////////////////////////////////


void LoadTarga32Bit(
	const std::string & filePath,
	std::vector<UCHAR> & targaDataArr,   // raw image data
	UINT & textureWidth,
	UINT & textureHeight)
{
	// this is a Targa image loading function. NOTE that Targa images are stored upside down
	// and need to be flipped before using. So here we will open the file, read it into
	// an array, and then take that array data and load it into the pTargaData_ array in
	// the correct order. Note we are purposely only dealing with 32-bit Targa files that
	// have alpha channels, this function will reject Targa's that are saved as 24-bit


	errno_t error = -1;
	//UCHAR bpp = 0;           // bites per pixel (supposed to be 32)

	TargaHeader targaFileHeader;
	FILE* pFile = nullptr;
	std::vector<UCHAR> targaImageDataArr;

	try
	{
		// open the targa file for reading in binary
		error = fopen_s(&pFile, filePath.c_str(), "rb");
		Assert::True(error == 0, "can't open the targa file for reading in binary: " + filePath);

		// read in the file header
		UINT count = static_cast<UINT>(fread(&targaFileHeader, sizeof(TargaHeader), 1, pFile));
		Assert::True(count == 1, "can't read in the file header: " + filePath);

		// get the important information from the header
		textureWidth = static_cast<UINT>(targaFileHeader.width);
		textureHeight = static_cast<UINT>(targaFileHeader.height);
		//bpp = targaFileHeader.bpp;

		// check that it is 32 bit and not 24 bit
		Assert::True(targaFileHeader.bpp == static_cast<UCHAR>(32), "this targa texture is not 32-bit: " + filePath);

		// calculate the size of the 32 bit image data
		UINT imageSize = textureWidth * textureHeight * 4;

		// allocate memory for the targa image data
		targaImageDataArr.resize(imageSize, 0);

		// allocate memory for the targa destination data
		targaDataArr.resize(imageSize, 0);

		// read in the targa image data
		count = static_cast<UINT>(fread(targaImageDataArr.data(), 1, imageSize, pFile));
		Assert::True(count == imageSize, "can't read in the targa image data from file: " + filePath);

		// close the file
		error = fclose(pFile);
		Assert::True(error == 0, "can't close the file: " + filePath);



		// setup the index into the targa image data
		UINT k = (imageSize)-(textureWidth * 4);

		// now copy the targa image data into the targa destination array in the correct
		// order since the targa format is stored upside down and also is not in RGBA order.
		for (UINT index = 0, j = 0; j < textureHeight; j++)
		{
			for (UINT i = 0; i < textureWidth; i++)
			{
				targaDataArr[index + 0] = targaImageDataArr[k + 2];  // red
				targaDataArr[index + 1] = targaImageDataArr[k + 1];  // green
				targaDataArr[index + 2] = targaImageDataArr[k + 0];  // blue
				targaDataArr[index + 3] = targaImageDataArr[k + 3];  // alpha

				k += 4;
				index += 4;
			}

			// set the targa image data index back to the preceding row at the beginning
			// of the column since its reading is upside down
			k -= (textureWidth * 8);
		}
	}
	catch (std::bad_alloc & e)
	{
		fclose(pFile);              // close the targa file
		Log::Error(e.what());
		throw LIB_Exception("can't allocate memory for the targa image data array / targa destination data array");
	}
	catch (LIB_Exception & e)
	{
		fclose(pFile);              // close the targa file
		Log::Error(e);
		throw LIB_Exception("can't read targa-image data");
	}
}

} // namespace ImgReader