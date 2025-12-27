///////////////////////////////////////////////////////////////////////////////////////////
// Filename:      model_importer_helpers.h
// Description:   contains private helpers for the ModelImporter class;
// 
// Created:       16.02.24
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "../Texture/texture.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>


namespace Core
{

//---------------------------------------------------------
// determine the type of texture storage by input material and texture type
//---------------------------------------------------------
TexStoreType DetermineTexStoreType(
    const aiScene* pScene,
    const aiMaterial* pMaterial,
    const UINT index,
    const aiTextureType textureType)
{
    if (pMaterial->GetTextureCount(textureType) == 0)
        return TexStoreType::None;

    // get path to the texture
    aiString path;
    pMaterial->GetTexture(textureType, index, &path);

    // ---------------------------------------------

    // check if texture is an embedded indexed texture by seeing if the file path is an index #
    if (path.C_Str()[0] == '*')
    {
        if (pScene->mTextures[0]->mHeight != 0)
        {
            LogErr(LOG, "SUPPORT DOES NOT EXIST YET FOR INDEXED NON COMPRESSES TEXTURES");
            return TexStoreType::EmbeddedIndexNonCompressed;
        }

        return TexStoreType::EmbeddedIndexCompressed;
    }

    // ---------------------------------------------

    // check if texture is an embedded texture but not indexed (path will be the texture's name instead of #)
    if (const aiTexture* pTex = pScene->GetEmbeddedTexture(path.C_Str()))
    {
        if (pTex->mHeight != 0)
        {
            LogErr(LOG, "SUPPORT DOES NOT EXIST YET FOR EMBEDDED NON COMPRESSES TEXTURES");
            return TexStoreType::EmbeddedNonCompressed;
        }

        return TexStoreType::EmbeddedCompressed;
        
    }

    // ---------------------------------------------

    // lastly check if texture is stored on the disk
    // (just check for '.' before the extension)
    if (strchr(path.C_Str(), '.') != NULL)
    {
        return TexStoreType::Disk;
    }

    // ---------------------------------------------

    // no texture exists
    return TexStoreType::None;   
} 

//---------------------------------------------------------
// return an index of the embedded compressed texture by path pStr
//---------------------------------------------------------
UINT GetIndexOfEmbeddedCompressedTexture(aiString* pStr)
{
    // assert that path is "*0", "*1", or something like that
    if (!(pStr->length >= 2))
        return 0;

    // return an index
    return (UINT)atoi(&pStr->C_Str()[1]); 
}

} // namespace
