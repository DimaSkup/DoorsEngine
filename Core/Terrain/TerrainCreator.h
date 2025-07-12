// =================================================================================
// Filename:   TerrainCreator.h
// Desc:       functional for terrains creation/setup
//
// Created:    10.07.2025  by DimaSkup
// =================================================================================
#pragma once

#include "../Mesh/Vertex3dTerrain.h"
#include "../Model/GeometryGenerator.h"
#include <Log.h>
#include <MathHelper.h>

#include "TerrainGeomipmapCreator.h"
#include "TerrainQuadtreeCreator.h"


namespace Core
{

class TerrainCreator
{
public:
    static bool CreateTerrain(ID3D11Device* pDevice, const char* configFilename)
    {
        if (!configFilename || configFilename[0] == '\0')
        {
            LogErr(LOG, "input path to terrain's config is empty");
            return false;
        }

        CreateTerrainQuadtree(pDevice, configFilename);
        CreateTerrainGeomipmapped(pDevice, configFilename);

        return true;
    }
};


//==================================================================================


//---------------------------------------------------------
// Desc:   generate height for the input grid by some particular function;
//         (there can be several different types of height generation)
// Args:   - vertices:     an array of terrain's vertices
//         - indices:      an array of terrain's indices
//         - numVertices:  how many vertices we have
//         - numIndices:   how many indices we have
//---------------------------------------------------------
void GenerateHeightsForTerrainGrid(
    Vertex3dTerrain* vertices,
    UINT* indices,
    int numVertices,
    int numIndices)
{
    // check input args
    if (!vertices || !indices)
    {
        LogErr(LOG, "input ptr to vertices/indices array == nullptr");
        return;
    }

    if ((numVertices <= 0) || (numIndices <= 0))
    {
        LogErr(LOG, "input number of vertices/indices <= 0");
        return;
    }

// THE FIRST WAY to heights generation
#if 1
    for (int i = 0; i < numVertices; ++i)
    {
        DirectX::XMFLOAT3& pos = vertices[i].position;

        // a function for making hills for the terrain
        pos.y = 0.1f * (pos.z * sinf(0.1f * pos.x) + pos.x * cosf(0.1f * pos.z));

        // get hill normal
        // n = (-df/dx, 1, -df/dz)
        DirectX::XMVECTOR normalVec{
           -0.03f * pos.z * cosf(0.1f * pos.x) - 0.3f * cosf(0.1f * pos.z),
           1.0f,
           -0.3f * sinf(0.1f * pos.x) + 0.03f * pos.x * sinf(0.1f * pos.z) };

        normalVec = DirectX::XMVector3Normalize(normalVec);
        DirectX::XMStoreFloat3(&vertices[i].normal, normalVec);
    }

// THE SECOND WAY to heights generation
#elif 1

    // generate heights for the grid
    float m = 100.0f;
    float n = 100.0f;
    const float sin_step = DirectX::XM_PI / m * 3.0f;
    const float cos_step = DirectX::XM_PI / n * 5.0f;
    float valForSin = 0.0f;
    float valForCos = 0.0f;

    for (UINT i = 0; i < m; ++i)
    {
        valForSin = 0.0f;
        for (UINT j = 0; j < n; ++j)
        {
            const UINT idx = i * n + j;
            grid.vertices[idx].position.y = 30 * (sinf(valForSin) - cosf(valForCos));

            valForSin += sin_step;
        }
        valForCos += cos_step;
    }

#endif

#if 0
    // after creation of heights we compute tangent for each vertex
    GeometryGenerator geoGen;
    geoGen.ComputeTangents(vertices, indices, numIndices);
#endif
}

//---------------------------------------------------------
// Desc:   compute averaged normals to terrain's faces
//         (NOTE: not work for all types of terrains)
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
    if (!vertices || !indices)
    {
        LogErr("input arr of vertices/indices == nullptr");
        return;
    }

    if ((numVertices <= 0) || (numIndices <= 0))
    {
        LogErr("input number of vertices/indices must be > 0");
        return;
    }

    // for each triangle
    for (int i = 0; i < numIndices / 3; ++i)
    {
        // indices of the ith triangle 
        int baseIdx = i * 3;
        UINT i0 = indices[baseIdx + 0];
        UINT i1 = indices[baseIdx + 1];
        UINT i2 = indices[baseIdx + 2];

        // positions of vertices of ith triangle stored as XMVECTOR
        XMVECTOR v0 = DirectX::XMLoadFloat3(&vertices[i0].position);
        XMVECTOR v1 = DirectX::XMLoadFloat3(&vertices[i1].position);
        XMVECTOR v2 = DirectX::XMLoadFloat3(&vertices[i2].position);

        // compute face normal
        XMVECTOR e0 = v1 - v0;
        XMVECTOR e1 = v2 - v0;
        XMVECTOR normalVec = DirectX::XMVector3Cross(e0, e1);
        XMFLOAT3 faceNormal;
        DirectX::XMStoreFloat3(&faceNormal, normalVec);

        // this triangle shares the following three vertices, 
        // so add this face normal into the average of these vertex normals
        vertices[i0].normal += faceNormal;
        vertices[i1].normal += faceNormal;
        vertices[i2].normal += faceNormal;
    }

    // for each vertex v, we have summed the face normals of all
    // the triangles that share v, so now we just need to normalize
    for (int i = 0; i < numVertices; ++i)
        vertices[i].normal = DirectX::XMFloat3Normalize(vertices[i].normal);
}

} // namespace Core
