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
// Desc:   initialize the sky plane: read its params from config file,
//         generate geometry, load textures and create a material
//---------------------------------------------------------
bool SkyPlane::Init(const SkyPlaneParams& params)
{
    // setup the current translation for the two textures
    // (we will pass it to shader when render)
    textureTranslation_[0] = 0.0f;
    textureTranslation_[1] = 0.0f;
    textureTranslation_[2] = 0.0f;
    textureTranslation_[3] = 0.0f;

    // setup brigtness and textures translation
    brightness_          = params.brightness;
    translationSpeed_[0] = params.translationSpeed[0];
    translationSpeed_[1] = params.translationSpeed[1];
    translationSpeed_[2] = params.translationSpeed[2];
    translationSpeed_[3] = params.translationSpeed[3];


    // generate sky plane's geometry
    GeometryGenerator geoGen;

    if (!geoGen.GenSkyPlane(
            params.skyPlaneResolution,
            params.skyPlaneLength,
            params.skyPlaneTop,
            params.skyPlaneBottom,
            params.texRepeat,
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
        LogErr(LOG, "can't init VB/IB for sky plane");
        Shutdown();
        return false;
    }

    // after initialization of buffers we release transient arrays
    SafeDeleteArr(vertices_);
    SafeDeleteArr(indices_);

    // set material for the sky plane
    matId_ = g_MaterialMgr.GetMatIdByName(params.materialName);

    if (matId_ == INVALID_MAT_ID)
    {
        LogErr(LOG, "no material by name: %s", params.materialName);
        Shutdown();
        return false;
    }

    assert(brightness_ > 0);

    // great success!
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

    if (!vb_.Init(vertices_, numVertices_, isDynamicBuf))
    {
        LogErr(LOG, "can't init VB for sky plane");
        return false;
    }

    if (!ib_.Init(indices_, numIndices_, isDynamicBuf))
    {
        LogErr(LOG, "can't init IB for sky plane");
        return false;
    }

    vertexStride_ = sizeof(VertexPosTex);

    return true;
}

} // namespace
