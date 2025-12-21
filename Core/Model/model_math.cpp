////////////////////////////////////////////////////////////////////
// Filename:     model_math.cpp
// Created:      06.02.23
////////////////////////////////////////////////////////////////////
#include <CoreCommon/pch.h>
#include "model_math.h"

using namespace DirectX;

namespace Core
{

//---------------------------------------------------------
// Desc:  calculate per-vertex normals with interpolation
//---------------------------------------------------------
void ModelMath::CalcNormals(
    Vertex3D* vertices,
    const UINT* indices,
    const int numVertices,
    const int numIndices)
{
    // reset all normals
    for (int i = 0; i < numVertices; ++i)
        vertices[i].normal = { 0,0,0 };

    // for each triangle
    for (int i = 0; i < numIndices; i += 3)
    {
        const UINT i0 = indices[i + 0];
        const UINT i1 = indices[i + 1];
        const UINT i2 = indices[i + 2];

        const XMVECTOR pos0 = DirectX::XMLoadFloat3(&vertices[i0].position);
        const XMVECTOR pos1 = DirectX::XMLoadFloat3(&vertices[i1].position);
        const XMVECTOR pos2 = DirectX::XMLoadFloat3(&vertices[i2].position);

        const XMVECTOR normal = XMVector3Cross(pos1 - pos0, pos2 - pos0);

        vertices[i0].normal += normal;
        vertices[i1].normal += normal;
        vertices[i2].normal += normal;
    }

    // normalize all the vertex normals
    for (int i = 0; i < numVertices; ++i)
        XMFloat3Normalize(vertices[i].normal);
}

//---------------------------------------------------------
// Desc:  compute per-vertex tangents
//
// from: https://terathon.com/blog/tangent-space.html
//---------------------------------------------------------
void ModelMath::CalcTangents(
    Vertex3D* vertices,
    const UINT* indices,
    const uint numVertices,
    const uint numIndices)
{
    if (!vertices || !indices)
    {
        LogErr(LOG, "vertices or indices arr == nullptr");
        return;
    }

    // reset all tangents to zero
    Vec3* tangs   = new Vec3[numVertices * 2];
    Vec3* bitangs = tangs + numVertices;
    memset(tangs, 0, numVertices * sizeof(Vec3) * 2);


    for (uint i = 0; i < numIndices; i += 3)
    {
        const UINT i1 = indices[i + 0];
        const UINT i2 = indices[i + 1];
        const UINT i3 = indices[i + 2];

        assert(i1 < numVertices);
        assert(i2 < numVertices);
        assert(i3 < numVertices);

        const Vertex3D& v1 = vertices[i1];
        const Vertex3D& v2 = vertices[i2];
        const Vertex3D& v3 = vertices[i3];

        const XMFLOAT3& p1 = v1.position;
        const XMFLOAT3& p2 = v2.position;
        const XMFLOAT3& p3 = v3.position;

        const XMFLOAT2& w1 = v1.texture;
        const XMFLOAT2& w2 = v2.texture;
        const XMFLOAT2& w3 = v3.texture;


        float x1 = p2.x - p1.x;
        float y1 = p2.y - p1.y;
        float z1 = p2.z - p1.z;

        float x2 = p3.x - p1.x;
        float y2 = p3.y - p1.y;
        float z2 = p3.z - p1.z;

        float s1 = w2.x - w1.x;
        float s2 = w3.x - w1.x;
        float t1 = w2.y - w1.y;
        float t2 = w3.y - w1.y;

        float r = 1.0f / (s1 * t2 - s2 * t1);

        const Vec3 tangent  = Vec3((t2*x1 - t1*x2), (t2*y1 - t1*y2), (t2*z1 - t1*z2)) * r;

        const Vec3 binormal = Vec3((s1*x2 - s2*x1), (s1*y2 - s2*y1), (s1*z2 - s2*z1)) * r;

        tangs[i1]   += tangent;
        tangs[i2]   += tangent;
        tangs[i3]   += tangent;

        bitangs[i1] += binormal;
        bitangs[i2] += binormal;
        bitangs[i3] += binormal;
    }


    for (uint i = 0; i < numVertices; ++i)
    {
        Vertex3D& v = vertices[i];

        const Vec3  n(&v.normal.x);
        const Vec3& t = tangs[i];

        // Gram-Schmidt orthogonalize
        const Vec3 T = Vec3Normalize(t - n * Vec3Dot(n,t));

        v.tangent = { T.x, T.y, T.z, 1.0f };

        // calc handedness
        if (Vec3Dot(Vec3Cross(n, t), bitangs[i]) < 0.0f)
            v.tangent.w = -1.0f;
    }

    SafeDeleteArr(tangs);
}

} // namespace
