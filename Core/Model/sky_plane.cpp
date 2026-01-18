/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: sky_plane.cpp

    Created:  29.10.2025  by DimaSkup
\**********************************************************************************/
#pragma once
#include <CoreCommon/pch.h>
#include "sky_plane.h"
#include "geometry_generator.h"
#include <Render/d3dclass.h>         // from Render module
#include <Texture/texture_mgr.h>
#include <Mesh/material_mgr.h>
#pragma warning(disable : 4996)

namespace Core
{

SkyPlane::SkyPlane()
{
    LogMsg(LOG, "Sky plane constructor");
}

SkyPlane::~SkyPlane()
{
    LogMsg(LOG, "Sky plane destructor");
    Shutdown();
}

//---------------------------------------------------------
// Desc:  read in sky plane params from the config file
// Args:  - skyPlaneResolution: how many quads we have along X and Z axis
//        - texRepeat:          how many times to repeat a texture over the sky plane
//        - skyPlaneWidth:      length of the plane
//        - skyPlaneTop:        1 upper central point of the plane
//        - skyPlaneBottom:     4 bottom corner points of the plane
//        - brightness:         cloud brightness, lower values give clouds more faded look.
//        - translationSpeed:   arr of 4 values, how fast we translate the cloud textures over the sky plane
//        - tex0Name,tex1Name:  a name to cloud texture
//---------------------------------------------------------
void ReadParams(
    FILE* pFile,
    int& skyPlaneResolution,
    int& texRepeat,
    float& skyPlaneWidth,
    float& skyPlaneTop,
    float& skyPlaneBottom,
    float& brightness,
    float* translationSpeed,
    char* tex0Name,
    char* tex1Name)
{
    assert(pFile != nullptr);
    assert(translationSpeed != nullptr);
    assert(tex0Name != nullptr);
    assert(tex1Name != nullptr);


    int count = 0;

    count = fscanf(pFile, "sky_pl_resolution %d\n", &skyPlaneResolution);
    assert(count == 1);

    count = fscanf(pFile, "sky_pl_width %f\n", &skyPlaneWidth);
    assert(count == 1);

    count = fscanf(pFile, "sky_pl_top %f\n", &skyPlaneTop);
    assert(count == 1);

    count = fscanf(pFile, "sky_pl_bottom %f\n", &skyPlaneBottom);
    assert(count == 1);

    count = fscanf(pFile, "tex_repeat %d\n", &texRepeat);
    assert(count == 1);


    count = fscanf(pFile, "brightness %f\n", &brightness);
    assert(count == 1);
    assert(brightness >= 0 && brightness <= 1 && "brightness is out of range [0;1]");


    count = fscanf(pFile, "tex0_speed_x %f\n", &translationSpeed[0]);
    assert(count == 1);

    count = fscanf(pFile, "tex0_speed_z %f\n", &translationSpeed[1]);
    assert(count == 1);

    count = fscanf(pFile, "tex1_speed_x %f\n", &translationSpeed[2]);
    assert(count == 1);

    count = fscanf(pFile, "tex1_speed_z %f\n", &translationSpeed[3]);
    assert(count == 1);


    count = fscanf(pFile, "tex0_name %s\n", tex0Name);
    assert(count == 1);

    count = fscanf(pFile, "tex1_name %s\n", tex1Name);
    assert(count == 1);
}

//---------------------------------------------------------
// Desc:   initialize the sky plane: read its params from config file,
//         generate geometry, load textures and create a material
//---------------------------------------------------------
bool SkyPlane::Init(const char* cfgFilename)
{
    if (StrHelper::IsEmpty(cfgFilename))
    {
        LogErr(LOG, "can't init sky plane: input filename is empty");
        return false;
    }


    FILE* pFile = fopen(cfgFilename, "r");
    if (!pFile)
    {
        LogErr(LOG, "can't open sky plane config file: %s", cfgFilename);
        return false;
    }


    // read in config params
    int   count              = 0;
    int   skyPlaneResolution = 0;
    int   texRepeat          = 0;
    float skyPlaneWidth      = 0;
    float skyPlaneTop        = 0;
    float skyPlaneBottom     = 0;
    char  tex0Name[MAX_LEN_TEX_NAME]{'\0'};
    char  tex1Name[MAX_LEN_TEX_NAME]{'\0'};

    ReadParams(
        pFile,
        skyPlaneResolution,
        texRepeat,
        skyPlaneWidth,
        skyPlaneTop,
        skyPlaneBottom,
        brightness_,
        translationSpeed_,
        tex0Name,
        tex1Name);
   

    // setup the current translation for the two textures and pass it to shader when render
    textureTranslation_[0] = 0.0f;
    textureTranslation_[1] = 0.0f;
    textureTranslation_[2] = 0.0f;
    textureTranslation_[3] = 0.0f;

    // generate sky plane's geometry
    GeometryGenerator geoGen;

    if (!geoGen.GenSkyPlane(
            skyPlaneResolution,
            skyPlaneWidth,
            skyPlaneTop,
            skyPlaneBottom,
            texRepeat,
            &vertices_,
            &indices_,
            numVertices_,
            numIndices_))
    {
        LogErr(LOG, "can't generate vertices for sky plane");
        Shutdown();
        return false;
    }


    if (!InitBuffers())
    {
        LogErr(LOG, "can't init vertex/index buffer for sky plane");
        Shutdown();
        return false;
    }

    // after initialization of buffers we release transient arrays
    SafeDeleteArr(vertices_);
    SafeDeleteArr(indices_);
    

    // get textures and create a material
    const TexID tex0Id = g_TextureMgr.GetTexIdByName(tex0Name);
    const TexID tex1Id = g_TextureMgr.GetTexIdByName(tex1Name);

    if (tex0Id == INVALID_TEX_ID)
    {
        LogErr(LOG, "no texture by name: %s", tex0Name);
    }
    if (tex1Id == INVALID_TEX_ID)
    {
        LogErr(LOG, "no texture by name: %s", tex1Name);
    }

    Material& mat = g_MaterialMgr.AddMaterial("sky_plane_clouds");
    mat.texIds[1] = tex0Id;
    mat.texIds[2] = tex1Id;

    //mat.SetBlending(MAT_PROP_BS_ENABLE);
    //mat.SetDepthStencil(MAT_PROP_DSS_SKY_DOME);

    matId_ = mat.id;

    // great success!
    fclose(pFile);
    return true;
}

//---------------------------------------------------------
// Desc:  release memory
//---------------------------------------------------------
void SkyPlane::Shutdown()
{
    SafeDeleteArr(vertices_);
    SafeDeleteArr(indices_);

    numVertices_ = 0;
    numIndices_  = 0;

    vb_.Shutdown();
    ib_.Shutdown();
}

//---------------------------------------------------------
// Desc:  frame processing that we do for the sky plane is the cloud texture
//        translation which simulates movement of the clouds across the sky
// Args:  - dt:  delta time (the time passed since the prev frame)
//---------------------------------------------------------
void SkyPlane::Update(const float dt)
{
    // translate the first cloud by X and Z
    textureTranslation_[0] += translationSpeed_[0] * dt;
    textureTranslation_[1] += translationSpeed_[1] * dt;

    // translate the second cloud by X and Z
    textureTranslation_[2] += translationSpeed_[2] * dt;
    textureTranslation_[3] += translationSpeed_[3] * dt;

    // clamp values in range [0,1]
    if (textureTranslation_[0] > 1.0f) { textureTranslation_[0] -= 1.0f; }
    if (textureTranslation_[1] > 1.0f) { textureTranslation_[1] -= 1.0f; }
    if (textureTranslation_[2] > 1.0f) { textureTranslation_[2] -= 1.0f; }
    if (textureTranslation_[3] > 1.0f) { textureTranslation_[3] -= 1.0f; }
}

//---------------------------------------------------------
// Desc:  initialize a vertex and index buffer with
//        precomputed vertices and indices
//---------------------------------------------------------
bool SkyPlane::InitBuffers()
{
    if (!vertices_)
    {
        LogErr(LOG, "lol, your vertices buffer is empty");
        return false;
    }
    if (!indices_)
    {
        LogErr(LOG, "lol, your indices buffer is empty");
        return false;
    }

    constexpr bool isDynamicBuf = false;

    vb_.Shutdown();
    ib_.Shutdown();

    if (!vb_.Initialize(Render::g_pDevice, vertices_, numVertices_, isDynamicBuf))
    {
        LogErr(LOG, "can't init a vertex buffer for sky plane");
        return false;
    }

    if (!ib_.Initialize(Render::g_pDevice, indices_, numIndices_, isDynamicBuf))
    {
        LogErr(LOG, "can't init an index buffer for sky plane");
        return false;
    }

    vertexStride_ = sizeof(VertexPosTex);

    return true;
}

} // namespace
