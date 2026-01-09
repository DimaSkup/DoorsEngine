/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: terrain_geomip_creator.cpp

    Created:  11.07.2025  by DimaSkup
\**********************************************************************************/
#include <CoreCommon/pch.h>
#include "terrain_geomip_creator.h"

#include "../Texture/texture_mgr.h"
#include "../Model/model_mgr.h"

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
    const int terrainLength = terrainCfg.terrainLength;  // note that terrain is always a square, so size by X == size by Z (width == depth)
    bool result = false;
   
    // if we want to generate heights
    if (terrainCfg.generateHeights)
    {
        // use "Fault formation" algorithm
        if (terrainCfg.useGenFaultFormation)
        {
            result = terrain.GenHeightFaultFormation(
                terrainLength,
                terrainCfg.numIterations,
                terrainCfg.minDelta,
                terrainCfg.maxDelta,
                terrainCfg.filter);
        }
        // use "midpoint displacement" for height generation
        else
        {
            result = terrain.GenHeightMidpointDisplacement(terrainLength, terrainCfg.roughness);
        }

        if (!result)
            return ErrMsgHelper(LOG, "can't generate heights for terrain");
    }

    // load a height map from the file
    else
    {
        result = terrain.LoadHeightMap(terrainCfg.pathHeightMap, terrainLength);

        if (!result)
            return ErrMsgHelper(LOG, "can't load a height map for terrain: %s", terrainCfg.pathHeightMap);
    }

    if (terrainCfg.saveHeightMap)
        terrain.SaveHeightMap(terrainCfg.pathSaveHeightMap);

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

      
    }
    // load texture map from file
    else
    {
        result = terrain.LoadTextureMap(terrainCfg.texDiffName0);

        if (!result)
            return ErrMsgHelper(LOG, "can't load terrain's texture map: %s", terrainCfg.texDiffName0);
    }

    // save the tile map if we want
    if (terrainCfg.saveTextureMap)
        terrain.SaveTextureMap(terrainCfg.pathSaveTextureMap);

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

    // if we don't wan't to use any lightmaps
    if (!terrainCfg.useLightmap)
        return true;

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
            return ErrMsgHelper(LOG, "can't calculate a terrain's lightmap");
    }

    // load a light map from the file
    else
    {
        result = terrain.LoadLightMap(terrainCfg.texNameLightmap);
        if (!result)
            return ErrMsgHelper(LOG, "failed to init lightmap: %s", terrainCfg.texNameLightmap);
    }

    // save the light map into file if we want
    if (terrainCfg.saveLightMap)
        terrain.SaveLightMap(terrainCfg.pathSaveLightMap);


    // create a lightmap's texture resource
    const TexID lightmapTexId = g_TextureMgr.CreateTextureFromRawData(
        "terrain_light_map",
        terrain.lightmap_.pData,
        terrain.lightmap_.size,
        terrain.lightmap_.size,
        8,                       // bits per pixel
        false);                  // don't generate mipmaps

    terrain.lightmap_.id = lightmapTexId;


    if (lightmapTexId == INVALID_TEX_ID)
    {
        return ErrMsgHelper(LOG, "can't create a texture resource for terrain's lightmap");
    }

    LogDbg(LOG, "terrain_light_map texture is created");
    return true;
}

//---------------------------------------------------------
// Desc:   load a terrain's detail map from file
// Args:   - terrain:   actual terrain's obj
//         - texName:   a name of loaded detail texture
// 
// Ret:    true if we managed to successfully initialize a terrain's detail map
//---------------------------------------------------------
bool TerrainInitDetailMap(TerrainGeomip& terrain, const char* texName)
{
    if (!texName || texName[0] == '\0')
        return ErrMsgHelper(LOG, "empty texture name");


    const TexID texId = g_TextureMgr.GetTexIdByName(texName);
    if (texId == INVALID_TEX_ID)
    {
        LogErr(LOG, "no texture by name: %s", texName);
        return false;
    }

    const Texture& tex = g_TextureMgr.GetTexByID(texId);

    // set params of texture
    terrain.detailMap_.SetID(texId);
    terrain.detailMap_.SetWidth(tex.GetWidth());
    terrain.detailMap_.SetHeight(tex.GetHeight());
    terrain.detailMap_.SetBPP(32);

    LogDbg(LOG, "terrain's detail map metadata is loaded");
    return true;
}

//---------------------------------------------------------
// Desc:   MAIN FUNCTION:
//         create a terrain which will be used together with
//         geomipmapping CLOD algorithm
// 
// Args:   - configFilepath: path to a file with params for terrain
//---------------------------------------------------------
bool TerrainGeomipCreator::Create(const char* configFilepath)
{
    // check input args
    if (StrHelper::IsEmpty(configFilepath))
    {
        LogErr(LOG, "terrain's config filepath is empty");
        exit(-1);
    }


    TerrainGeomip& terrain = g_ModelMgr.GetTerrainGeomip();
    TerrainConfig  terrainCfg;


    if (!terrain.LoadSetupFile(configFilepath, terrainCfg))
    {
        LogErr(LOG, "can't load terrain's configs");
        exit(-1);
    }

    if (!TerrainInitHeight(terrain, terrainCfg))
    {
        LogErr(LOG, "can't init the terrain heights");
    }

    if (!TerrainInitTileMap(terrain, terrainCfg))
    {
        LogErr(LOG, "can't init the terrain's tile map");
    }

    if (!TerrainInitLightMap(terrain, terrainCfg))
    {
        LogErr(LOG, "can't init the terrain's light map");
    }

    if (!TerrainInitDetailMap(terrain, terrainCfg.texNameDetailMap))
    {
        LogErr(LOG, "can't init the terrain's detail map");
    }

    if (!terrain.LoadNatureDensityMap(terrainCfg.pathNatureDensityMap))
    {
        LogErr(LOG, "can't load nature density map for terrain");
    }
   
    if (!terrain.InitGeomipmapping(terrainCfg.patchSize))
    {
        LogErr(LOG, "can't init the geomip terrain");
        exit(-1);
    }


    // setup distances to LODs (for instance: where we switch from LOD0 to LOD1)
    terrain.GetLodMgr().SetDistanceToLOD(0, terrainCfg.distToLod0);
    terrain.GetLodMgr().SetDistanceToLOD(1, terrainCfg.distToLod1);
    terrain.GetLodMgr().SetDistanceToLOD(2, terrainCfg.distToLod2);
    terrain.GetLodMgr().SetDistanceToLOD(3, terrainCfg.distToLod3);

    // compute the bounding box of the terrain
    const int terrainSize           = terrain.GetTerrainLength();
    const DirectX::XMFLOAT3 center  = { 0,0,0 };
    const DirectX::XMFLOAT3 extents = { (float)terrainSize, 1.0f, (float)terrainSize };

    // setup axis-aligned bounding box
    terrain.SetAABB(center, extents);


    LogMsg(LOG, "terrain is created!");
    return true;
}

} // namespace
