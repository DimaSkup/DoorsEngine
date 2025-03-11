// =================================================================================
// Filename: textureclass.cpp
// =================================================================================
#include "textureclass.h"

#include "ImageReader.h"
#include "../../ImageReader/Common/LIB_Exception.h"

#include <CoreCommon/Log.h>
#include <CoreCommon/MemHelpers.h>
#include <CoreCommon/Assert.h>
#include <CoreCommon/StringHelper.h>

#include <D3DX11tex.h>
#include <vector>            // TEMPORARY

#pragma warning (disable : 4996)


namespace Core
{
	
// =================================================================================
// CONSTRUCTORS / DESTRUCTOR
// =================================================================================

Texture::Texture()
{
	// default constructor
}

///////////////////////////////////////////////////////////

Texture::Texture(
	ID3D11Device* pDevice, 
	const std::string& filePath) :
	path_(filePath)
{
	// create and initialize a texture from a file by filePath

	try
	{
		LoadFromFile(pDevice, filePath);
	}
	catch (EngineException & e)
	{
		Log::Error(e);
		throw EngineException("can't create a texture from file: " + filePath);
	}
}

///////////////////////////////////////////////////////////

Texture::Texture(
	ID3D11Device* pDevice,
	const std::string& texArrLabel,
	const std::string* filenames,
	const int numFilenames,
	const DXGI_FORMAT format)
	//const UINT filter,
	//const UINT mipFilter)
{
	// a constructor for loading multiple textures to create a Texture2DArray

	Assert::True((pDevice != nullptr) && (filenames != nullptr) && (numFilenames > 0) && (!texArrLabel.empty()), "invalid input data");

	// load the texture elements individually from file. These texture won't
	// be used by the GPU (0 bind flags), they are just used to load the image 
	// data from file. We use the STAGING usage so the CPU can read the resource.
	HRESULT hr = S_OK;
	UINT size = (UINT)numFilenames;
	std::vector<ID3D11Texture2D*> srcTex(size, nullptr);

	for (UINT i = 0; i < size; ++i)
	{
		D3DX11_IMAGE_LOAD_INFO loadInfo;
		loadInfo.Width          = D3DX11_FROM_FILE;
		loadInfo.Height         = D3DX11_FROM_FILE;
		loadInfo.Depth          = D3DX11_FROM_FILE;
		loadInfo.FirstMipLevel  = 0;
		loadInfo.MipLevels      = D3DX11_FROM_FILE;
		loadInfo.Usage          = D3D11_USAGE_STAGING;
		loadInfo.BindFlags      = 0;
		loadInfo.CpuAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
		loadInfo.MiscFlags      = 0;
		loadInfo.Format         = format;
		loadInfo.Filter         = D3DX11_DEFAULT;//filter;
		loadInfo.MipFilter      = D3DX11_DEFAULT;//mipFilter;
		loadInfo.pSrcInfo       = 0;

		hr = D3DX11CreateTextureFromFile(
			pDevice,
			StringHelper::StringToWide(filenames[i]).c_str(),
			&loadInfo,
			nullptr,
			(ID3D11Resource**)&srcTex[i], 
			nullptr);
		Assert::NotFailed(hr, "can't create a texture from file: " + filenames[i]);
	}


	// Create the texture array. Each element in the texture array
	// has the same format/dimensions
	ID3D11DeviceContext* pContext = nullptr;
	D3D11_TEXTURE2D_DESC texElemDesc;
	D3D11_TEXTURE2D_DESC texArrayDesc;
	ID3D11Texture2D* texArr = nullptr;

	srcTex[0]->GetDesc(&texElemDesc);
	pDevice->GetImmediateContext(&pContext);
		
	texArrayDesc.Width              = texElemDesc.Width;
	texArrayDesc.Height             = texElemDesc.Height;
	texArrayDesc.MipLevels          = texElemDesc.MipLevels;
	texArrayDesc.ArraySize          = size;
	texArrayDesc.Format             = texElemDesc.Format;
	texArrayDesc.SampleDesc.Count   = 1;
	texArrayDesc.SampleDesc.Quality = 0;
	texArrayDesc.Usage              = D3D11_USAGE_DEFAULT;
	texArrayDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
	texArrayDesc.CPUAccessFlags     = 0;
	texArrayDesc.MiscFlags          = 0;

	hr = pDevice->CreateTexture2D(&texArrayDesc, nullptr, &texArr);
	Assert::NotFailed(hr, "can't create a texture array");


	//
	// copy individual texture element into texture array
	//

	// for each texture element...
	for (UINT texElem = 0; texElem < size; ++texElem)
	{
		// for each mipmap level...
		for (UINT mipLevel = 0; mipLevel < texElemDesc.MipLevels; ++mipLevel)
		{
			D3D11_MAPPED_SUBRESOURCE mappedTex2D;

			hr = pContext->Map(srcTex[texElem], mipLevel, D3D11_MAP_READ, 0, &mappedTex2D);
			Assert::NotFailed(hr, "can't map a texture by idx: " + std::to_string(texElem));

			pContext->UpdateSubresource(
				texArr,                       // dst resource
				D3D11CalcSubresource(         // compute idx identifying the subresource we are updating in the dst resource
					mipLevel,
					texElem,
					texElemDesc.MipLevels),
				nullptr,                      // pDstBox - a ptr to a D3D11_BOX instance that specifies the volume in the destination subresource we are updating; specify null to update the entire subresource
				mappedTex2D.pData,            // ptr to the src data
				mappedTex2D.RowPitch,         // byte size of one row of the src data
				mappedTex2D.DepthPitch);      // byte size of one depth slice of the src data

			pContext->Unmap(srcTex[texElem], mipLevel);
		}
	}


	//
	// Create a resource view to the texture array
	//

	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format                         = texArrayDesc.Format;
	viewDesc.ViewDimension                  = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	viewDesc.Texture2DArray.MostDetailedMip = 0;
	viewDesc.Texture2DArray.MipLevels       = texArrayDesc.MipLevels;
	viewDesc.Texture2DArray.FirstArraySlice = 0;
	viewDesc.Texture2DArray.ArraySize       = size;

	ID3D11ShaderResourceView* texArrSRV = 0;

	hr = pDevice->CreateShaderResourceView(texArr, &viewDesc, &texArrSRV);

	//
	// Cleanup -- we only need the resource view
	//

	for (UINT i = 0; i < size; ++i)
		SafeRelease(&srcTex[i]);

	// assignment
	pTexture_     = texArr;
	pTextureView_ = texArrSRV;
	width_        = texArrayDesc.Width;
	height_       = texArrayDesc.Height;
	path_         = texArrLabel;
}

///////////////////////////////////////////////////////////

Texture::Texture(
	ID3D11Device* pDevice, 
	const Color & color)
{
	// THIS FUNC creates a 1x1 texture with input color value

	try
	{
		Initialize1x1ColorTexture(pDevice, color);
	}
	catch (EngineException& e)
	{
		Log::Error(e);
		throw EngineException("can't create a texture by input color data");
	}
}

///////////////////////////////////////////////////////////

Texture::Texture(
	ID3D11Device* pDevice, 
	const Color* pColorData, 
	const UINT width,
	const UINT height)
{
	// THIS FUNC creates a width_x_height texture with input color data

	try
	{
		Assert::NotNullptr(pColorData, "the input ptr to color data == nullptr");
		Assert::True((bool)(width & height), "texture dimensions must be greater that zero");

		InitializeColorTexture(pDevice, pColorData, width, height);
	}
	catch (EngineException& e)
	{
		Log::Error(e);
		throw EngineException("can't create a texture by given color data");
	}	
}

///////////////////////////////////////////////////////////

Texture::Texture(
	ID3D11Device* pDevice,
	const std::string& path,
	const uint8_t* pData,
	const size_t size)
	:
	path_(path)
{
	// a constructor for loading embedded compressed textures;
	// 
	// source texture can be compressed or embedded so we firstly load its 
	// content into the memory and then passed here ptr to this loaded data;


	try
	{
		Assert::True((pData != nullptr) && (size > 0), "wrong input data");

		ImgReader::ImageReader imageReader;
		ImgReader::ImageReader::DXTextureData texData(path, &pTexture_, &pTextureView_);

		imageReader.LoadTextureFromMemory(pDevice, pData, size, texData);

		width_ = texData.textureWidth;
		height_ = texData.textureHeight;
	}
	catch (ImgReader::LIB_Exception& e)
	{
		Log::Error(e.GetStr());

		// if we didn't manage to initialize texture's data from the memory
		// we create a 1x1 color texture for this texture object
		Initialize1x1ColorTexture(pDevice, Colors::UnloadedTextureColor);
	}
	catch (EngineException& e)
	{
		Log::Error(e);
		Log::Error("can't create an embedded compressed texture");

		Initialize1x1ColorTexture(pDevice, Colors::UnloadedTextureColor);
	}
}

///////////////////////////////////////////////////////////

Texture::Texture(Texture&& rhs) noexcept :
	path_(std::exchange(rhs.path_, "")),
	pTexture_(std::exchange(rhs.pTexture_, nullptr)),
	pTextureView_(std::exchange(rhs.pTextureView_, nullptr)),
	width_(rhs.width_),
	height_(rhs.height_)
{
	// move constructor
}

///////////////////////////////////////////////////////////

Texture& Texture::operator=(Texture&& rhs) noexcept
{
	// move assignment
	if (this != &rhs)
	{
		Clear();                    // lifetime of *this ends
		std::construct_at(this, std::move(rhs));
	}

	return *this;
}

///////////////////////////////////////////////////////////

Texture::~Texture()
{
	Clear();
}


// =================================================================================
// Public API
// =================================================================================

void Texture::Copy(Texture& src)
{
	this->Copy(src.pTexture_);

	// copy common data of the texture instance
	path_   = src.path_;
	width_  = src.width_;
	height_ = src.height_;
}

///////////////////////////////////////////////////////////

void Texture::Copy(ID3D11Resource* const pSrcTexResource)
{
	// deep copy: execute copying of the src texture into the current;
	// note:      we discard all the previous data of the current texture;

	// guard self assignment
	if (this->pTexture_ == pSrcTexResource)
		return;

	// clear the previous data
	Clear();

	ID3D11Device* pDevice = nullptr;
	ID3D11DeviceContext* pContext = nullptr;

	// get ptrs to the device and device context
	pSrcTexResource->GetDevice(&pDevice);
	pDevice->GetImmediateContext(&pContext);


	// ------------------------------------------
	
	HRESULT                  hr = S_OK;
	uint32_t*                pixelsData  = nullptr;
	ID3D11Texture2D*         pSrcTexture = static_cast<ID3D11Texture2D*>(pSrcTexResource);
	ID3D11Texture2D*         pDstTexture = nullptr;
	ID3D11Texture2D*         pTextureBuf = nullptr;
	D3D11_TEXTURE2D_DESC     textureBufDesc;
	D3D11_TEXTURE2D_DESC     dstTextureDesc;
	D3D11_SUBRESOURCE_DATA   initialData;
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;

	// create a staging texture for resource copying
	pSrcTexture->GetDesc(&textureBufDesc);
	textureBufDesc.BindFlags      = 0;
	textureBufDesc.Usage          = D3D11_USAGE_STAGING;
	textureBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	hr = pDevice->CreateTexture2D(&textureBufDesc, nullptr, &pTextureBuf);
	Assert::NotFailed(hr, "Failed to create a staging texture");

	// copy the data from the source texture
	pContext->CopyResource(pTextureBuf, pSrcTexture);

	// map the staging buffer
	hr = pContext->Map(pTextureBuf, 0, D3D11_MAP_READ, 0, &mappedSubresource);
	Assert::NotFailed(hr, "can't map the staging buffer");

	// copy src texture data into the temporal buffer
	const int numPixels = textureBufDesc.Width * textureBufDesc.Height;
	pixelsData = new uint32_t[numPixels];
	memcpy(pixelsData, mappedSubresource.pData, sizeof(uint32_t) * numPixels);

	pContext->Unmap(pTextureBuf, 0);


	// ----------------------------------------------------

	// setup initial data for a dst texture
	initialData.pSysMem = pixelsData;
	initialData.SysMemPitch = mappedSubresource.RowPitch;

	// create a dst texture with the same params as the src one
	pSrcTexture->GetDesc(&dstTextureDesc);
	dstTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	hr = pDevice->CreateTexture2D(&dstTextureDesc, &initialData, &pDstTexture);
	Assert::NotFailed(hr, "Failed to create a dst texture");

	// store a ptr to the 2D texture 
	pTexture_ = pDstTexture;


	// define which view dimension we need to use
	UINT sdCount   = dstTextureDesc.SampleDesc.Count;
	UINT sdQuality = dstTextureDesc.SampleDesc.Quality;
	bool no4xMSAA  = ((sdCount == 1) && (sdQuality == 0));

	// create a new shader resource view (SRV) for the dst texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format                    = dstTextureDesc.Format;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels       = dstTextureDesc.MipLevels;
	srvDesc.ViewDimension             = (no4xMSAA) ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D_SRV_DIMENSION_TEXTURE2DMS;

	hr = pDevice->CreateShaderResourceView(pTexture_, &srvDesc, &pTextureView_);
	Assert::NotFailed(hr, "Failed to create shader resource view (SRV)");


	// ----------------------------------------------------

	// copy common data of the src texture
	path_   = "copy_without_path";
	width_  = dstTextureDesc.Width;
	height_ = dstTextureDesc.Height;

	// after all clear all the temp data
	SafeDeleteArr(pixelsData);
	SafeRelease(&pTextureBuf);
}

///////////////////////////////////////////////////////////

POINT Texture::GetTextureSize()
{
	ID3D11Texture2D* p2DTexture = static_cast<ID3D11Texture2D*>(pTexture_);
	D3D11_TEXTURE2D_DESC desc;
	
	p2DTexture->GetDesc(&desc);
	
	return { (LONG)desc.Width, (LONG)desc.Height };
}


// =================================================================================
// Private API
// =================================================================================

void Texture::Clear()
{
	width_ = 0;
	height_ = 0;
	path_.clear();
	SafeRelease(&pTexture_);
	SafeRelease(&pTextureView_);
}

///////////////////////////////////////////////////////////

void Texture::LoadFromFile(
	ID3D11Device* pDevice,
	const std::string & filePath)
{
	// create a texture's resource and shader resource view loading texture data from the file
	try
	{
		ImgReader::ImageReader imageReader;
		ImgReader::ImageReader::DXTextureData data(filePath, &pTexture_, &pTextureView_);

		imageReader.LoadTextureFromFile(pDevice, data);

		path_ = filePath;
		width_ = data.textureWidth;
		height_ = data.textureHeight;
	}
	catch (ImgReader::LIB_Exception& e)
	{
		Log::Error(e.GetStr());

		// if we didn't manage to initialize a texture from the file 
		// we create a 1x1 color texture for this texture object
		Initialize1x1ColorTexture(pDevice, Colors::UnloadedTextureColor);
	}
	catch (EngineException & e)
	{
		Log::Error(e);
		throw EngineException("can't initialize a texture from file");
	}
}


///////////////////////////////////////////////////////////

void Texture::Initialize1x1ColorTexture(ID3D11Device* pDevice, const Color & colorData)
{
	InitializeColorTexture(pDevice, &colorData, 1, 1);
}

///////////////////////////////////////////////////////////

void Texture::InitializeColorTexture(
	ID3D11Device* pDevice,
	const Color* pColorData,
	const UINT width,
	const UINT height)
{
	// Initialize a color texture using input color data (pColorData) and
	// the input width/height

	width_ = width;
	height_ = height;

	ID3D11Texture2D* p2DTexture = nullptr;
	D3D11_TEXTURE2D_DESC textureDesc;
	D3D11_SUBRESOURCE_DATA initialData{};

	// setup description for this texture
	textureDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.Width              = width;
	textureDesc.Height             = height;
	textureDesc.ArraySize          = 1;
	textureDesc.MipLevels          = 0;
	textureDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.Usage              = D3D11_USAGE_DEFAULT;
	textureDesc.CPUAccessFlags     = 0;
	textureDesc.SampleDesc.Count   = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.MiscFlags          = 0;

	// setup initial data for this texture
	initialData.pSysMem = pColorData;
	initialData.SysMemPitch = width * sizeof(Color);

	// create a new 2D texture
	HRESULT hr = pDevice->CreateTexture2D(&textureDesc, &initialData, &p2DTexture);
	Assert::NotFailed(hr, "Failed to initialize texture from color data");

	// store a ptr to the 2D texture 
	//pTexture_ = static_cast<ID3D11Texture2D*>(p2DTexture);
	pTexture_ = p2DTexture;

	// setup description for a shader resource view (SRV)
	CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D, textureDesc.Format);

	// create a new SRV from texture
	hr = pDevice->CreateShaderResourceView(pTexture_, &srvDesc, &pTextureView_);
	Assert::NotFailed(hr, "Failed to create shader resource view from texture generated from color data");
}

} // namespace Core