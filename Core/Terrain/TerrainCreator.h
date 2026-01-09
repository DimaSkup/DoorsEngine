// =================================================================================
// Filename:   TerrainCreator.h
// Desc:       functional for terrains creation/setup
//
// Created:    10.07.2025  by DimaSkup
// =================================================================================
#pragma once

#include "../Mesh/vertex3d_terrain.h"
#include "../Model/geometry_generator.h"
#include "terrain_geomip_creator.h"
#include <Log.h>


namespace Core
{

class TerrainCreator
{
public:

    static bool CreateTerrain(const char* configFilepath)
    {
        if (StrHelper::IsEmpty(configFilepath))
        {
            LogErr(LOG, "config filepath is empty");
            return false;
        }

        TerrainGeomipCreator creator;

        if (!creator.Create(configFilepath))
        {
            LogErr(LOG, "can't create a terrain for some reason");
            return false;
        }

        return true;
    }
};

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
        DirectX::XMFLOAT3& n = vertices[i].normal;
        const float invLen   = 1.0f / (SQR(n.x) + SQR(n.y) + SQR(n.z));

        n.x *= invLen;
        n.y *= invLen;
        n.z *= invLen;
    }
        
}

} // namespace Core
