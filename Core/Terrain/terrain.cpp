// =================================================================================
// Filename:  Terrain.cpp
// Desc:      implementation of terrain geomimapping CLOD algorithm
//            (CLOD - continuous level of detail)
//
// Created:   10.06.2025 by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include <pack_color.h>
#include "terrain.h"
#include "../Mesh/material_mgr.h"
#include <Render/d3dclass.h>      // for using global pointers to DX11 device and context

#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <Render/debug_draw_manager.h>


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
void Terrain::ReleaseBuffers()
{
    vertices_.purge();
    indices_.purge();

    // release memory from the vertex buffer and index buffer
    vb_.Shutdown();
    ib_.Shutdown();
}

//---------------------------------------------------------
// Desc:   release all the memory related to the terrain
//---------------------------------------------------------
void Terrain::Shutdown()
{
    vertices_.purge();
    indices_.purge();

    ReleaseBuffers();
    ClearMemoryFromMaps();
}

//---------------------------------------------------------
// Desc:   initiate the geomipmapping system
// Args:   - patchSize:  the size of the patch (in vertices)
//                       a good size is usually around 17 (17x17 verts)
//---------------------------------------------------------
bool Terrain::InitGeomipmapping(const int patchSize)
{
    int divisor = 0;
    int LOD = 0;

    // since terrain is squared the width and height are equal
    const int terrainLen = GetTerrainLength();

    LogMsg(LOG, "start initialization of terrain geometry and buffers (geomip)");

    // check if we actually able to properly initialize the terrain's geometry 
    if (((terrainLen - 1) % (patchSize - 1)) != 0)
    {
        int patchSz = patchSize - 1;
        int recommendedTerrainSize = ((terrainLen -1 + patchSz) / (patchSz)) * (patchSz) + 1;
        
        LogErr(LOG, "terrain length minus 1 (%d) must be divisible by patch size minus 1 (%d)", terrainLen, patchSize);
        SetConsoleColor(YELLOW);
        LogErr(LOG, "Try using terrain size = %d", recommendedTerrainSize);
        
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


    // alloc containers for indices of terrain patches (which will be splitted by LODs)
    const int numAllPatches = SQR(numPatchesPerSide);
    visiblePatches_.resize(numAllPatches);
    highDetailedPatches_.resize(numAllPatches);
    midDetailedPatches_.resize(numAllPatches);
    lowDetailedPatches_.resize(numAllPatches);

    // init vertex/index buffers
    PopulateBuffers();

    ComputeBoundings();

    LogMsg("Geomipmapping system successfully initialized");
    return true;
}

//---------------------------------------------------------
// Desc:  compute boudning sphere and AABB around each terrain's patch (sector);
//        also add AABB for rendering as "debug shape"
//---------------------------------------------------------
void Terrain::ComputeBoundings()
{
    const int patchSize         = lodMgr_.patchSize_ - 1;
    const int numPatchesPerSide = lodMgr_.numPatchesPerSide_;
    int idx = 0;

    const int numAllPatches = SQR(numPatchesPerSide);
    patchesAABBs_.resize(numAllPatches);
    patchesBoundSpheres_.resize(numAllPatches);

    const Vec3 colorYellow(1, 1, 0);


    for (int pz = 0; pz < numPatchesPerSide; ++pz)
    {
        for (int px = 0; px < numPatchesPerSide; ++px)
        {
            const int x0             = px * patchSize;
            const int z0             = pz * patchSize;

            const int size           = lodMgr_.patchSize_ - 1;
            const int x1             = x0 + size;
            const int z1             = z0 + size;

            const float h00          = GetScaledHeightAtPoint(x0, z0);
            const float h10          = GetScaledHeightAtPoint(x1, z0);
            const float h01          = GetScaledHeightAtPoint(x0, z1);
            const float h11          = GetScaledHeightAtPoint(x1, z1);

            const float minHeight    = min(h00, min(h01, min(h10, h11)));
            const float maxHeight    = max(h00, max(h01, max(h10, h11)));

            const Vec3 minPoint      = { (float)x0, minHeight, (float)z0 };
            const Vec3 maxPoint      = { (float)x1, maxHeight, (float)z1 };

            const Vec3 center        = (maxPoint + minPoint) * 0.5f;
            const Vec3 centerToPoint = (maxPoint - center);
            const float radius       = Vec3Length(centerToPoint);

            const Rect3d aabb(minPoint.x, maxPoint.x, minPoint.y, maxPoint.y, minPoint.z, maxPoint.z);
            const Sphere sphere(center, radius);

            patchesAABBs_[idx] = aabb;
            patchesBoundSpheres_[idx] = sphere;

            const Vec3 aabbMinPoint = aabb.MinPoint();
            const Vec3 aabbMaxPoint = aabb.MaxPoint();

            idx++;
        }   
    }
}

//---------------------------------------------------------
// Desc:   initialize DirectX vertex/index buffers
//---------------------------------------------------------
bool Terrain::InitBuffers(
    const Vertex3dTerrain* vertices,
    const UINT* indices,
    const int numVertices,
    const int numIndices)
{
    constexpr bool isDynamicBuf = false;

    if (!vb_.Init(vertices, numVertices, isDynamicBuf))
    {
        LogErr(LOG, "can't init a vertex buffer for terrain");
        Shutdown();
        return false;
    }

    if (!ib_.Init(indices, numIndices, isDynamicBuf))
    {
        LogErr(LOG, "can't init an index buffer for terrain");
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
void Terrain::InitVertices(Vertex3dTerrain* vertices, const int numVertices)
{
    int idx = 0;
    const int   terrainLen    = GetTerrainLength();
    const float invTerrainLen = 1.0f / (float)terrainLen;

    // no smoothing computations
    if constexpr (TERRAIN_SMOOTHING_LEVEL == 0)
    {
        for (int z = 0; z < terrainLen; z++)
        {
            for (int x = 0; x < terrainLen; x++)
            {
                assert(idx < numVertices);

                const float fX = (float)x;
                const float fZ = (float)z;
                
                vertices[idx] = {
                    fX,                                 // posX
                    GetScaledHeightAtPoint(x, z),       // posY
                    fZ,                                 // posZ
                    fX * invTerrainLen,                 // texU
                    fZ * invTerrainLen                  // texV
                };

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
// Desc:   calculate a normal vector for each terrain's vertex
// Args:   - vertices:  arr of terrain's vertices
//         - indices:   arr of terrain's indices
//         - numVertices:  how many vertices in the vertices arr
//---------------------------------------------------------
void Terrain::CalcNormals(
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
//---------------------------------------------------------
bool Terrain::IsPatchInsideViewFrustum(const int patchIdx, const Frustum& frustum)
{
    return frustum.TestSphere(patchesBoundSpheres_[patchIdx]);
}

//---------------------------------------------------------
// Desc:  calculate a normal vector by 3 input positions
//---------------------------------------------------------
Vec3 CompNormalVec(const XMFLOAT3& p0, const XMFLOAT3& p1, const XMFLOAT3& p2)
{
    XMVECTOR pos0 = XMLoadFloat3(&p0);
    XMVECTOR pos1 = XMLoadFloat3(&p1);
    XMVECTOR pos2 = XMLoadFloat3(&p2);

    XMVECTOR e0 = pos1 - pos0;
    XMVECTOR e1 = pos2 - pos0;

    XMVECTOR n = XMVector3Normalize(XMVector3Cross(e1, e0));

    DirectX::XMFLOAT3 v;
    DirectX::XMStoreFloat3(&v, n);

    return Vec3(v.x, v.y, v.z);
}

//---------------------------------------------------------
// Desc:  execute ray/terrain intersection test
// Args:  - rayOrig:  origin of the ray
//        - rayDir:   direction of the ray
// 
// Out:   - outData:  data about intersection (if we have any)
// Ret:   true if we have any intersection
//---------------------------------------------------------
bool Terrain::TestRayIntersection(
    const Vec3& rayOrig,
    const Vec3& rayDir,
    IntersectionData& outData) const
{
    // ray direction must be normalized
    assert(FloatEqual(Vec3Length(rayDir), 1.0f) == true);

    //
    // ray/bounding_spheres tests (broad phase)
    //
    cvector<int> patchesToTest;
    patchesToTest.reserve(16);

    float dist = 0;

    // test only visible terrain patches (sectors)
    for (const int i : GetAllVisiblePatches())
    {
        if (!IntersectRaySphere(patchesBoundSpheres_[i], rayOrig, rayDir, dist))
            continue;

        // ray intersects this sphere
        patchesToTest.push_back(i);
    }


    // ray/AABB test
    //if (patchesAABBs_)


    //
    // ray/triangle tests for each terrain patch (narrow phase)
    //
    const int   terrainLen           = GetTerrainLength();
    const int   numPatchesPerSide    = GetNumPatchesPerSide();
    const float invNumPatchesPerSide = 1.0f / numPatchesPerSide;
    const int   patchSize            = lodMgr_.GetPatchSize();

    const XMVECTOR rayOrigW = { rayOrig.x, rayOrig.y, rayOrig.z };
    const XMVECTOR rayDirW  = { rayDir.x, rayDir.y, rayDir.z };

    float tmin = FLT_MAX;
    int triangleIdx = 0;
    bool bIntersect = false;
    int intersectedPatchIdx = -1;
    XMVECTOR p0, p1, p2;         // intersected triangle

    for (const int patchIdx : patchesToTest)
    {
        // set maximal lod (detalization for the geometry)
        TerrainLodMgr::PatchLod plod = {0,0,0,0,0};

        UINT baseIndex = 0;
        UINT indexCount = 0;
        GetLodInfoByPatch(plod, baseIndex, indexCount);

        const int patchZ = (int)(patchIdx * invNumPatchesPerSide);  // const int patchZ = idx / numPatchesPerSide;
        const int patchX = patchIdx & (numPatchesPerSide - 1);      // const int patchX = idx % numPatchesPerSide;
        const int z      = patchZ * (patchSize - 1);
        const int x      = patchX * (patchSize - 1);

        const UINT baseVertex = (UINT)(z * terrainLen + x);

        const Vertex3dTerrain* verts = &vertices_[baseVertex];
        const UINT*          indices = &indices_[baseIndex];

        // exec ray/triangle tests for this patch (its mesh)
        for (int i = 0; i < (int)indexCount / 3; ++i)
        {
            // indices for this triangle
            const UINT i0 = indices[i*3 + 0];
            const UINT i1 = indices[i*3 + 1];
            const UINT i2 = indices[i*3 + 2];

            // vertices of this triangle
            const XMVECTOR v0 = XMLoadFloat3(&verts[i0].position);
            const XMVECTOR v1 = XMLoadFloat3(&verts[i1].position);
            const XMVECTOR v2 = XMLoadFloat3(&verts[i2].position);

            float t = 0;

            if (!DirectX::TriangleTests::Intersects(rayOrigW, rayDirW, v0, v1, v2, t))
                continue;

            if (t > tmin)
                continue;

            // this is a new nearest intersected triangle
            tmin = t;
            triangleIdx = i;
            p0 = v0;
            p1 = v1;
            p2 = v2;
            intersectedPatchIdx = patchIdx;
            bIntersect = true;
        }
    }

    if (!bIntersect)
        return false;

    // store endpoints of the intersected triangle
    XMFLOAT3 pos0, pos1, pos2;

    XMStoreFloat3(&pos0, p0);
    XMStoreFloat3(&pos1, p1);
    XMStoreFloat3(&pos2, p2);

    outData.vx0 = pos0.x;
    outData.vy0 = pos0.y;
    outData.vz0 = pos0.z;

    outData.vx1 = pos1.x;
    outData.vy1 = pos1.y;
    outData.vz1 = pos1.z;

    outData.vx2 = pos2.x;
    outData.vy2 = pos2.y;
    outData.vz2 = pos2.z;


    // store ray origin
    outData.rayOrigX = rayOrig.x;
    outData.rayOrigY = rayOrig.y;
    outData.rayOrigZ = rayOrig.z;

    // store intersection point
    const XMVECTOR vIntersect = rayOrigW + rayDirW * tmin;
    XMFLOAT3 intersectP;
    XMStoreFloat3(&intersectP, vIntersect);

    outData.px = intersectP.x;
    outData.py = intersectP.y;
    outData.pz = intersectP.z;

    // store a distance to the intersection point
    outData.distToIntersect = tmin;

    // store normal vec of intersected triangle
    Vec3 normal = CompNormalVec(pos0, pos1, pos2);

    if (Vec3Dot(normal, rayDir) > 0)
        normal = -normal;

    outData.nx = normal.x;
    outData.ny = normal.y;
    outData.nz = normal.z;


    return true;
}

//---------------------------------------------------------
// Desc:   update the geomipmapping system
// Args:   - camParams:     camera params for LODs computation
//         - worldFrustum:  frustum in a world space
//         - distFogged:    after this distance all the terrain patches will be
//                          completely fogged (so we set them as low detailed)
//---------------------------------------------------------
void Terrain::Update(
    const CameraParams& cam,
    const Frustum& worldFrustum,
    const float distFogged)
{
    const int patchSize         = lodMgr_.patchSize_ - 1;
    const int numPatchesPerSide = lodMgr_.numPatchesPerSide_;
    int       numVisPatches     = 0;


    // go through each patch and test if it is visible
    int idx = -1;

    for (int pz = 0; pz < numPatchesPerSide; ++pz)
    {
        for (int px = 0; px < numPatchesPerSide; ++px)
        {
            ++idx;

            // check if we see this patch
            if (!IsPatchInsideViewFrustum(idx, worldFrustum))
                continue;

            // store number (idx) of this patch so we will render it
            visiblePatches_[numVisPatches++] = (pz * numPatchesPerSide) + px;
        }
    }

    visiblePatches_.resize(numVisPatches);
    highDetailedPatches_.resize(numVisPatches);
    midDetailedPatches_.resize(numVisPatches);
    lowDetailedPatches_.resize(numVisPatches);

    // update LOD info for each visible patch
    lodMgr_.Update(
        cam.posX,
        cam.posY,
        cam.posZ,
        distFogged,
        visiblePatches_,
        highDetailedPatches_,
        midDetailedPatches_,
        lowDetailedPatches_);
}

//---------------------------------------------------------
// Desc:   compute axis-aligned bounding box (AABB) for the whole terrain
// NOTE:   we use bounding of each terrain's patch for computation
//---------------------------------------------------------
void Terrain::CalcAABB(void)
{
    float minY = FLT_MAX;
    float maxY = FLT_MIN;

    // for through each terrain's patch and define
    // min and max height of the whole terrain
    for (index i = 0; i < patchesAABBs_.size(); ++i)
    {
        const Rect3d& box = patchesAABBs_[i];

        minY = Min(box.y0, minY);
        maxY = Max(box.y1, maxY);
    }

    const float minX = patchesAABBs_[0].x0;
    const float minZ = patchesAABBs_[0].z0;
    const float maxX = patchesAABBs_.back().x1;
    const float maxZ = patchesAABBs_.back().z1;

    aabb_ = { minX, maxX, minY, maxY, minZ, maxZ };
}

//---------------------------------------------------------
// Desc:    setup material for terrain geometry (light props, textures, etc)
// Args:    - matId:   an identifier of material (for details look at MaterialMgr)
//---------------------------------------------------------
void Terrain::SetMaterial(const MaterialID matID)
{
    if (matID <= 0)
    {
        LogErr(LOG, "can't setup material ID because input ID == 0");
        return;
    }

    materialID_ = matID;
}

//---------------------------------------------------------
// Desc:    setup a texture of type 
// Args:    - type:    a texture type (diffuse, normal map, etc.)
//          - texId:   a texture identifier (for detaild look at TextureMgr)
//---------------------------------------------------------
void Terrain::SetTexture(const eTexType type, const TexID texID)
{
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
int Terrain::CalcNumIndices()
{
    const int maxPermutationsPerLevel = 16;     // true/false for each of the four neighbour sides
    const int indicesPerQuad          = 6;      // two triangles
    int       numQuads                = SQR(lodMgr_.patchSize_ - 1);
    int       numIndices              = 0;

    SetConsoleColor(CYAN);

    for (int lod = 0; lod <= lodMgr_.maxLOD_; ++lod)
    {
        LogMsg("LOD %d: numQuads %d", lod, numQuads);
        numIndices += (numQuads * indicesPerQuad * maxPermutationsPerLevel);

        // for each LOD the number quads reduces 4 times from the prev LOD
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
int Terrain::InitIndices(cvector<UINT>& indices)
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
int Terrain::InitIndicesLOD(int idx, cvector<UINT>& indices, const int lod)
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
int Terrain::InitIndicesLODSingle(
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
// Desc:    (helper) add indices of vertices into the output idxs array
//          to make a new triangle
// Args:    - outIdxs:     output indices array to be filled with idxs
//          - outNumIdxs:  how many indices we currently have in the output indices arr
//          - i0, i1, i2:  indices of triangle's vertices
//---------------------------------------------------------
inline void AddTriangle(UINT* outIdxsBuf, int& outNumIdxs, UINT i0, UINT i1, UINT i2)
{
    outIdxsBuf[outNumIdxs + 0] = i0;
    outIdxsBuf[outNumIdxs + 1] = i1;
    outIdxsBuf[outNumIdxs + 2] = i2;

    outNumIdxs += 3;
}

inline int AddTriangle(int idx, cvector<UINT>& indices, uint i0, uint i1, uint i2)
{
    indices[idx + 0] = i0;
    indices[idx + 1] = i1;
    indices[idx + 2] = i2;

    return idx + 3;
}

//---------------------------------------------------------
// Desc:   init indices for fan at particular position
//         of the current patch type and LOD
// Ret:    index where the next subset of indices starts
//---------------------------------------------------------
int Terrain::CreateTriangleFan(
    int idx,                           // start index position
    cvector<UINT>& indices,            // array of all indices
    const int lodCore,                 // the number of current LOD (can be: 0,1,2,etc.)
    const int lodLeft,                 // LOD number of the left neighbour's patch
    const int lodRight,                // LOD number of the right neighbour's patch
    const int lodTop,                  // LOD number of the top neighbour's patch
    const int lodBottom,               // LOD number of the bottom neighbour's patch
    const int x,                       // position in 3d space
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
void Terrain::PopulateBuffers()
{
    // compute vertices
    numVertices_ = SQR(terrainLength_);
    LogMsg(LOG, "preparing size for %d vertices", numVertices_);
    vertices_.resize(numVertices_);
    InitVertices(vertices_.data(), numVertices_);

    // compute indices
    numIndices_ = CalcNumIndices();
    indices_.resize(numIndices_);
    numIndices_ = InitIndices(indices_);
    LogMsg(LOG, "final number of indices %d", numIndices_);

    // compute normal vector of each terrain's vertex
    CalcNormals(vertices_.data(), indices_.data(), numVertices_, numIndices_);

    // create GPU-side vertex and index buffers
    InitBuffers(vertices_.data(), indices_.data(), numVertices_, numIndices_);
}

} // namespace
