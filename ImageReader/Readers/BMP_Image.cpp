////////////////////////////////////////////////////////////////////////////////////////////
// Filename:     BMP_Image.cpp
// Description:  implementation of functional for the BMP_Image class;
//
// Created:      17.02.24
////////////////////////////////////////////////////////////////////////////////////////////
#include "BMP_Image.h"

#include "../Common/log.h"
#include "DDS_ImageReader.h"
#pragma warning (disable : 4996)

namespace ImgReader 
{

// ********************************************************************************
//                             PUBLIC METHODS
// ********************************************************************************

bool BMP_Image::LoadTextureFromFile(
	const char* bmpFilePath,
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

    DDS_ImageReader ddsImage;

	const bool result = ddsImage.LoadTextureFromFile(
		bmpFilePath,
		pDevice,
		ppTexture,
		ppTextureView,
		textureWidth,
		textureHeight);

    if (!result)
    {
        sprintf(g_String, "can't load a BMP texture from the file: %s", bmpFilePath);
        LogErr(g_String);
        return false;
    }

    return true;
}

} // namespace ImgReader
