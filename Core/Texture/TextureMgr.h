////////////////////////////////////////////////////////////////////////////////////////////////
// Filename:      TextureMgr.h
// Description:   a manager for work with textures: 
//                when we ask for the texture for the first time we initialize it from 
//                the file and store it in the manager so later it'll be faster to just copy it
//                but not to read it from the file anew.
//                 
// Created:       06.06.23
////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "textureclass.h"
#include "TextureHelperTypes.h"

#include <d3d11.h>
#include <d3dx11tex.h>
#include <windows.h>
#include <map>
#include <vector>
#include <string>


class TextureMgr
{
public:
	static const TexID TEX_ID_UNLOADED = 0;
	static const TexID TEX_ID_UNHANDLED = 1;

public:
	TextureMgr();
	~TextureMgr();

	// restrict shallow copying
	TextureMgr(const TextureMgr&) = delete;
	TextureMgr& operator=(const TextureMgr&) = delete;

	inline static TextureMgr* Get() { return pInstance_; }

	// public creation API
	void Initialize(ID3D11Device* pDevice);

	TexID Add(const TexName& name, Texture& tex);
	TexID Add(const TexName& name, Texture&& tex);
	

	TexID LoadFromFile(const TexPath& path);

	// TODO: FIX IT
	//void LoadFromFile(const std::vector<TexPath>& texPaths, std::vector<TexID>& outTexIDs);

	TexID CreateWithColor(const Color& textureColor);


	// public query API
	Texture& GetTexByID(const TexID id);
	Texture* GetTexPtrByID(const TexID id);
	Texture* GetTexPtrByName(const TexName& name);

	TexID GetIDByName(const TexName& name);
	void GetIDsByNames(const std::vector<TexName>& names, std::vector<TexID>& outIDs);

	void GetSRVByTexID(const TexID texID, SRV*& outSRVs);
	void GetSRVsByTexIDs(const TexID* texIDs, const int numTex, std::vector<SRV*>& outSRVs);
	void GetSRVsByTexIDs(const std::vector<TexID>& texIDs, std::vector<SRV*>& outSRVs);

	void GetAllTexturesPathsWithinDirectory(
		const std::string& pathToDir,
		std::vector<TexPath>& outPathsToTextures);

private:
	void AddDefaultTex(const TexName& name, Texture&& tex);
	void UpdateMapIdToSRV();

	inline int GenID() { return lastTexID_++; }


private:
	const u32 INVALID_TEXTURE_ID = 0;

	static int lastTexID_;
	static TextureMgr* pInstance_;
	ID3D11Device* pDevice_ = nullptr;

	std::map<TexID, SRV*> idToSRV_;

	std::vector<TexID> ids_;             // SORTED array of unique IDs
	std::vector<TexName> names_;         // name (there can be path) which is used for searching of texture
	std::vector<Texture> textures_;
};