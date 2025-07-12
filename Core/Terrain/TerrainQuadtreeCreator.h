//==================================================================================
// Filename:   TerrainQuadtreeCreator.h
// Desc:       functional for creation and setup of a quadtree terrain
//
// Created:    11.07.2025  by DimaSkup
//==================================================================================
#pragma once

#include "../Model/ModelMgr.h"
#include "TerrainQuadtree.h"
#include <log.h>


namespace Core
{

// prototypes
bool TerrainInitHeight  (TerrainQuadtree& terrain, const TerrainConfig& terrainCfg);
bool TerrainInitTileMap (TerrainQuadtree& terrain, const TerrainConfig& terrainCfg);
bool TerrainInitLightMap(TerrainQuadtree& terrain, const TerrainConfig& terrainCfg);
bool TerrainInitHeight  (TerrainQuadtree& terrain, const TerrainConfig& terrainCfg);


//---------------------------------------------------------
// Desc:   MAIN FUNCTION:
//         create a terrain which will be used together with the quadtree algorithm
// Args:   - pDevice:        ptr to DirectX11 device
//         - configFilename: path to a file with params for terrain
//---------------------------------------------------------
bool CreateTerrainQuadtree(ID3D11Device* pDevice, const char* configFilename)
{
    return true;

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

    TerrainQuadtree&  terrain = g_ModelMgr.GetTerrainQuadtree();
    GeometryGenerator geoGen;
    TerrainConfig     terrainCfg;

    // load terrain configs
    terrain.LoadSetupFile(configFilename, terrainCfg);

    const int width = terrainCfg.width;
    const int depth = terrainCfg.depth;

    // generate terrain's flag grid
    int numVertices = 0;
    int numIndices = 0;

    terrain.Release();

    geoGen.GenerateTerrainFlatGrid(
        width,
        depth,
        width + 1,
        depth + 1,
        terrain.GetVerticesAddrOf(),
        terrain.GetIndicesAddrOf(),
        numVertices,
        numIndices);

    terrain.SetNumVertices(numVertices);
    terrain.SetNumIndices(numIndices);

    // ------------------------------------------

    // generate a height map or load it from file
    if (!TerrainInitHeight(terrain, terrainCfg))
    {
        LogErr(LOG, "can't inialize the terrain heights");
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
        LogErr(LOG, "can't initialize the terrain's lightmap");
        exit(-1);
    }

    if (!terrain.LoadDetailMap(terrainCfg.pathDetailMap))
    {
        LogErr(LOG, "can't load the terrain's detail map");
        exit(-1);
    }


    // TEMP
    terrain.Release();

    // great success!
    LogMsg(LOG, "terrain (quadtree type) is created!");
    return true;
}

//---------------------------------------------------------
// Desc:   generate terrain's height map or load it from file
// Args:   - terrain:     actual terrain's obj
//         - terrainCfg:  container for different configs for terrain
//---------------------------------------------------------
bool TerrainInitHeight(TerrainQuadtree& terrain, const TerrainConfig& terrainCfg)
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

        // use "midpoint displacement"
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
    if (result)
    {
        Vertex3dTerrain* vertices = terrain.GetVertices();

        for (int i = 0; i < terrain.GetNumVertices(); ++i)
        {
            DirectX::XMFLOAT3& pos = vertices[i].position;
            pos.y = terrain.GetScaledHeightAtPoint((int)pos.x, (int)pos.z);
        }
    }

    return result;
}


//---------------------------------------------------------
// Desc:   generate terrain's tile map or load it from file
// Args:   - terrain:    actual terrain's obj
//         - terrainCfg: container for different configs for terrain
//---------------------------------------------------------
bool TerrainInitTileMap(TerrainQuadtree& terrain, const TerrainConfig& terrainCfg)
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

        const TexID tileMapTexID = g_TextureMgr.CreateTextureFromRawData(
            "terrain_tile_map",
            terrain.texture_.GetData(),
            terrain.texture_.GetWidth(),
            terrain.texture_.GetHeight(),
            terrain.texture_.GetBPP(),
            mipMapped);

        if (tileMapTexID != INVALID_TEXTURE_ID)
        {
            LogDbg(LOG, "terrain_tile_map texture is created");
            terrain.texture_.SetID(tileMapTexID);
        }
        else
        {
            LogErr("can't create a texture resource for terrain's tile map");
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

    return true;
}

//---------------------------------------------------------
// Desc:   generate lightmap or load it from file
// Args:   - terrain:   actual terrain's obj
//---------------------------------------------------------
bool TerrainInitLightMap(TerrainQuadtree& terrain, const TerrainConfig& terrainCfg)
{
    return true;
}


} // namespace Core
