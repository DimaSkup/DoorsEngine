//==================================================================================
// Filename:   TerrainGeomipmapCreator.h
// Desc:       functional for creation and setup of a geomipmapped terrain
//
// Created:    11.07.2025  by DimaSkup
//==================================================================================
#pragma once

#include "../Texture/TextureMgr.h"
#include "../Model/ModelMgr.h"

#include "TerrainBase.h"
#include "TerrainGeomipmapped.h"


namespace Core
{

//---------------------------------------------------------
// Desc:   a little helper for code clarity
// Args:   - msg:  error text content
// Ret:    always false because it is for error messages
//---------------------------------------------------------
inline bool ErrMsgHelper(
    const char* fullFilePath,
    const char* funcName,
    const int codeLine,
    const char* format,
    const char* text = "")
{
    LogErr(fullFilePath, funcName, codeLine, format, text);
    return false;
}

//---------------------------------------------------------
// Desc:   generate terrain's height map or load it from file
// Args:   - terrain:    actual terrain's obj
//         - terrainCfg: container for different configs for terrain
// Ret:    true if we managed to successfully initialize a terrain's height map
//---------------------------------------------------------
bool TerrainInitHeight(TerrainGeomip& terrain, const TerrainConfig& terrainCfg)
{
    const int terrainWidth = terrainCfg.width;  // note that terrain is always a square, so size by X == size by Z (width == depth)
    bool result = false;
   
    // if we want to generate heights
    if (terrainCfg.generateHeights)
    {
        // use "Fault formation" algorithm
        if (terrainCfg.useGenFaultFormation)
        {
            result = terrain.GenHeightFaultFormation(
                terrainWidth,
                terrainCfg.numIterations,
                terrainCfg.minDelta,
                terrainCfg.maxDelta,
                terrainCfg.filter);
        }
        // use "midpoint displacement" for height generation
        else
        {
            result = terrain.GenHeightMidpointDisplacement(terrainWidth, terrainCfg.roughness);
        }

        if (!result)
            return ErrMsgHelper(LOG, "can't generate heights for terrain");

        if (terrainCfg.saveHeightMap)
            terrain.SaveHeightMap(terrainCfg.pathSaveHeightMap);
    }

    // load a height map from the file
    else
    {
        result = terrain.LoadHeightMap(terrainCfg.pathHeightMap, terrainWidth);

        if (!result)
            return ErrMsgHelper(LOG, "can't load a height map for terrain: %s", terrainCfg.pathHeightMap);

        if (terrainCfg.saveHeightMap)
            terrain.SaveHeightMap(terrainCfg.pathSaveHeightMap);
    }

    // set heights for the terrain at particular positions
    if (result)
    {
        for (int i = 0; i < terrain.GetNumVertices(); ++i)
        {
            DirectX::XMFLOAT3& pos = terrain.vertices_[i].position;
            pos.y = terrain.GetScaledHeightAtPoint((int)pos.x, (int)pos.z);
        }
    }

    return result;
}

//---------------------------------------------------------
// Desc:   generate terrain's tile map or load it from file
// Args:   - terrain:    actual terrain's obj
//         - terrainCfg: container for different configs for terrain
// Ret:    true if we managed to successfully initialize a terrain's tile map
//---------------------------------------------------------
bool TerrainInitTileMap(TerrainGeomip& terrain, const TerrainConfig& terrainCfg)
{
    bool result = false;

    // if we want to generate a new texture map
    if (terrainCfg.generateTextureMap)
    {
        // load tiles which will be used to generate texture map for terrain
        terrain.LoadTile(LOWEST_TILE,  terrainCfg.pathLowestTile);
        terrain.LoadTile(LOW_TILE,     terrainCfg.pathLowTile);
        terrain.LoadTile(HIGH_TILE,    terrainCfg.pathHighTile);
        terrain.LoadTile(HIGHEST_TILE, terrainCfg.pathHighestTile);

        // generate texture map based on loaded tiles and height map
        result = terrain.GenerateTextureMap(terrainCfg.textureMapSize);

        if (!result)
            return ErrMsgHelper(LOG, "can't generate a texture map for terrain");


        // create a texture resource
        constexpr bool mipMapped = true;

        const TexID tileMapTexId = g_TextureMgr.CreateTextureFromRawData(
            "terrain_tile_map",
            terrain.texture_.GetData(),
            terrain.texture_.GetWidth(),
            terrain.texture_.GetHeight(),
            terrain.texture_.GetBPP(),
            mipMapped);

        if (tileMapTexId)
        {
            LogDbg(LOG, "terrain_tile_map texture is created");
            terrain.texture_.SetID(tileMapTexId);
        }
        else
        {
            return ErrMsgHelper(LOG, "can't create texture resource for terrain's tile map");
        }

        // save the tile map if we want
        if (terrainCfg.saveTextureMap)
            terrain.SaveTextureMap(terrainCfg.pathSaveTextureMap);
    }
    // load texture map from file
    else
    {
        result = terrain.LoadTextureMap(terrainCfg.pathTextureMap);

        if (!result)
            return ErrMsgHelper(LOG, "can't load terrain's texture map: %s", terrainCfg.pathTextureMap);

        if (terrainCfg.saveTextureMap)
            terrain.SaveTextureMap(terrainCfg.pathSaveTextureMap);
    }

    return true;
}

//---------------------------------------------------------
// Desc:   generate lightmap or load it from file
// Args:   - terrain:    actual terrain's obj
//         - terrainCfg: container for different configs for terrain
// Ret:    true if we managed to successfully initialize a terrain's lightmap
//---------------------------------------------------------
bool TerrainInitLightMap(TerrainGeomip& terrain, const TerrainConfig& terrainCfg)
{
    bool result = false;

    // if we want to generate a new light map
    if (terrainCfg.generateLightMap)
    {
        // setup the terrain's lighting
        terrain.SetLightingType(terrainCfg.lightingType);

        terrain.CustomizeSlopeLighting(
            terrainCfg.lightDirX,
            terrainCfg.lightDirZ,
            terrainCfg.lightMinBrightness,
            terrainCfg.lightMaxBrightness,
            terrainCfg.shadowSoftness);

        // compute pixel raw data for the lightmap texture
        result = terrain.CalculateLighting();

        if (!result)
            return ErrMsgHelper(LOG, "can't calculate terrain's lightmap");

        if (terrainCfg.saveLightMap)
            terrain.SaveLightMap(terrainCfg.pathSaveLightMap);
    }

    // load a light map from the file
    else
    {
        result = terrain.LoadLightMap(terrainCfg.pathLightMap);

        if (!result)
            return ErrMsgHelper(LOG, "can't load terrain's light map from file: %s", terrainCfg.pathLightMap);

        if (terrainCfg.saveLightMap)
            terrain.SaveLightMap(terrainCfg.pathSaveLightMap);
    }

    // create a lightmap's texture resource
    const TexID lightmapTexId = g_TextureMgr.CreateTextureFromRawData(
        "terrain_light_map",
        terrain.lightmap_.pData,
        terrain.lightmap_.size,
        terrain.lightmap_.size,
        8,                       // bits per pixel
        false);                  // don't generate mipmaps

    if (lightmapTexId)
    {
        LogDbg(LOG, "terrain_light_map texture is created");
        terrain.lightmap_.id = lightmapTexId;
    }
    else
    {
        return ErrMsgHelper(LOG, "can't create a texture resource for terrain's lightmap");
    }
    

    return true;
}

//---------------------------------------------------------
// Desc:   load a terrain's detail map from file
// Args:   - terrain:    actual terrain's obj
//         - filePath:   a path to texture
// Ret:    true if we managed to successfully initialize a terrain's detail map
//---------------------------------------------------------
bool TerrainInitDetailMap(TerrainGeomip& terrain, const char* filePath)
{
    if (StrHelper::IsEmpty(filePath))
        return ErrMsgHelper(LOG, "input path to terrain's detail map is empty");

    if (!terrain.LoadDetailMap(filePath))
        return ErrMsgHelper(LOG, "can't load the terrain's detail map");


    // create a DirectX11 texture resource for the terrain's detail map
    constexpr bool mipMapped = true;

    const TexID detailMapTexId = g_TextureMgr.CreateTextureFromRawData(
        "terrain_detail_map",
        terrain.detailMap_.GetData(),
        terrain.detailMap_.GetWidth(),
        terrain.detailMap_.GetHeight(),
        terrain.detailMap_.GetBPP(),
        mipMapped);

    if (detailMapTexId != INVALID_TEXTURE_ID)
    {
        terrain.detailMap_.SetID(detailMapTexId);
        LogDbg(LOG, "terrain_detail_map texture is created");
    }
    else
    {
        return ErrMsgHelper(LOG, "can't create a texture resource for terrain's detail map");
    }

    return true;
}

//---------------------------------------------------------
// Desc:   MAIN FUNCTION:
//         create a terrain which will be used together with
//         geomipmapping CLOD algorithm
// Args:   - pDevice:        ptr to DirectX11 device
//         - configFilename: path to a file with params for terrain
//---------------------------------------------------------
bool CreateTerrainGeomipmapped(ID3D11Device* pDevice, const char* configFilename)
{
    // check input args
    if (!pDevice)
        return ErrMsgHelper(LOG, "input ptr to DirectX device == nullptr");

    if (StrHelper::IsEmpty(configFilename))
        return ErrMsgHelper(LOG, "intput path to terrain config file is empty!");


    TerrainGeomip& terrain = g_ModelMgr.GetTerrainGeomip();
    GeometryGenerator    geoGen;
    TerrainConfig        terrainCfg;

    // load terrain configs
    terrain.LoadSetupFile(configFilename, terrainCfg);

    const int width = terrainCfg.width;
    const int depth = terrainCfg.depth;

    // generate terrain flat grid's vertices and indices by input params
    int numVertices = 0;
    int numIndices = 0;

    terrain.ClearMemory();

    geoGen.GenerateTerrainFlatGrid(
        width-1,                     // num quads by X
        depth-1,                     // num quads by Z
        width,                       // num vertices by X
        depth,                       // num vertices by Z
        &terrain.vertices_,
        &terrain.indices_,
        numVertices,
        numIndices);

    terrain.numVertices_ = (uint32)numVertices;
    terrain.numIndices_  = (uint32)numIndices;

    // ------------------------------------------

    if (!TerrainInitHeight(terrain, terrainCfg))
    {
        LogErr(LOG, "can't initialize the terrain heights");
        exit(-1);
    }

    if (!TerrainInitTileMap(terrain, terrainCfg))
    {
        LogErr(LOG, "can't initialize the terrain's tile map");
        exit(-1);
    }

    if (!TerrainInitLightMap(terrain, terrainCfg))
    {
        LogErr(LOG, "can't initialize the terrain's light map");
        exit(-1);
    }

    if (!TerrainInitDetailMap(terrain, terrainCfg.pathDetailMap))
    {
        LogErr(LOG, "can't initialize the terrain's detail map");
        exit(-1);
    }
   
    // ------------------------------------------

#if 0
    terrain.ClearMemory();
    terrain.AllocateMemory(1000000, 1);
    UINT indices[1]{ 0 };


    // initialize vertex/index buffers
    terrain.InitBuffers(
        pDevice,
        terrain.vertices_,
        indices, //terrain.indices_,
        1000000, //terrain.numVertices_,
        1);      // terrain.numIndices_);
#else


    for (int i = 0; i < (int)terrain.numIndices_; ++i)
        terrain.indices_[i] = i;

#endif

    if (!terrain.InitGeomipmapping(terrainCfg.patchSize))
    {
        LogErr(LOG, "can't initialize the geomip terrain");
        exit(-1);
    }

    // compute the bounding box of the terrain
    const DirectX::XMFLOAT3 center = { 0,0,0 };
    const DirectX::XMFLOAT3 extents = { (float)width, 1.0f, (float)depth };

    // setup axis-aligned bounding box
    terrain.SetAABB(center, extents);

    // release CPU copy of transient data since we're already loaded
    // all the necessary stuff on GPU
    //terrain.ClearMemoryFromMaps();

    LogMsg(LOG, "terrain (geomipmap type) is created!");
    return true;
}

} // namespace Core
