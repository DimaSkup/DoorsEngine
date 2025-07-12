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
// Desc:   generate terrain's height map or load it from file
// Args:   - terrain:    actual terrain's obj
//         - terrainCfg: container for different configs for terrain
//---------------------------------------------------------
bool TerrainInitHeight(TerrainGeomipmapped& terrain, const TerrainConfig& terrainCfg)
{
    const int terrainWidth = terrainCfg.width;  // note that terrain is always a square
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
            result = terrain.GenHeightMidpointDisplacement(
                terrainWidth,
                terrainCfg.roughness);
        }

        if (terrainCfg.saveHeightMap)
            terrain.SaveHeightMap(terrainCfg.pathSaveHeightMap);
    }

    // load a height map from the file
    else
    {
        result = terrain.LoadHeightMap(terrainCfg.pathHeightMap, terrainWidth);

        if (terrainCfg.saveHeightMap)
            terrain.SaveHeightMap(terrainCfg.pathSaveHeightMap);
    }


    // set heights for the terrain at particular positions
#if 1
    if (result)
    {
        for (int i = 0; i < terrain.GetNumVertices(); ++i)
        {
            DirectX::XMFLOAT3& pos = terrain.vertices_[i].position;
            pos.y = terrain.GetScaledHeightAtPoint((int)pos.x, (int)pos.z);
        }
    }
#endif

    return result;
}

//---------------------------------------------------------
// Desc:   generate terrain's tile map or load it from file
// Args:   - terrain:    actual terrain's obj
//         - terrainCfg: container for different configs for terrain
//---------------------------------------------------------
bool TerrainInitTileMap(TerrainGeomipmapped& terrain, const TerrainConfig& terrainCfg)
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
        {
            LogErr(LOG, "can't generate a texture map for terrain");
            return false;
        }

        
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
            LogErr("can't create texture resource for terrain's tile map");
            result = false;
        }

        // save the tile map if we want
        if (terrainCfg.saveTextureMap)
            terrain.SaveTextureMap(terrainCfg.pathSaveTextureMap);
    }
    // load texture map from file
    else
    {
        result = terrain.LoadTextureMap(terrainCfg.pathTextureMap);

        if (terrainCfg.saveTextureMap)
            terrain.SaveTextureMap(terrainCfg.pathSaveTextureMap);
    }

    return result;
}

//---------------------------------------------------------
// Desc:   generate lightmap or load it from file
// Args:   - terrain:    actual terrain's obj
//         - terrainCfg: container for different configs for terrain
//---------------------------------------------------------
bool TerrainInitLightMap(TerrainGeomipmapped& terrain, const TerrainConfig& terrainCfg)
{
    bool result = false;

    if (terrainCfg.generateLightMap)
    {
        // setup the terrain's lighting system
        terrain.SetLightingType(terrainCfg.lightingType);
        terrain.SetLightColor(terrainCfg.lightColor);

        terrain.CustomizeSlopeLighting(
            terrainCfg.lightDirX,
            terrainCfg.lightDirZ,
            terrainCfg.lightMinBrightness,
            terrainCfg.lightMaxBrightness,
            terrainCfg.shadowSoftness);

        // compute pixel raw data for the lightmap texture
        result = terrain.CalculateLighting();

        if (terrainCfg.saveLightMap)
            terrain.SaveLightMap(terrainCfg.pathSaveLightMap);
    }
    else
    {
        result = terrain.LoadLightMap(terrainCfg.pathLightMap);

        if (terrainCfg.saveLightMap)
            terrain.SaveLightMap(terrainCfg.pathSaveLightMap);
    }

    // if we successfully loaded/generated lightmap's raw data
    if (result)
    {
        LightmapData& lightmap = terrain.lightmap_;

        // create texture resource
        const TexID lightmapTexId = g_TextureMgr.CreateTextureFromRawData(
            "terrain_light_map",
            lightmap.pData,
            lightmap.size,
            lightmap.size,
            8,                 // bits per pixel
            false);

        if (lightmapTexId)
        {
            LogDbg(LOG, "terrain_light_map texture is created");
            terrain.lightmap_.id = lightmapTexId;
        }
        else
        {
            LogErr("can't create texture resource for terrain's lightmap");
            result = false;
        }
    }

    return result;
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
    {
        LogErr("input ptr to DirectX device == nullptr");
        return false;
    }

    if (!configFilename || configFilename[0] == '\0')
    {
        LogErr("intput path to terrain config file is empty!");
        return false;
    }

    TerrainGeomipmapped& terrain = g_ModelMgr.GetTerrainGeomip();
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

    // generate a height map or load it from file
    if (!TerrainInitHeight(terrain, terrainCfg))
    {
        LogErr(LOG, "can't initialize the terrain heights");
        exit(-1);
    }

    // generate a tile map or load it from file
    if (!TerrainInitTileMap(terrain, terrainCfg))
    {
        LogErr(LOG, "can't initialize the terrain's tile map");
        exit(-1);
    }

    // generate a light map or load it from file
    if (!TerrainInitLightMap(terrain, terrainCfg))
    {
        LogErr(LOG, "can't initialize the terrain's light map");
        exit(-1);
    }

    if (!terrain.LoadDetailMap(terrainCfg.pathDetailMap))
    {
        LogErr(LOG, "can't load the terrain's detail map");
        exit(-1);
    }


    // ------------------------------------------
    // Create DirectX textures for the terrain
    //
    constexpr bool mipMapped = true;

    Image& detailMap = terrain.detailMap_;

    const TexID detailMapTexId = g_TextureMgr.CreateTextureFromRawData(
        "terrain_detail_map",
        detailMap.GetData(),
        detailMap.GetWidth(),
        detailMap.GetHeight(),
        detailMap.GetBPP(),
        mipMapped);

    if (detailMapTexId)
        LogDbg(LOG, "terrain_detail_map texture is created");

    // store an ID of created detail texture
    detailMap.SetID(detailMapTexId);


    terrain.ClearMemory();
    terrain.AllocateMemory(1000000, 1);
    UINT indices[1]{ 0 };

    // initialize vertex/index buffers
    terrain.InitBuffers(
        pDevice,
        terrain.vertices_,
        indices, //terrain.indices_,
        terrain.numVertices_,
        1);      // terrain.numIndices_);

    terrain.InitGeomipmapping(terrainCfg.patchSize);

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
