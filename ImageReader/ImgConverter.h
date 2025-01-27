// ********************************************************************************
// Filename:      ImgConverter.h
// Description:   functional for storing image as .dds
// 
//                supported input extensions: .bmp, .png, .gif, .tiff, .jpeg
//                
//                features:
//                1. convertation between uncompressed formats;
//                2. decompression from compressed formats;
// 
// Created:       12.11.24
// ********************************************************************************
#pragma once

#include <DirectXTex.h>
#include <filesystem>
#include <string>
#include <d3d11.h>


namespace fs = std::filesystem;


namespace ImgReader
{
	
class ImgConverter
{
public:
	void LoadFromFile(const fs::path& filepath, DirectX::ScratchImage& srcImg);

	void LoadFromMemory(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		ID3D11Resource* pTexture,
		DirectX::ScratchImage& image);

	void GenDstImgPath(const fs::path& srcPath, fs::path& dstPath);

	void Convert(
		const DirectX::ScratchImage& srcImg,
		const DXGI_FORMAT dstFormat,
		const DirectX::ConvertOptions& opts,
		DirectX::ScratchImage& dstImg);

	void Decompress(
		const DirectX::ScratchImage& srcImg,
		const DXGI_FORMAT dstFormat,
		DirectX::ScratchImage& dstImg);

	void Compress(
		const DirectX::ScratchImage& srcImg,
		const DXGI_FORMAT dstFormat,
		const DirectX::CompressOptions opts,
		DirectX::ScratchImage& dstImg);

	DirectX::ScratchImage GenMipMaps(
		DirectX::ScratchImage& srcImage,
		const DirectX::TEX_FILTER_FLAGS filter);

	void ProcessImage(
		const DirectX::ScratchImage& srcImage,
		const DXGI_FORMAT dstFormat,
		DirectX::ScratchImage& dstImage);

	void SaveToFile(
		const DirectX::ScratchImage& image,
		const DirectX::DDS_FLAGS flags,
		const fs::path& dstPath);
};

}