///////////////////////////////////////////////////////////////////////////////////////////
// Filename:      ModelInitializerHelper.h
// Description:   contains private helpers for the ModelImporter class;
// 
// Created:       16.02.24
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "../Texture/textureclass.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace Core
{

TexStoreType DetermineTextureStorageType(const aiScene* pScene,
	const aiMaterial* pMaterial,
	const UINT index,
	const aiTextureType textureType)
{
	// this function determines all the possible texture storage types

	if (pMaterial->GetTextureCount(textureType) == 0)
		return TexStoreType::None;

	// get path to the texture
	aiString path;
	pMaterial->GetTexture(textureType, index, &path);

	// ---------------------------------------------

	// check if texture is an embedded indexed texture by seeing if the file path is an index #
	if (path.C_Str()[0] == '*')
	{
		if (pScene->mTextures[0]->mHeight == 0)
		{
			return TexStoreType::EmbeddedIndexCompressed;
		}
		else
		{
			Log::Error("SUPPORT DOES NOT EXIST YET FOR INDEXED NON COMPRESSES TEXTURES");
			return TexStoreType::EmbeddedIndexNonCompressed;
		}
	}

	// ---------------------------------------------

	// check if texture is an embedded texture but not indexed (path will be the texture's name instead of #)
	if (const aiTexture* pTex = pScene->GetEmbeddedTexture(path.C_Str()))
	{
		if (pTex->mHeight == 0)
		{
			return TexStoreType::EmbeddedCompressed;
		}
		else
		{
			Log::Error("SUPPORT DOES NOT EXIST YET FOR EMBEDDED NON COMPRESSES TEXTURES");
			return TexStoreType::EmbeddedNonCompressed;
		}
	}

	// ---------------------------------------------

	// lastly check if texture is a filepath by check for extension value
	
	const std::string strPath(path.C_Str());
	const bool hasExt = (strPath.find_last_of('.') != std::string::npos);

	if (hasExt)
	{
		return TexStoreType::Disk;
	}

	// ---------------------------------------------

	// no texture exists
	return TexStoreType::None;   

} 

///////////////////////////////////////////////////////////

UINT GetIndexOfEmbeddedCompressedTexture(aiString* pStr)
{
	// this function returns an index of the embedded compressed texture by path pStr

	assert(pStr->length >= 2);             // assert that path is "*0", "*1", or something like that
	return (UINT)atoi(&pStr->C_Str()[1]);  // return an index
}

} // namespace Core