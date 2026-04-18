/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: sky_initializer.cpp
    Desc:     implementation of the SkyInitializer class

    Created:  02.04.2026  by DimaSkup
\**********************************************************************************/
#include "../Common/pch.h"
#include "sky_initializer.h"
#include <Model/model_mgr.h>
#include <Texture/texture_mgr.h>
#include <Mesh/material_mgr.h>

#pragma warning (disable : 4996)


namespace Game
{

using SkyBoxParams   = Core::SkyModel::SkyBoxParams;
using SkyPlaneParams = Core::SkyPlane::SkyPlaneParams;

using Core::g_TextureMgr;
using Core::g_MaterialMgr;
using Core::g_ModelMgr;


//---------------------------------------------------------
// internal helper structures
//---------------------------------------------------------
struct SkyDomeParams
{
    Vec3 colorCenter;
    Vec3 colorApex;
    uint numSlices;
    uint numStacks;
};

//---------------------------------------------------------
// internal helpers
//---------------------------------------------------------
void CreateSkyBox  (FILE* pFile, ECS::EntityMgr& enttMgr);
void CreateSkyPlane(FILE* pFile);
void CreateSkyDome (FILE* pFile);

void ReadSkyBoxParams  (FILE* pFile, SkyBoxParams& params);
void ReadSkyPlaneParams(FILE* pFile, SkyPlaneParams& params);
void ReadSkyDomeParams (FILE* pFile, SkyDomeParams& params);


//---------------------------------------------------------
// Desc:  read in sky configs from a file by cfgFilepath
//        and create/initialize all the sky stuff: skybox, skydome, skyplane, etc.
//---------------------------------------------------------
bool SkyInitializer::Init(const char* cfgFilepath, ECS::EntityMgr& enttMgr)
{
    if (StrHelper::IsEmpty(cfgFilepath))
    {
        LogErr(LOG, "empty filepath");
        return false;
    }

    char buf[128];

    // open file for reading
    FILE* pFile = fopen(cfgFilepath, "r");
    if (!pFile)
    {
        LogErr(LOG, "can't open file: %s", cfgFilepath);
        return false;
    }

    // skip comments section
    do {
        fgets(buf, sizeof(buf), pFile);
    } while (buf[0] == ';');


    // read in declarations of sky elements
    while (fgets(buf, sizeof(buf), pFile))
    {
        if (strncmp(buf, "skybox", 6) == 0)
            CreateSkyBox(pFile, enttMgr);

        else if (strncmp(buf, "skyplane", 8) == 0)
            CreateSkyPlane(pFile);

        else if (strncmp(buf, "skydome", 7) == 0)
            CreateSkyDome(pFile);
    }

    fclose(pFile);
    return true;
}

//---------------------------------------------------------
// Desc:    load a sky params from config file,
//          create sky model, and create sky entity
//---------------------------------------------------------
void CreateSkyBox(FILE* pFile, ECS::EntityMgr& enttMgr)
{
    using namespace Core;
    assert(pFile);

    LogDbg(LOG, "create a skybox");

    ModelsCreator creator;
    Material* pMat = nullptr;
    TexID skyMapId = INVALID_TEX_ID;

    SkyBoxParams skyParams;
    memset(&skyParams, 0, sizeof(skyParams));

    // read init params from file
    ReadSkyBoxParams(pFile, skyParams);

    // load a texture for the sky
    if (skyParams.bLoadCubeMap)
        skyMapId = g_TextureMgr.GetTexIdByName(skyParams.texName);

    else
        skyMapId = g_TextureMgr.CreateCubeMap(skyParams.texName, skyParams.cubeMapParams);


    // setup a sky material
    pMat = &g_MaterialMgr.GetMatByName(skyParams.materialName);
    pMat->SetTexture(TEX_TYPE_DIFFUSE, skyMapId);

    // create and setup a sky model
    creator.CreateSkyCube(skyParams.size);
    g_ModelMgr.GetSky().SetMaterialId(pMat->id);

    // create and setup a sky entity
    const EntityID             enttId = enttMgr.CreateEntity("sky");
    const DirectX::XMFLOAT3 skyBoxPos = { 0, skyParams.offsetY,0 };

    enttMgr.AddTransformComponent(enttId, skyBoxPos);
    enttMgr.AddMaterialComponent(enttId, pMat->id);
}

//---------------------------------------------------------
// Desc:  read in params from file for a sky plane and initialize it
//---------------------------------------------------------
void CreateSkyPlane(FILE* pFile)
{
    assert(pFile);
    LogDbg(LOG, "create a skyplane");

    SkyPlaneParams params;
    memset(&params, 0, sizeof(params));

    ReadSkyPlaneParams(pFile, params);

    if (!g_ModelMgr.GetSkyPlane().Init(params))
    {
        LogFatal(LOG, "can't init a sky plane");
    }
}

//---------------------------------------------------------
// Desc:  read in params from file for a sky dome and initialize it
//---------------------------------------------------------
void CreateSkyDome(FILE* pFile)
{
    assert(pFile);
}

//---------------------------------------------------------
// Desc:  read in sky box params from the config file
//---------------------------------------------------------
void ReadSkyBoxParams(FILE* pFile, SkyBoxParams& params)
{
    assert(pFile);

    ReadFileInt   (pFile, "load_cubemap_texture",  &params.bLoadCubeMap);
    ReadFileStr   (pFile, "material",              params.materialName);
    ReadFileStr   (pFile, "cubemap_texture",       params.texName);
    ReadFileStr   (pFile, "cubemap_dir",           params.cubeMapParams.directory);

    ReadFileStr   (pFile, "sky_pos_x",             params.cubeMapParams.texNames[0]);
    ReadFileStr   (pFile, "sky_neg_x",             params.cubeMapParams.texNames[1]);
    ReadFileStr   (pFile, "sky_pos_y",             params.cubeMapParams.texNames[2]);
    ReadFileStr   (pFile, "sky_neg_y",             params.cubeMapParams.texNames[3]);
    ReadFileStr   (pFile, "sky_pos_z",             params.cubeMapParams.texNames[4]);
    ReadFileStr   (pFile, "sky_neg_z",             params.cubeMapParams.texNames[5]);

    ReadFileFloat (pFile, "sky_box_size",          &params.size);
    ReadFileFloat (pFile, "sky_box_offset_y",      &params.offsetY);

    // check params
    assert(params.size > 0);
}

//---------------------------------------------------------
// Desc:  read in sky plane params from the config file
// Args:
//---------------------------------------------------------
void ReadSkyPlaneParams(FILE* pFile, SkyPlaneParams& params)   
{
    assert(pFile);

    ReadFileInt     (pFile, "sky_pl_resolution", &params.skyPlaneResolution);

    ReadFileFloat   (pFile, "sky_pl_length", &params.skyPlaneLength);
    ReadFileFloat   (pFile, "sky_pl_top",    &params.skyPlaneTop);
    ReadFileFloat   (pFile, "sky_pl_bottom", &params.skyPlaneBottom);
    ReadFileInt     (pFile, "tex_repeat",    &params.texRepeat);
    ReadFileFloat   (pFile, "brightness",    &params.brightness);


    ReadFileFloat   (pFile, "tex0_speed_x",  &params.translationSpeed[0]);
    ReadFileFloat   (pFile, "tex0_speed_z",  &params.translationSpeed[1]);
    ReadFileFloat   (pFile, "tex1_speed_x",  &params.translationSpeed[2]);
    ReadFileFloat   (pFile, "tex1_speed_z",  &params.translationSpeed[3]);
    ReadFileStr     (pFile, "material",      params.materialName);

    // check params
    assert(params.skyPlaneResolution > 0);
    assert(params.skyPlaneLength >= params.skyPlaneResolution);
    assert(params.skyPlaneBottom >= 0);
    assert(params.skyPlaneTop > params.skyPlaneBottom);

    assert(params.texRepeat > 0);
    assert(params.brightness >= 0 && params.brightness <= 1);
}


}
