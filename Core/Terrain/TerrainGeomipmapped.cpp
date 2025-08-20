// =================================================================================
// Filename:  TerrainGeomip.cpp
// Desc:      implementation of terrain geomimapping CLOD algorithm
//            (CLOD - continuous level of detail)
//
// Created:   10.06.2025 by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include <CoreCommon/Frustum.h>
#include "TerrainGeomipmapped.h"
#include "../Mesh/MaterialMgr.h"
#include <Render/d3dclass.h>      // for using global pointers to DX11 device and context

#include <DirectXMath.h>
#include <DirectXCollision.h>

using namespace DirectX;


namespace Core
{

// 0 - we don't execute any smoothing computations during terrain initialization
// 1 - mid smothing
// 2 - hard smoothing
constexpr int TERRAIN_SMOOTHING_LEVEL = 2;


//---------------------------------------------------------
// Desc:   release memory from the vertices/indices buffers
//---------------------------------------------------------
void TerrainGeomip::ReleaseBuffers()
{
    SafeDeleteArr(vertices_);
    SafeDeleteArr(indices_);

    // release memory from the vertex buffer and index buffer
    vb_.Shutdown();
    ib_.Shutdown();
}

//---------------------------------------------------------
// Desc:   release all the memory related to the terrain
//---------------------------------------------------------
void TerrainGeomip::Shutdown()
{
    ReleaseBuffers();
    ClearMemoryFromMaps();
}

//---------------------------------------------------------
// Desc:   initiate the geomipmapping system
// Args:   - patchSize:  the size of the patch (in vertices)
//                       a good size is usually around 17 (17x17 verts)
//---------------------------------------------------------
bool TerrainGeomip::InitGeomipmapping(const int patchSize)
{
    int divisor = 0;
    int LOD = 0;
    int numAllPatches = 0;

    // since terrain is squared the width and height are equal
    const int terrainLen = GetTerrainLength();

    LogMsg(LOG, "start initialization of terrain geometry and buffers (geomip)");

    // check if we actually able to properly initialize the terrain's geometry 
    if ((terrainLen - 1) % (patchSize - 1) != 0)
    {
        int patchSz = patchSize - 1;
        int recommendedTerrainSize = ((terrainLen -1 + patchSz) / (patchSz)) * (patchSz) + 1;
        
        LogErr(LOG, "terrain length minus 1 (%d) must be divisible by patch size minus 1 (%d)", terrainLen, patchSize);
        SetConsoleColor(YELLOW);
        LogMsg("Try using terrain size = %d", recommendedTerrainSize);
        
        return false;
    }

    if (patchSize < 3)
    {
        LogErr(LOG, "the minimum patch size is 3 (%d)", patchSize);
        return false;
    }

    if (patchSize % 2 == 0)
    {
        LogErr(LOG, "patch size must be an odd number (%d)", patchSize);
        return false;
    }

    // init the LOD manager
    const int numPatchesPerSide = (terrainLen-1) / (patchSize-1);
    const int maxLod = lodMgr_.Init(patchSize, numPatchesPerSide);
    lodInfo_.resize(maxLod + 1);

    // init vertex/index buffers
    PopulateBuffers();

    LogMsg("Geomipmapping system successfully initialized");
    return true;
}

//---------------------------------------------------------
// Desc:   initialize DirectX vertex/index buffers
//---------------------------------------------------------
bool TerrainGeomip::InitBuffers(
    const Vertex3dTerrain* vertices,
    const UINT* indices,
    const int numVertices,
    const int numIndices)
{
    constexpr bool isDynamic = false;

    // initialize the vertex buffer
    if (!vb_.Initialize(Render::g_pDevice, vertices, numVertices, isDynamic))
    {
        LogErr(LOG, "can't initialize a vertex buffer for terrain");
        Shutdown();
        return false;
    }

    // initialize the index buffer
    if (!ib_.Initialize(Render::g_pDevice, indices, numIndices, isDynamic))
    {
        LogErr(LOG, "can't initialize an index buffer for terrain");
        Shutdown();
        return false;
    }

    return true;
}

//---------------------------------------------------------
// Desc:   init all the vertices of the terrain (its positions, texture coords, etc.)
// Args:   - vertices:      array of vertices
//         - numVertices:   number of vertices in the array
//---------------------------------------------------------
void TerrainGeomip::InitVertices(Vertex3dTerrain* vertices, const int numVertices)
{
    int idx = 0;
    const int   terrainLen    = GetTerrainLength();
    const float invTerrainLen = 1.0f / (float)terrainLen;

    float yUp    = 0;
    float yRight = 0;
    float yDown  = 0;
    float yLeft  = 0;


    // no smoothing computations
    if constexpr (TERRAIN_SMOOTHING_LEVEL == 0)
    {
        for (int z = 0; z < terrainLen; z++)
        {
            for (int x = 0; x < terrainLen; x++)
            {
                assert(idx < numVertices);

                const float fX = (float)x;
                const float fY = GetScaledHeightAtPoint(x, z);
                const float fZ = (float)z;
                const float texU = fX * invTerrainLen;
                const float texV = fZ * invTerrainLen;

                vertices[idx] = { fX, fY, fZ, texU, texV };
                idx++;
            }
        }
    }

    //-------------------------------------------------
    // do some smoothing
    if constexpr (TERRAIN_SMOOTHING_LEVEL == 1)
    {
        for (int z = 0; z < terrainLen; z++)
        {
            for (int x = 0; x < terrainLen; x++)
            {
                assert(idx < numVertices);

                // compute averaged interpolated height to make our terrain smoother
                float heightSum = 0;
                int numHeights = 0;

                /*
                    v - our vertex
                    * - neighbours which used for interpolation
                        
                        *
                        |
                    *---v---*
                        |
                        *
                */

                // get and sum bottom height if we have such
                if (z > 0)
                {
                    heightSum += GetScaledHeightAtPoint(x, z - 1);
                    numHeights++;
                }

                // get and sum left height if we have such
                if (x > 0)
                {
                    heightSum += GetScaledHeightAtPoint(x - 1, z);
                    numHeights++;
                }

                // get and sum right height if we have such
                if (x < terrainLen)
                {
                    heightSum += GetScaledHeightAtPoint(x + 1, z);
                    numHeights++;
                }

                // get and sum upper height if we have such
                if (z < terrainLen)
                {
                    heightSum += GetScaledHeightAtPoint(x, z + 1);
                    numHeights++;
                }

                const float heightInterpolated = (heightSum / numHeights);

                const float fX = (float)x;
                const float fY = (GetScaledHeightAtPoint(x, z) + heightInterpolated) * 0.5f;
                const float fZ = (float)z;
                const float texU = fX * invTerrainLen;
                const float texV = fZ * invTerrainLen;

                vertices[idx] = { fX, fY, fZ, texU, texV };
                idx++;
            }
        }
    }

    //-------------------------------------------------
    // hard smoothing
    if constexpr (TERRAIN_SMOOTHING_LEVEL == 2)
    {
        for (int z = 0; z < terrainLen; z++)
        {
            for (int x = 0; x < terrainLen; x++)
            {
                assert(idx < numVertices);

                // compute averaged interpolated height to make our terrain smoother
                float heightSum = 0;
                int numHeights = 0;

                /*
                    v - our vertex
                    * - neighbours which used for interpolation
                        
                    *---*---*
                    |   |   |
                    *---v---*
                    |   |   |
                    *---*---*
                */

                // if we aren't on the edge of the terrain
                if ((x > 0 && z > 0) && (x < terrainLen && z < terrainLen))
                {
                    // bottom line of quad
                    heightSum += GetScaledHeightAtPoint(x-1, z-1);
                    heightSum += GetScaledHeightAtPoint(x,   z-1);
                    heightSum += GetScaledHeightAtPoint(x+1, z-1);

                    // middle line of quad
                    heightSum += GetScaledHeightAtPoint(x-1, z);
                    heightSum += GetScaledHeightAtPoint(x+1, z);

                    // top line of quad
                    heightSum += GetScaledHeightAtPoint(x-1, z+1);
                    heightSum += GetScaledHeightAtPoint(x,   z+1);
                    heightSum += GetScaledHeightAtPoint(x+1, z+1);

                    numHeights += 8;
                }
                // handle specific cases
                else
                {
                    // get and sum bottom height if we have such
                    if (z > 0)
                    {
                        heightSum += GetScaledHeightAtPoint(x, z - 1);
                        numHeights++;
                    }

                    // get and sum left height if we have such
                    if (x > 0)
                    {
                        heightSum += GetScaledHeightAtPoint(x - 1, z);
                        numHeights++;
                    }

                    // get and sum right height if we have such
                    if (x < terrainLen)
                    {
                        heightSum += GetScaledHeightAtPoint(x + 1, z);
                        numHeights++;
                    }

                    // get and sum upper height if we have such
                    if (z < terrainLen)
                    {
                        heightSum += GetScaledHeightAtPoint(x, z + 1);
                        numHeights++;
                    }
                }

                const float heightInterpolated = (heightSum / numHeights);

                const float fX = (float)x;
                const float fY = (GetScaledHeightAtPoint(x, z) + heightInterpolated) * 0.5f;
                const float fZ = (float)z;
                const float texU = fX * invTerrainLen;
                const float texV = fZ * invTerrainLen;

                vertices[idx] = { fX, fY, fZ, texU, texV };
                idx++;
            }
        }
    }

    assert(idx == numVertices);
}

//---------------------------------------------------------
// Desc:   test if we are currently in the patch by input coords
// Args:   - camPosX, camPosZ: position of the camera by X-axis and Z-axis
//         - cx, cz:           center of the patch by X-axis and Z-axis
//         - halfSize:         half of the patch length
//---------------------------------------------------------
inline bool TestWeInPatch(
    const int camPosX,
    const int camPosZ,
    const int cx,
    const int cz,
    const int halfSize)
{
    return (camPosX >= cx-halfSize) || (camPosX <= cx+halfSize) ||
           (camPosZ >= cz-halfSize) || (camPosZ <= cz+halfSize);
}

//---------------------------------------------------------
// Desc:   calculate a normal vector for each terrain's vertex
// Args:   - vertices:  arr of terrain's vertices
//         - indices:   arr of terrain's indices
//         - numVertices:  how many vertices in the vertices arr
//---------------------------------------------------------
void TerrainGeomip::CalcNormals(
    Vertex3dTerrain* vertices,
    const UINT* indices,
    const int numVertices,
    const int numIndices)
{
    const int terrainLen = GetTerrainLength();
    const int numQuadsPerSide = terrainLen - 1;
    const int patchLen = lodMgr_.patchSize_ - 1;

    UINT idx = 0;

    // accumulate each triangle triangle normal into each of the triangle vertices
    for (int z = 0; z < numQuadsPerSide; z += patchLen)
    {
        for (int x = 0; x < numQuadsPerSide; x += patchLen)
        {
            const int baseVertexIdx = (z * terrainLen) + x;

            // get number of indices of the patch with maximal detalization level
            const int numIdxs = lodInfo_[0].info[0][0][0][0].count;

            // for each triangle
            for (int i = 0; i < numIdxs; i += 3)
            {
                const UINT i0         = baseVertexIdx + indices[i + 0];
                const UINT i1         = baseVertexIdx + indices[i + 1];
                const UINT i2         = baseVertexIdx + indices[i + 2];

                const XMVECTOR pos0   = DirectX::XMLoadFloat3(&vertices[i0].position);
                const XMVECTOR pos1   = DirectX::XMLoadFloat3(&vertices[i1].position);
                const XMVECTOR pos2   = DirectX::XMLoadFloat3(&vertices[i2].position);

                const XMVECTOR normal = XMVector3Cross(pos1 - pos0, pos2 - pos0);

                vertices[i0].normal += normal;
                vertices[i1].normal += normal;
                vertices[i2].normal += normal;
            }
        }
    }

    // normalize all the vertex normals
    for (int i = 0; i < numVertices; ++i)
        XMFloat3Normalize(vertices[i].normal);
}

//---------------------------------------------------------
// Desc:   test if patch by (x,z) is in the frustum
//---------------------------------------------------------
bool TerrainGeomip::IsPatchInsideViewFrustum(
    const int x0,
    const int z0,
    const DirectX::BoundingFrustum& frustum)
{
    // generate bounding box and sphere for terrain's patch by (x0,y0)
    const int size = lodMgr_.patchSize_ - 1;
    const int x1   = x0 + size;
    const int z1   = z0 + size;

    const float h00 = GetScaledHeightAtPoint(x0, z0);
    const float h10 = GetScaledHeightAtPoint(x1, z0);
    const float h01 = GetScaledHeightAtPoint(x0, z1);
    const float h11 = GetScaledHeightAtPoint(x1, z1);

    const float minHeight = min(h00, min(h01, min(h10, h11)));
    const float maxHeight = max(h00, max(h01, max(h10, h11)));

    DirectX::BoundingBox    aabb;
    DirectX::BoundingSphere sphere;
    DirectX::XMVECTOR       minPoint = { (float)x0, minHeight, (float)z0 };
    DirectX::XMVECTOR       maxPoint = { (float)x1, maxHeight, (float)z1 };

    // test bounding shapes against the frustum
    DirectX::BoundingBox::CreateFromPoints(aabb, minPoint, maxPoint);
    DirectX::BoundingSphere::CreateFromBoundingBox(sphere, aabb);

    return frustum.Intersects(sphere);

#if 0

    return fc.Intersects(aabb);

    return frustum.SphereTest(cx, cy, cz, radius);

    return
        frustum.PointTest(fX0, minHeight, fZ0) ||
        frustum.PointTest(fX0, minHeight, fZ1) ||
        frustum.PointTest(fX1, minHeight, fZ0) ||
        frustum.PointTest(fX1, minHeight, fZ1) ||

        frustum.PointTest(fX0, maxHeight, fZ0) ||
        frustum.PointTest(fX0, maxHeight, fZ1) ||
        frustum.PointTest(fX1, maxHeight, fZ0) ||
        frustum.PointTest(fX1, maxHeight, fZ1);
#endif
}

//---------------------------------------------------------
// Desc:   update the geomipmapping system
// Args:   - camParams: camera params for LODs computation and frustum culling
//---------------------------------------------------------
void TerrainGeomip::Update(const CameraParams& cam)
{
    lodMgr_.Update(cam.posX, cam.posY, cam.posZ);

    // create a view frustum planes
    //const Frustum frustum(cam.fov, cam.aspectRatio, cam.nearZ, cam.farZ);

    DirectX::BoundingFrustum frustum;

    //const XMMATRIX view    = XMMATRIX(cam.view);
    const XMMATRIX invView = XMMatrixInverse(nullptr, XMMATRIX(cam.view));
    DirectX::BoundingFrustum::CreateFromMatrix(frustum, XMMATRIX(cam.proj));

    // transform each frustum plane into world space

    const int patchSize         = lodMgr_.patchSize_ - 1;
    const int numPatchesPerSide = lodMgr_.numPatchesPerSide_;
    int       numVisPatches     = 0;


    // go through each patch and test if it is visible
    visiblePatches_.resize(SQR(numPatchesPerSide));

    for (int pz = 0; pz < numPatchesPerSide; ++pz)
    {
        for (int px = 0; px < numPatchesPerSide; ++px)
        {
            const int x = px * patchSize;
            const int z = pz * patchSize;

            DirectX::BoundingFrustum localSpaceFrustum;
            frustum.Transform(localSpaceFrustum, invView);

            // check if we see this patch
            if (!IsPatchInsideViewFrustum(x, z, localSpaceFrustum))
            {
                continue;
            }

            // store number (idx) of this patch so we will render it
            visiblePatches_[numVisPatches++] = (pz * numPatchesPerSide) + px;
        }
    }

    visiblePatches_.resize(numVisPatches);
}

//---------------------------------------------------------
// Desc:    (helper) add indices of vertices into the output idxs array
//          to make a new triangle
// Args:    - outIdxs:     output indices array to be filled with idxs
//          - outNumIdxs:  how many indices we currently have in the output indices arr
//          - i0, i1, i2:  indices of triangle's vertices
//---------------------------------------------------------
inline void AddTriangle(
    UINT* outIdxsBuf,
    int& outNumIdxs,
    const UINT i0,
    const UINT i1,
    const UINT i2)
{
    outIdxsBuf[outNumIdxs++] = i0;
    outIdxsBuf[outNumIdxs++] = i1;
    outIdxsBuf[outNumIdxs++] = i2;
}

inline int AddTriangle(
    int idx,
    cvector<UINT>& indices,
    const uint i0,
    const uint i1,
    const uint i2)
{
    indices[idx++] = i0;
    indices[idx++] = i1;
    indices[idx++] = i2;

    return idx;

}

//---------------------------------------------------------
// Desc:   setup axis-aligned bounding box (AABB) for this terrain's geometry
// Args:   - center:   the center of AABB
//         - extents:  a vector from the center to any vertex of AABB
//---------------------------------------------------------
void TerrainGeomip::SetAABB(const DirectX::XMFLOAT3& center, const DirectX::XMFLOAT3& extents)
{
    center_  = center;
    extents_ = extents;
}

//---------------------------------------------------------
// Desc:    setup material for terrain geometry (light props, textures, etc)
// Args:    - matId:   an identifier of material (for details look at MaterialMgr)
//---------------------------------------------------------
void TerrainGeomip::SetMaterial(const MaterialID matID)
{
    // setup the material ID for the terrain
    if (matID > 0)
    {
        materialID_ = matID;
    }
    else
    {
        LogErr("can't setup material ID because input ID == 0");
        return;
    }
}

//---------------------------------------------------------
// Desc:    setup a texture of type 
// Args:    - type:    a texture type (diffuse, normal map, etc.)
//          - texId:   a texture identifier (for detaild look at TextureMgr)
//---------------------------------------------------------
void TerrainGeomip::SetTexture(const eTexType type, const TexID texID)
{
    // set texture (its ID) by input idx

    if (texID == 0)
    {
        LogErr(LOG, "wrong input texture ID: %ld", texID);
        return;
    }

    // everything is ok so set the texture for terrain's material
    Material& mat = g_MaterialMgr.GetMatById(materialID_);

    // NOTE: we used slightly different approach of terrain types:
    // for instance ambient texture type can be used for detail map or something like that
    mat.SetTexture(type, texID);
}

//---------------------------------------------------------
// Desc:   calculate the summary number of indices for
//         a single patch for all its LODs
// Ret:    summary number of indices
//---------------------------------------------------------
int TerrainGeomip::CalcNumIndices()
{
    const int maxPermutationsPerLevel = 16;     // true/false for each of the four sides
    const int indicesPerQuad          = 6;      // two triangles
    int       numQuads                = SQR(lodMgr_.patchSize_ - 1);
    int       numIndices              = 0;

    SetConsoleColor(CYAN);

    for (int lod = 0; lod <= lodMgr_.maxLOD_; ++lod)
    {
        LogMsg("LOD %d: numQuads %d", lod, numQuads);
        numIndices += (numQuads * indicesPerQuad * maxPermutationsPerLevel);
        numQuads /= 4;
    }

    LogMsg("initial number of indices: %d", numIndices);
    SetConsoleColor(RESET);

    return numIndices;
}

//---------------------------------------------------------
// Desc:   initialize indices for each lod and each patch subtype
//         (for different neighbours)
// Args:   - indices:   array of indices to be filled
//---------------------------------------------------------
int TerrainGeomip::InitIndices(cvector<UINT>& indices)
{
    int idx = 0;

    SetConsoleColor(CYAN);

    for (int lod = 0; lod <= lodMgr_.maxLOD_; ++lod)
    {
        LogMsg("*** init indices lod %d ***", lod);
        idx = InitIndicesLOD(idx, indices, lod);
    }

    SetConsoleColor(RESET);
    return idx;
}

//---------------------------------------------------------
// Desc:   init indices for each patch type of the current lod
// Args:   - idx:      start index position in the buffer
//         - indices:  array of all the indices
//         - lod:      level of detail (can be: 0,1,2,etc.)
//---------------------------------------------------------
int TerrainGeomip::InitIndicesLOD(int idx, cvector<UINT>& indices, const int lod)
{
    int totalIdxsForLOD = 0;

    for (int l = 0; l < LEFT; ++l)
    {
        for (int r = 0; r < RIGHT; ++r)
        {
            for (int t = 0; t < TOP; ++t)
            {
                for (int b = 0; b < BOTTOM; ++b)
                {
                    SingleLodInfo& info = lodInfo_[lod].info[l][r][t][b];

                    // where we start if we want to use indices for this lod
                    info.start = idx;

                    // init indices for the current lod type
                    idx = InitIndicesLODSingle(idx, indices, lod, lod+l, lod+r, lod+t, lod+b);

                    // how many indices we have for this type of lod 
                    info.count = idx - info.start;

                    totalIdxsForLOD += info.count;
                }
            }
        }
    }

    SetConsoleColor(CYAN);
    LogMsg("Total indices for LOD: %d", totalIdxsForLOD);
    SetConsoleColor(RESET);

    return idx;
}

//---------------------------------------------------------
// Desc:   init indices for patch of the current type and LOD
// Args:   - idx:       start index position
//         - indices:   array of all indices
//         - lodCore:   the number of current LOD (can be: 0,1,2,etc.)
//         - lodLeft:   LOD number of the left neighbour's patch
//         - lodRight:  LOD number of the right neighbour's patch
//         - lodTop:    LOD number of the top neighbour's patch
//         - lodBottom: LOD number of the bottom neighbour's patch
// Ret:    index where the next subset of indices starts
//---------------------------------------------------------
int TerrainGeomip::InitIndicesLODSingle(
    int idx,
    cvector<UINT>& indices,
    const int lodCore,
    const int lodLeft,
    const int lodRight,
    const int lodTop,
    const int lodBottom)
{
    const int fanStep = (int)pow(2, lodCore+1);            // lod0 --> 2, lod1 --> 4, lod2 --> 8, etc.
    const int endPos  = lodMgr_.patchSize_ - 1 - fanStep;  // patch size 5, fan step 2 --> end pos = 2;
                                                           // patch size 9, fan step 2 --> end pos = 6

    for (int z = 0; z <= endPos; z += fanStep)
    {
        for (int x = 0; x <= endPos; x += fanStep)
        {
            // define types of neighbours LODs
            int lLeft   = (x == 0)      ? lodLeft   : lodCore;
            int lRight  = (x == endPos) ? lodRight  : lodCore;
            int lBottom = (z == 0)      ? lodBottom : lodCore;
            int lTop    = (z == endPos) ? lodTop    : lodCore;

            idx = CreateTriangleFan(idx, indices, lodCore, lLeft, lRight, lTop, lBottom, x, z);
        }
    }

    return idx;
}

//---------------------------------------------------------
// Desc:   init indices for fan at particular position
//         of the current patch type and LOD
// Args:   - idx:       start index position
//         - indices:   array of all indices
//         - lodCore:   the number of current LOD (can be: 0,1,2,etc.)
//         - lodLeft:   LOD number of the left neighbour's patch
//         - lodRight:  LOD number of the right neighbour's patch
//         - lodTop:    LOD number of the top neighbour's patch
//         - lodBottom: LOD number of the bottom neighbour's patch
//         - x, z:      position in 3d space
// Ret:    index where the next subset of indices starts
//---------------------------------------------------------
int TerrainGeomip::CreateTriangleFan(
    int idx,
    cvector<UINT>& indices,
    const int lodCore,
    const int lodLeft,
    const int lodRight,
    const int lodTop,
    const int lodBottom,
    const int x,
    const int z)
{
    // step ALONG side
    const int stepLeft    = powi(2, lodLeft);   // because LOD starts at zero...
    const int stepRight   = powi(2, lodRight);
    const int stepTop     = powi(2, lodTop);
    const int stepBottom  = powi(2, lodBottom);
    const int stepCenter  = powi(2, lodCore);   // how many vertices we need to step over to get the center of the fan

    const UINT terrainLen = (UINT)terrainLength_;

    // indices to make triangles
    const UINT idxCenter  = ((z + stepCenter) * terrainLen) + x + stepCenter;
    UINT i1 = 0;
    UINT i2 = 0;

    /*
        Fan visualization

        NOTE: Vertex by number in braces is optional according
              to LOD of its neighbor patch

        3 -------- (4) -------- 5
        | \         |         / |
        |    \      |      /    |
        |      \    |    /      |
        |         \ | /         |
       (2) -------- 0 -------- (6)
        |         / | \         |
        |      /    |    \      |
        |    /      |      \    |
        | /         |         \ |
        1 -------- (8) -------- 7

    */

    // first up
    // (case 1) add vertex 2 (so we have two triangles on the left side fan)
    // (case 2) we have no mid-left vertex
    // 
    //         3                3
    //         | \              | \
    //   (1)   2--0        (2)  |  0
    //         | /              | /
    //         1                1
    i1 = z * terrainLen + x;
    i2 = (z + stepLeft) * terrainLen + x;
    idx = AddTriangle(idx, indices, idxCenter, i1, i2);

    // second up
    if (lodLeft == lodCore)
    {
        i1 = i2;
        i2 += stepLeft * terrainLen;
        idx = AddTriangle(idx, indices, idxCenter, i1, i2);
    }

    // first right
    // (case 1) add vertex 4 (so we have two triangles on the upper side of the fan)
    // (case 2) we have no upper-mid vertex
    // 
    //         3--4--5             3-----5
    //   (1)    \ | /         (2)   \   /
    //            0                   0
    i1 = i2;
    i2 += stepTop;
    idx = AddTriangle(idx, indices, idxCenter, i1, i2);

    // second right
    // (case 1) add vertex 6 (so we have two triangles on the right side of the fan)
    // (case 2) we have no mid-right vertex
    //            5                   5
    //          / |                 / |
    //  (1)    0--6         (2)    0  |
    //          \ |                 \ |
    //            7                   7
    if (lodTop == lodCore)
    {
        i1 = i2;
        i2 += stepTop;
        idx = AddTriangle(idx, indices, idxCenter, i1, i2);
    }

    // first down
    i1 = i2;
    i2 -= stepRight * terrainLen;
    idx = AddTriangle(idx, indices, idxCenter, i1, i2);

    // second down
    if (lodRight == lodCore)
    {
        i1 = i2;
        i2 -= stepRight * terrainLen;
        idx = AddTriangle(idx, indices, idxCenter, i1, i2);
    }

    // first left
    // (case 1) add vertex 8 (so we have two triangles on the bottom side of the fan)
    // (case 2) we have no bottom-mid vertex
    //          0                   0
    //  (1)   / | \        (2)    /   \
    //       1--8--7             1-----7
    i1 = i2;
    i2 -= stepBottom;
    idx = AddTriangle(idx, indices, idxCenter, i1, i2);

    // second left
    if (lodBottom == lodCore)
    {
        i1 = i2;
        i2 -= stepBottom;
        idx = AddTriangle(idx, indices, idxCenter, i1, i2);
    }

    return idx;
}

//---------------------------------------------------------
// Desc:   fill in the vertex and index buffers with data
//---------------------------------------------------------
void TerrainGeomip::PopulateBuffers()
{
    // compute vertices
    numVertices_ = SQR(terrainLength_);
    cvector<Vertex3dTerrain> vertices(numVertices_);

    LogMsg(LOG, "preparing size for %d vertices", numVertices_);
    InitVertices(vertices.data(), numVertices_);

    // compute indices
    numIndices_ = CalcNumIndices();
    cvector<UINT> indices(numIndices_);
    
    numIndices_ = InitIndices(indices);
    LogMsg(LOG, "final number of indices %d", numIndices_);


    // compute normal vector of each terrain's vertex
    CalcNormals(vertices.data(), indices.data(), numVertices_, numIndices_);

    // create GPU-side vertex and index buffers
    InitBuffers(vertices.data(), indices.data(), numVertices_, numIndices_);
}

} // namespace
