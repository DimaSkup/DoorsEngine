/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: vertices_splitter.h
    Desc:     automatically detects UV seams
              (including mirrored UVs) and splits vertices correctly
              before tangent generation:

              1. for every triangle, examine its 3 vertices
              2. for each vertex in triangle:
                 - look up the original vertex index (pos index, normal index, etc.)
                 - check if this vertex was already used with different UVs
              3. if UVs differ, create a duplicate vertex
              4. build a new index buffer referencing the correct
                 (possibly duplicated) vertices

              This guarantees every triangle corner with unique UVs gets its own vertex,
              which is required for proper tangent-space computation

    Created:  02.12.2025  by DimaSkup
\**********************************************************************************/
#pragma once

#include <math/vec2.h>
#include <math/vec3.h>
#include <types.h>
#include <cvector.h>
#include <unordered_map>

namespace Core
{

//---------------------------------------------------------
// helper structures
//---------------------------------------------------------
struct RawMesh
{
    cvector<Vec3> positions;
    cvector<Vec3> normals;
    cvector<Vec2> uvs;
    cvector<uint32> indices;   // triangle index list
};

struct VertexKey
{
    uint32 posIdx;
    uint32 normalIdx;
    Vec2 uv;
};

struct Vec2Hash
{
    size_t operator()(const Vec2& v) const
    {
        const size_t h1 = std::hash<float>()(v.x);
        const size_t h2 = std::hash<float>()(v.y);
        return h1 ^ (h2 << 1);
    }
};

struct VertexKeyHash
{
    size_t operator()(VertexKey const& k) const
    {
        const size_t h1 = std::hash<uint32>()(k.posIdx);
        const size_t h2 = std::hash<uint32>()(k.normalIdx);
        const size_t h3 = Vec2Hash()(k.uv);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

struct VertexKeyEq
{
    bool operator()(VertexKey const& v0, VertexKey const& v1) const
    {
        return (v0.posIdx == v1.posIdx) &&
               (v0.normalIdx == v1.normalIdx) &&
               fabs(v0.uv.x - v1.uv.x) < EPSILON_E6 &&
               fabs(v0.uv.y - v1.uv.y) < EPSILON_E6;
    }
};

//---------------------------------------------------------
// Vertex splitter
//---------------------------------------------------------
void SplitMirroredUVVertices(
    const cvector<Vec3>& inPos,
    const cvector<Vec3>& inNorm,
    const cvector<Vec2>& inUV,
    const cvector<uint32>& inIndices,

    cvector<Vec3>& outPos,
    cvector<Vec3>& outNorm,
    cvector<Vec2>& outUV,
    cvector<uint32>& outIndices)
{
    std::unordered_map<VertexKey, uint32, VertexKeyHash, VertexKeyEq> map;

    outPos.reserve(inPos.size());
    outNorm.reserve(inNorm.size());
    outUV.reserve(inUV.size());
    outIndices.resize(inIndices.size());


    for (index i = 0; i < inIndices.size(); ++i)
    {
        uint32 idx = inIndices[i];

        VertexKey key;
        key.posIdx    = idx;
        key.normalIdx = idx;
        key.uv        = inUV[idx];

        const auto it = map.find(key);
        if (it == map.end())
        {
            // create new vertex
            const uint32 newIdx = (uint32)(outPos.size());

            map[key] = newIdx;

            outPos.push_back(inPos[idx]);
            outNorm.push_back(inNorm[idx]);
            outUV.push_back(inUV[idx]);

            outIndices[i] = newIdx;
        }
        else
        {
            outIndices[i] = it->second;
        }
    }
}

}
