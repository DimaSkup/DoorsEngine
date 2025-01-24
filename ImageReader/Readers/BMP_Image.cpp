////////////////////////////////////////////////////////////////////////////////////////////
// Filename:     BMP_Image.cpp
// Description:  implementation of functional for the BMP_Image class;
//
// Created:      17.02.24
////////////////////////////////////////////////////////////////////////////////////////////
#include "BMP_Image.h"

#include "../Common/log.h"
#include "DDS_ImageReader.h"
#include <fstream>

#include <DirectXColors.h>


namespace ImgReader 
{


// ********************************************************************************
//                             PUBLIC METHODS
// ********************************************************************************

void BMP_Image::LoadTextureFromFile(
	const std::string & bmpFilePath,
	ID3D11Device* pDevice,
	ID3D11Resource** ppTexture,
	ID3D11ShaderResourceView** ppTextureView,
	u32& textureWidth,
	u32& textureHeight)
{
	// load texture's data from file by filePath and initialize a texture resource and
	// a shader resource view with this data

	// because we can use the same loading both for dds and bmp files
	// we just use the DDS_ImageReader for processing the input file by bmpFilePath
	try
	{
		DDS_ImageReader ddsImage;

		ddsImage.LoadTextureFromFile(
			bmpFilePath,
			pDevice,
			ppTexture,
			ppTextureView,
			textureWidth,
			textureHeight);
	}
	catch (LIB_Exception& e)
	{
		const std::string errMsg{ "can't load a BMP texture from the file: " + bmpFilePath };

		Log::Error(e);
		Log::Error(errMsg);
		throw LIB_Exception(errMsg);
	}
}

} // namespace ImgReader