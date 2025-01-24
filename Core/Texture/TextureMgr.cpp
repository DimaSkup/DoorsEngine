////////////////////////////////////////////////////////////////////
// Filename:      TextureMgr.cpp
// Description:   a manager for work with textures: initialization of
//                ALL the textures, getting it and releasing;
// Created:       06.06.23
////////////////////////////////////////////////////////////////////
#include "TextureMgr.h"

#include "../Common/FileSystemPaths.h"
#include "../Common/StringHelper.h"
#include "../Common/log.h"
#include "../Common/Utils.h"
#include "../Common/Assert.h"

#include "ImageReader.h"

#include <stdexcept>
#include <filesystem>
#include <map>
#include <set>
#include <algorithm>
#include <filesystem>

// for ID generator
#include <cctype>
#include <random>



namespace fs = std::filesystem;
using namespace CoreUtils;

// initialize a static pointer to this class instance
TextureMgr* TextureMgr::pInstance_ = nullptr;

// we use this value as ID for each created/added texture
int TextureMgr::lastTexID_ = 0;


TextureMgr::TextureMgr()
{
	if (pInstance_ == nullptr)
	{
		pInstance_ = this;	

		// reserve some memory ahead
		const u32 reserveForTexCount = 30;
		ids_.reserve(reserveForTexCount);
		names_.reserve(reserveForTexCount);
		textures_.reserve(reserveForTexCount);

		// setup the static logger in the image reader module
		ImgReader::ImageReader imgReader;
		imgReader.SetupLogger(Log::GetFilePtr(), &Log::GetLogMsgsList());
	}
	else
	{
		throw EngineException("you can't have more that only one instance of this class");
	}
}

TextureMgr::~TextureMgr()
{
	ids_.clear();
	names_.clear();
	textures_.clear();   
	idToSRV_.clear();
	
	pInstance_ = nullptr;
}

// ************************************************************************************
//
//                            PUBLIC CREATION API
//
// ************************************************************************************

void TextureMgr::Initialize(ID3D11Device* pDevice)
{
	// check input params
	Assert::NotNullptr(pDevice, "ptr to the device == nullptr");
	pDevice_ = pDevice;

	// create and store a couple of default textures
	//LoadFromFile(g_TexDirPath + "notexture.dds");
	AddDefaultTex("unloaded", { pDevice, Colors::UnloadedTextureColor });
	AddDefaultTex("unhandled_texture", { pDevice, Colors::UnhandledTextureColor });
}

///////////////////////////////////////////////////////////

TexID TextureMgr::Add(const TexName& name, Texture& tex)
{
	// add a new texture by name (key)

	try
	{
		Assert::NotEmpty(name.empty(), "a texture name (path) cannot be empty");

		// if there is already a texture by such name we just return its ID
		const bool isUniqueName = !ArrHasVal(names_, name);
		if (!isUniqueName) return GetIDByName(name);

		const TexID id = GenID();

		ids_.push_back(id);
		names_.push_back(name);
		textures_.push_back(std::move(tex));

		UpdateMapIdToSRV();

		// return an id of the added texture
		return id;
	}
	catch (EngineException& e)
	{
		Log::Error(e);
		throw EngineException("can't add a texture by name: " + name);
	}
}

///////////////////////////////////////////////////////////

TexID TextureMgr::Add(const TexName& name, Texture&& tex)
{
	// add a new texture by name (key)

	try
	{
		Assert::NotEmpty(name.empty(), "a texture name (path) cannot be empty");

		// check if there is already a texture by such a name
		// if so we just return its ID
		const bool isUniqueName = !ArrHasVal(names_, name);
		if (!isUniqueName)
		{
			return GetIDByName(name);
		}

		const TexID id = GenID();

		ids_.push_back(id);
		names_.push_back(name);
		textures_.push_back(std::move(tex));

		UpdateMapIdToSRV();

		// return an ID of the added texture
		return id;
	}
	catch (EngineException& e)
	{
		Log::Error(e);
		throw EngineException("can't add a texture by name: " + name);
	}
}

///////////////////////////////////////////////////////////

TexID TextureMgr::LoadFromFile(const TexPath& path)
{
	// return a ptr to the texture which is loaded from the file by texturePath;
	// 
	// input: path to the texture file;
	// 
	// 1. if such a texture already exists we just return a ptr to it;
	// 2. if there is no texture by such name (path) we try to create it
	
	

	try
	{
		fs::path texPath = path;

		// check if such texture file exist
		if (!fs::exists(texPath))
		{
			Log::Error("a path to texture is empty");
			return INVALID_TEXTURE_ID;
		}

		// if there is such a texture we just return a ptr to it
		if (ArrHasVal(names_, path))
			return GetIDByName(path);

		// else we create a new texture from file
		const TexID id = GenID();

		ids_.push_back(id);
		names_.push_back(path);
		textures_.push_back(std::move(Texture(pDevice_, path)));

		UpdateMapIdToSRV();

		// check if we successfully created this texture
		bool isSuccess = (textures_.back().GetTextureResourceView() != nullptr);
		
		return (isSuccess) ? id : INVALID_TEXTURE_ID;	
	}
	catch (EngineException& e)
	{
		Log::Error(e);
		throw EngineException("can't create a texture from file: " + path);
	}
}

///////////////////////////////////////////////////////////

TexID TextureMgr::CreateWithColor(const Color& color)
{
	// if there is already a texture by such ID we just return a ptr to it;
	// or in another case we create this texture and return a ptr to this new texture;

	// generate a name for this texture
	const std::string name{ "color_texture" +
		std::to_string(color.GetR()) + "_" +
		std::to_string(color.GetG()) + "_" +
		std::to_string(color.GetB()) };

	TexID id = GetIDByName(name);

	return (id != INVALID_TEXTURE_ID) ? id : Add(name, { pDevice_, color });
}



// ************************************************************************************
//
//                                PUBLIC QUERY API
//
// ************************************************************************************

Texture& TextureMgr::GetTexByID(const TexID id)
{
	// return a tex by input id or return an unloaded texture (id: 0)
	return (BinarySearch(ids_, id)) ? textures_[GetIdxInSortedArr(ids_, id)] : textures_[0];
}

///////////////////////////////////////////////////////////

Texture* TextureMgr::GetTexPtrByID(const TexID id)
{
	// return a ptr to the texture by ID or nullptr if there is no such a texture
	return (BinarySearch(ids_, id)) ? &textures_[GetIdxInSortedArr(ids_, id)] : nullptr;
}

///////////////////////////////////////////////////////////

Texture* TextureMgr::GetTexPtrByName(const TexName& name)
{
	// return a ptr to the texture by name or nullptr if there is no such a texture
	const index idx = FindIdxOfVal(names_, name);
	return (idx != names_.size()) ? &textures_[idx] : nullptr;
}

///////////////////////////////////////////////////////////

TexID TextureMgr::GetIDByName(const TexName& name)
{
	// return an ID of texture object by input name;
	// if there is no such a textures we return 0;

	const index idx = CoreUtils::FindIdxOfVal(names_, name);
	return (idx != names_.size()) ? ids_[idx] : INVALID_TEXTURE_ID;
}

///////////////////////////////////////////////////////////

void TextureMgr::GetIDsByNames(
	const std::vector<TexName>& names, 
	std::vector<TexID>& outIDs)
{
	const size namesCount = std::ssize(names_);

	// get data idxs of names
	std::vector<index> idxs;
	CoreUtils::GetIdxsInArr(names_, names, idxs);

	// get IDs
	outIDs.resize(std::ssize(names));
	
	// if idx == namesCount -- there is no such a texture so just set ID == 0
	for (int i = 0; const index idx : idxs)
		outIDs[i++] = (idx != namesCount) ? ids_[idx] : INVALID_TEXTURE_ID;
}

///////////////////////////////////////////////////////////

void TextureMgr::GetSRVByTexID(
	const TexID texID,
	SRV*& outTexSRV)
{
	outTexSRV = (idToSRV_.contains(texID)) ? idToSRV_[texID] : idToSRV_[INVALID_TEXTURE_ID];
}


///////////////////////////////////////////////////////////

void TextureMgr::GetSRVsByTexIDs(
	const TexID* texIDs,
	const int numTex,
	std::vector<SRV*>& outSRVs)
{
	// here get SRV (shader resource view) of each input texture by its ID

	std::vector<int> idxsToNotZero;
	idxsToNotZero.resize(numTex);
	int pos = 0;

	for (int idx = 0; idx < numTex; ++idx)
	{
		idxsToNotZero[pos] = idx;
		pos += bool(texIDs[idx]);
	}

	idxsToNotZero.resize(pos + 1);

	// ---------------------------------------------

	// get SRVs
	ID3D11ShaderResourceView* unloadedTexSRV = idToSRV_[0];
	outSRVs.resize(numTex, unloadedTexSRV);

	for (const int idx : idxsToNotZero)
		outSRVs[idx] = idToSRV_[texIDs[idx]];
}

///////////////////////////////////////////////////////////

void TextureMgr::GetSRVsByTexIDs(
	const std::vector<TexID>& texIDs,
	std::vector<SRV*>& outSRVs)
{
	// here get SRV (shader resource view) of each input texture by its ID

	const size inIDsCount = std::ssize(texIDs);
	std::vector<ptrdiff_t> idxsToNotZero;
	idxsToNotZero.resize(inIDsCount);

	size pos = 0;

	for (index idx = 0; idx < inIDsCount; ++idx)
	{
		idxsToNotZero[pos] = idx;
		pos += bool(texIDs[idx]);
	}

	idxsToNotZero.resize(pos+1);

	// ---------------------------------------------

	// get SRVs
	ID3D11ShaderResourceView* unloadedTexSRV = idToSRV_[0];
	outSRVs.resize(texIDs.size(), unloadedTexSRV);

	for (const index idx : idxsToNotZero)
		outSRVs[idx] = idToSRV_[texIDs[idx]];
}

///////////////////////////////////////////////////////////

void TextureMgr::GetAllTexturesPathsWithinDirectory(
	const std::string& pathToDir,
	std::vector<TexPath>& outPaths)
{
	// get an array of paths to textures in the directory by pathToDir

	namespace fs = std::filesystem;

	std::set<std::string> extentions{ ".dds", ".tga", ".png", ".bmp" };

	// go through each file in the directory
	for (const fs::directory_entry& entry : fs::directory_iterator(pathToDir))
	{
		const fs::path texturePath = entry.path();
		const std::string ext = texturePath.extension().string();

		// if we have some texture image format
		if (extentions.contains(ext))
		{
			std::string path = StringHelper::ToString(texturePath);
			std::replace(path.begin(), path.end(), '\\', '/');  // in the pass change from '\\' into '/' symbol

			outPaths.emplace_back(path);
		}
	}
}




// ***********************************************************************************
// 
//                              PRIVATE HELPERS
// 
// ***********************************************************************************

void TextureMgr::AddDefaultTex(const TexName& name, Texture&& tex)
{
	// add some default texture into the TextureMgr and
	// set for it a specified id

	try
	{
		const bool isUniqueName = !ArrHasVal(names_, name);
		Assert::True(isUniqueName, "there is already a default texture by such name: " + name);

		const TexID id = GenID();

		ids_.push_back(id);
		names_.push_back(name);
		textures_.push_back(std::move(tex));

		UpdateMapIdToSRV();
	}
	catch (EngineException& e)
	{
		Log::Error(e);
		throw EngineException("can't add a texture by name: " + name);
	}
}

///////////////////////////////////////////////////////////

void TextureMgr::UpdateMapIdToSRV()
{
	// update all pairs ['texture_id' => 'ptr_to_srv']
	idToSRV_.clear();

	// set keys 
	for (index idx = 0; idx < std::ssize(ids_); ++idx)
		idToSRV_[ids_[idx]];

	// set values 
	for (index idx = 0; auto& it : idToSRV_)
		it.second = textures_[idx++].GetTextureResourceView();
}