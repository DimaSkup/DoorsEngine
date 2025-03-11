// *********************************************************************************
// Filename:      textureclass.h
// Description:   Encapsulates the loading, unloading, and accessing
//                of a single texture resource. For each texture 
//                needed an object of this class to be instantiated.
//
// Revising:      09.04.22
// *********************************************************************************
#pragma once

#include <CoreCommon/Types.h>
#include "../Render/Color.h"

#include <assimp/material.h>
#include <d3d11.h>


namespace Core
{

enum class TexStoreType
{
	Invalid,
	None,
	EmbeddedIndexCompressed,
	EmbeddedIndexNonCompressed,
	EmbeddedCompressed,
	EmbeddedNonCompressed,
	Disk
};

///////////////////////////////////////////////////////////

class Texture
{
public:
	static const u32 TEXTURE_TYPE_COUNT = 22;     // AI_TEXTURE_TYPE_MAX + 1
	
	struct TexDimensions
	{
		UINT width = 0;
		UINT height = 0;
	};


public:
	Texture();

	// move constructor/assignment
	Texture(Texture&& rhs) noexcept;
	Texture& operator=(Texture&& rhs) noexcept;

	// restrict shallow copying
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	// a constructor for loading textures from the disk
	Texture(
		ID3D11Device* pDevice,
		const std::string & filePath);	

	// a constructor for loading multiple textures to create a Texture2DArray
	Texture(
		ID3D11Device* pDevice,
		const std::string& texArrLabel,
		const std::string* arrFilenames,
		const int numFilenames,
		const DXGI_FORMAT format);
		//const UINT filter,
		//const UINT mipFilter);

	// make 1x1 texture with single color
	Texture(
		ID3D11Device* pDevice, 
		const Color& color);

	// make width_x_height texture with color data
	Texture(
		ID3D11Device* pDevice, 
		const Color* pColorData,
		const UINT width,
		const UINT height);

	// a constructor for loading embedded compressed textures 
	Texture(
		ID3D11Device* pDevice,
		const std::string& path,
		const uint8_t* pData,
		const size_t size);

	~Texture();

	// --------------------------------------------------------------------------------

	// deep copy
	void Copy(Texture& src);
	void Copy(ID3D11Resource* const pSrcTexResource);


	inline ID3D11Resource*           GetResource()                                { return pTexture_; }
	inline ID3D11ShaderResourceView* GetTextureResourceView()               const { return pTextureView_; }
	inline ID3D11ShaderResourceView* const* GetTextureResourceViewAddress() const { return &pTextureView_; }

	inline const std::string& GetPath() const { return path_; }
	inline UINT GetWidth()              const { return width_; }
	inline UINT GetHeight()             const { return height_; }

	POINT GetTextureSize();

	// set where to store this texture if it was generated
	inline void SetPath(const std::string& newPath)  { path_ = newPath; } 

	// --------------------------------------------------------------------------------

private:
	void Clear();

	void LoadFromFile(
		ID3D11Device* pDevice, 
		const std::string & filePath);

	void Initialize1x1ColorTexture(
		ID3D11Device* pDevice, 
		const Color & colorData);

	void InitializeColorTexture(
		ID3D11Device* pDevice, 
		const Color* pColorData, 
		const UINT width, 
		const UINT height);

private:
	std::string path_ {"no_path"};                       

	ID3D11Resource* pTexture_ = nullptr;
	ID3D11ShaderResourceView* pTextureView_ = nullptr;   

	UINT width_ = 30;                                   
	UINT height_ = 30;
};

} // namespace Core