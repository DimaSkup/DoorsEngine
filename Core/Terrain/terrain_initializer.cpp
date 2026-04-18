/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: terrain_initializer.cpp

    Created:  11.07.2025  by DimaSkup
\**********************************************************************************/
#include <CoreCommon/pch.h>
#include "terrain_initializer.h"

#include "../Texture/texture_mgr.h"
#include "../Model/model_mgr.h"

#include "Terrain.h"


namespace Core
{

//---------------------------------------------------------
// prototypes of private internal helpers
//---------------------------------------------------------
bool TerrainInitHeight(Terrain& terrain, const TerrainConfig& terrainCfg);
bool TerrainInitTileMap(Terrain& terrain, const TerrainConfig& terrainCfg);
bool TerrainInitLightMap(Terrain& terrain, const TerrainConfig& terrainCfg);
bool TerrainInitDetailMap(Terrain& terrain, const char* texName);


//---------------------------------------------------------
// Desc:   MAIN FUNCTION:
//         create a terrain which will be used together with
//         geomipmapping CLOD algorithm
// 
// Args:   - cfgFilepath: path to a file with params for terrain
//---------------------------------------------------------
bool TerrainInitializer::Init(const char* cfgFilepath)
{
    // check input args
    if (StrHelper::IsEmpty(cfgFilepath))
    {
        LogFatal(LOG, "terrain's config filepath is empty");
    }


    Terrain& terrain = g_ModelMgr.GetTerrain();
    TerrainConfig terrainCfg;

    if (!terrain.LoadSetupFile(cfgFilepath, terrainCfg))
    {
        LogFatal(LOG, "can't load terrain's configs");
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
        LogFatal(LOG, "can't init the geomip terrain");
    }


    // setup distances to LODs (for instance: where we switch from LOD0 to LOD1)
    terrain.GetLodMgr().SetDistanceToLOD(0, terrainCfg.distToLod0);
    terrain.GetLodMgr().SetDistanceToLOD(1, terrainCfg.distToLod1);
    terrain.GetLodMgr().SetDistanceToLOD(2, terrainCfg.distToLod2);
    terrain.GetLodMgr().SetDistanceToLOD(3, terrainCfg.distToLod3);

    // compute terrain's axis-aligned bounding box
    terrain.CalcAABB();


    LogMsg(LOG, "terrain is created!");
    return true;
}


//==================================================================================
// PRIVATE INTERNAL HELPERS
//==================================================================================

//---------------------------------------------------------
// Desc:   generate terrain's height map or load it from file
// Args:   - terrain:    actual terrain's obj
//         - terrainCfg: container for different configs for terrain
// Ret:    true if we managed to successfully initialize a terrain's height map
//---------------------------------------------------------
bool TerrainInitHeight(Terrain& terrain, const TerrainConfig& terrainCfg)
{
    bool result = true;
   
    // if we want to generate heights
    if (terrainCfg.generateHeights)
    {
        // use "Fault formation" algorithm
        if (terrainCfg.useGenFaultFormation)
        {
            result = terrain.GenHeightFaultFormation(
                terrainCfg.terrainLength,
                terrainCfg.numIterations,
                terrainCfg.minDelta,
                terrainCfg.maxDelta,
                terrainCfg.filter);
        }
        // use "midpoint displacement" for height generation
        else
        {
            result = terrain.GenHeightMidpointDisplacement(terrainCfg.terrainLength, terrainCfg.roughness);
        }

        if (!result)
        {
            LogErr(LOG, "can't generate heights for terrain");
            return false;
        }
    }

    // load a height map from the file
    else
    {
        if (!terrain.LoadHeightMap(terrainCfg.pathHeightMap))
        {
            LogErr(LOG, "can't load a height map for terrain: %s", terrainCfg.pathHeightMap);
            return false;
        }

        if (terrain.heightMap_.GetWidth() != terrainCfg.terrainLength)
        {
            LogErr(LOG, "dimensions of height map must == terrain's length (but %u != %d)", terrain.heightMap_.GetWidth(), terrainCfg.terrainLength);
            return false;
        }
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
bool TerrainInitTileMap(Terrain& terrain, const TerrainConfig& terrainCfg)
{
    // if we want to generate a new texture map
    if (terrainCfg.generateTextureMap)
    {
        // load tiles which will be used to generate texture map for terrain
        terrain.LoadTile(LOWEST_TILE,  terrainCfg.pathLowestTile);
        terrain.LoadTile(LOW_TILE,     terrainCfg.pathLowTile);
        terrain.LoadTile(HIGH_TILE,    terrainCfg.pathHighTile);
        terrain.LoadTile(HIGHEST_TILE, terrainCfg.pathHighestTile);

        // generate texture map based on loaded tiles and height map
        if (!terrain.GenerateTextureMap(terrainCfg.textureMapSize))
        {
            LogErr(LOG, "can't generate a texture map for terrain");
            return false;
        }

        // create a texture resource
        constexpr bool mipMapped = true;

        const TexID tileMapTexId = g_TextureMgr.CreateTextureFromRawData(
            "terrain_tile_map",
            terrain.texture_.GetPixels(),
            terrain.texture_.GetWidth(),
            terrain.texture_.GetHeight(),
            terrain.texture_.GetBPP(),
            mipMapped);

        if (tileMapTexId)
        {
            LogDbg(LOG, "terrain_tile_map texture is created");
            terrain.texture_.SetId(tileMapTexId);
        }
        else
        {
            LogErr(LOG, "can't create texture resource for terrain's tile map");
            return false;
        }
    }

    // load texture map from file
    else
    {
        if (!terrain.LoadTextureMap(terrainCfg.texDiffName0))
        {
            LogErr(LOG, "can't load terrain's texture map: %s", terrainCfg.texDiffName0);
            return false;
        }
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
bool TerrainInitLightMap(Terrain& terrain, const TerrainConfig& terrainCfg)
{
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
        if (!terrain.CalculateLighting())
        {
            LogErr(LOG, "can't calculate a terrain's lightmap");
            return false;
        }
    }

    // load a light map from the file
    else
    {
        if (!terrain.LoadLightMap(terrainCfg.texNameLightmap))
        {
            LogErr(LOG, "failed to init lightmap: %s", terrainCfg.texNameLightmap);
            return false;
        }
    }

    // save the light map into file if we want
    if (terrainCfg.saveLightMap)
        terrain.SaveLightMap(terrainCfg.pathSaveLightMap);


    // create a lightmap's texture resource
    const int  bitsPerPixel = 8;
    const bool bGenerateMips = false;

    terrain.lightmap_.id = g_TextureMgr.CreateTextureFromRawData(
        "terrain_light_map",
        terrain.lightmap_.pData,
        terrain.lightmap_.size,
        terrain.lightmap_.size,
        bitsPerPixel,
        bGenerateMips);

    if (terrain.lightmap_.id == INVALID_TEX_ID)
    {
        LogErr(LOG, "can't create a texture resource for terrain's lightmap");
        return false;
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
bool TerrainInitDetailMap(Terrain& terrain, const char* texName)
{
    if (StrHelper::IsEmpty(texName))
    {
        LogErr(LOG, "empty texture name");
        return false;
    }

    const TexID texId = g_TextureMgr.GetTexIdByName(texName);
    if (texId == INVALID_TEX_ID)
    {
        LogErr(LOG, "no texture by name: %s", texName);
        return false;
    }

    const Texture& tex = g_TextureMgr.GetTexById(texId);

    // set params of texture
    terrain.detailMap_.SetId(texId);
    terrain.detailMap_.SetWidth(tex.GetWidth());
    terrain.detailMap_.SetHeight(tex.GetHeight());
    terrain.detailMap_.SetBPP(32);

    LogDbg(LOG, "terrain's detail map metadata is loaded");
    return true;
}

//---------------------------------------------------------
// Desc:   compute averaged normals to terrain's faces
//---------------------------------------------------------
void ComputeAveragedNormals(
    Vertex3dTerrain* vertices,
    const UINT* indices,
    const int numVertices,
    const int numIndices)
{
    // compute normal-vectors for input vertices with vertex normal averaging
    //                  n0 + n1 + n2 + n3
    //        Navg = -----------------------
    //               || n0 + n1 + n2 + n3 ||

    using namespace DirectX;

    // check input params
    if (!vertices)
    {
        LogErr(LOG, "input vertices == nullptr");
        return;
    }
    if (!indices)
    {
        LogErr(LOG, "input indices == nullptr");
        return;
    }

    if ((numVertices <= 0) || (numIndices <= 0))
    {
        LogErr(LOG, "input number of vertices/indices must be > 0");
        return;
    }

    // for each triangle
    for (int i = 0; i < numIndices / 3; ++i)
    {
        // indices of the ith triangle 
        int baseIdx = i * 3;
        const UINT i0 = indices[baseIdx + 0];
        const UINT i1 = indices[baseIdx + 1];
        const UINT i2 = indices[baseIdx + 2];

        // positions of vertices of ith triangle stored as XMVECTOR
        const XMVECTOR v0 = XMLoadFloat3(&vertices[i0].position);
        const XMVECTOR v1 = XMLoadFloat3(&vertices[i1].position);
        const XMVECTOR v2 = XMLoadFloat3(&vertices[i2].position);

        // compute face normal
        const XMVECTOR e0 = v1 - v0;
        const XMVECTOR e1 = v2 - v0;
        const XMVECTOR normalVec = XMVector3Cross(e0, e1);

        XMFLOAT3 faceNormal;
        XMStoreFloat3(&faceNormal, normalVec);

        // this triangle shares the following three vertices, 
        // so add this face normal into the average of these vertex normals
        vertices[i0].normal.x = faceNormal.x;
        vertices[i0].normal.y = faceNormal.y;
        vertices[i0].normal.z = faceNormal.z;

        vertices[i1].normal.x = faceNormal.x;
        vertices[i1].normal.y = faceNormal.y;
        vertices[i1].normal.z = faceNormal.z;

        vertices[i2].normal.x = faceNormal.x;
        vertices[i2].normal.y = faceNormal.y;
        vertices[i2].normal.z = faceNormal.z;
    }

    // normalize normal vector of each vertex
    for (int i = 0; i < numVertices; ++i)
    {
        XMFLOAT3& n = vertices[i].normal;
        const float invLen = 1.0f / (SQR(n.x) + SQR(n.y) + SQR(n.z));

        n.x *= invLen;
        n.y *= invLen;
        n.z *= invLen;
    }
}


} // namespace
