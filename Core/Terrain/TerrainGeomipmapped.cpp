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
#include "../Render/d3dclass.h"

#include <DirectXMath.h>
#include <DirectXCollision.h>


using namespace DirectX;


namespace Core
{

// yes, I did it
#define NEW new (std::nothrow)


//---------------------------------------------------------
// Desc:   release memory from the CPU copy of vertices/indices
//---------------------------------------------------------
void TerrainGeomip::ClearMemory()
{

    SafeDeleteArr(vertices_);
    SafeDeleteArr(indices_);
}

//---------------------------------------------------------
// Desc:   release all the memory related to the terrain
//---------------------------------------------------------
void TerrainGeomip::Shutdown()
{
    ClearMemory();

    // delete the patch number
    SafeDeleteArr(patches_);

    // reset patch values
    patchSize_          = 0;
    numPatchesPerSide_  = 0;
    maxLOD_             = 0;

    // release memory from the vertex buffer and index buffer
    vb_.Shutdown();
    ib_.Shutdown();
}

//---------------------------------------------------------
// Desc:   allocate memory for vertices/indices data arrays
// Args:   - numVertices:  the number of vertices
//         - numIndices:   the number of indices
// Ret:    true if we managed to allocate memory
//---------------------------------------------------------
bool TerrainGeomip::AllocateMemory(const int numVertices, const int numIndices)
{
    assert(numVertices > 0);
    assert(numIndices > 0);

    // prepare memory
    ClearMemory();

    numVertices_ = numVertices;
    numIndices_  = numIndices;

    // alloc vertices array
    vertices_ = NEW Vertex3dTerrain[numVertices_]{};
    if (!vertices_)
    {
        LogErr(LOG, "can't allocate memory for array of terrain's vertices");
        Shutdown();
        return false;
    }

    // alloc indices array
    indices_ = NEW UINT[numIndices_];
    if (!indices_)
    {
        LogErr(LOG, "can't allocate memory for array of terrain's indices");
        Shutdown();
        return false;
    }

    return true;
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
    const int terrainSize = heightMap_.GetWidth();


    if ((terrainSize - 1) % (patchSize - 1) != 0)
    {
        int patchSz = patchSize - 1;
        int recommendedTerrainSize = ((terrainSize-1 + patchSz) / (patchSz)) * (patchSz) + 1;
        LogErr(LOG, "terrain size minus 1 (%d) must be divisible by patch size minus 1 (%d)", terrainSize, patchSize);
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

    // release all the memory from prev patches data
    if (patches_)
        Shutdown();

    // initiate the patch info
    patchSize_         = patchSize;
    numPatchesPerSide_ = terrainSize / patchSize;

    numAllPatches      = numPatchesPerSide_ * numPatchesPerSide_;

    patches_ = NEW GeomPatch[numAllPatches];
    if (!patches_)
    {
        LogErr(LOG, "can't allocate memory for terrain's patches");
        return false;
    }

    // figure out the max level of detail for the patch
    divisor = patchSize - 1;

    while (divisor > 2)
    {
        divisor = divisor >> 1;
        LOD++;
    }

    // the max amount of detail
    maxLOD_ = LOD;

    // init the patch values
    for (int i = 0; i < numAllPatches; ++i)
    {
        // init the patches to the lowest level of detail
        patches_[i].LOD = LOD;
        patches_[i].isVisible = false;
    }

    // initialize vertex/index buffers
    if (!InitBuffers())
    {
        LogErr(LOG, "can't initialize vb/ib buffers for the geomip terrain");
        return false;
    }

    LogMsg("Geomipmapping system successfully initialized");
    return true;
}

//---------------------------------------------------------
// Desc:   initialize DirectX vertex/index buffers
//---------------------------------------------------------
bool TerrainGeomip::InitBuffers()
{
    constexpr bool isDynamic = true;

    // initialize the vertex buffer
    if (!vb_.Initialize(g_pDevice, vertices_, numVertices_, isDynamic))
    {
        LogErr(LOG, "can't initialize a vertex buffer for terrain");
        Shutdown();
        return false;
    }

    // initialize the index buffer
    if (!ib_.Initialize(g_pDevice, indices_, numIndices_, isDynamic))
    {
        LogErr(LOG, "can't initialize an index buffer for terrain");
        Shutdown();
        return false;
    }

    return true;
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
// Desc:   compute normal vector by 3 input vertices
// Args:   - v0, v1, v2:  vertices
//---------------------------------------------------------
void ComputeFaceNormal(
    Vertex3dTerrain& v0,
    Vertex3dTerrain& v1,
    Vertex3dTerrain& v2)
{
    using namespace DirectX;

    // compute face normal for the first triangle
    XMVECTOR pos0 = XMLoadFloat3(&v0.position);
    XMVECTOR pos1 = XMLoadFloat3(&v1.position);
    XMVECTOR pos2 = XMLoadFloat3(&v2.position);

    XMVECTOR e0 = pos1 - pos0;
    XMVECTOR e1 = pos2 - pos0;

    XMVECTOR normal = XMVector3Normalize(XMVector3Cross(e0, e1));

    XMStoreFloat3(&v0.normal, normal);
    XMStoreFloat3(&v1.normal, normal);
    XMStoreFloat3(&v2.normal, normal);
}

void TerrainGeomip::CalcNormals(
    Vertex3dTerrain* vertices,
    const UINT* indices,
    const int numVertices,
    const int numIndices)
{
    const int depth = terrainDepth_;
    const int width = terrainWidth_;
    const int patchSz = patchSize_ - 1;

    UINT idx = 0;

#if 0
    // accumulate each triangle triangle normal into each of the triangle vertices
    for (int z = 0; z < depth - 1; z += (patchSz - 1))
    {
        for (int x = 0; x < width - 1; x += (patchSz - 1))
        {
            const int baseVertexIdx = z * width + x;

            // for each triangle
            for (int i = 0; i < numIndices; i += 3)
            {
                const UINT i0         = baseVertexIdx + indices[i + 0];
                const UINT i1         = baseVertexIdx + indices[i + 1];
                const UINT i2         = baseVertexIdx + indices[i + 2];

                const XMVECTOR pos0   = XMLoadFloat3(&vertices[i0].position);
                const XMVECTOR pos1   = XMLoadFloat3(&vertices[i1].position);
                const XMVECTOR pos2   = XMLoadFloat3(&vertices[i2].position);

                const XMVECTOR normal = XMVector3Cross(pos1 - pos0, pos2 - pos0);

                //XMFLOAT3 n = { 0,0,0 };
                //XMStoreFloat3(&n, normal);

                vertices[i0].normal += normal;
                vertices[i1].normal += normal;
                vertices[i2].normal += normal;
            }
        }
    }
#endif

    // for each triangle
    for (int i = 0; i < numIndices; i += 3)
    {
        const UINT i0 = indices[i + 0];
        const UINT i1 = indices[i + 1];
        const UINT i2 = indices[i + 2];

        const XMVECTOR pos0 = XMLoadFloat3(&vertices[i0].position);
        const XMVECTOR pos1 = XMLoadFloat3(&vertices[i1].position);
        const XMVECTOR pos2 = XMLoadFloat3(&vertices[i2].position);

        const XMVECTOR normal = XMVector3Cross(pos1 - pos0, pos2 - pos0);

        //XMFLOAT3 n = { 0,0,0 };
        //XMStoreFloat3(&n, normal);

        vertices[i0].normal += normal;
        vertices[i1].normal += normal;
        vertices[i2].normal += normal;
    }

    // normalize all the vertex normals
    for (int i = 0; i < numVertices; ++i)
        XMFloat3Normalize(vertices[i].normal);
}

//---------------------------------------------------------
// Desc:   update the geomipmapping system
// Args:   - cameraPos: camera position in the world
//---------------------------------------------------------
void TerrainGeomip::Update(const CameraParams& camParams)
{
    const int camPosX = (int)camParams.posX;
    const int camPosY = (int)camParams.posY;
    const int camPosZ = (int)camParams.posZ;

    const int patchSize     = patchSize_;
    const int halfPatchSize = patchSize / 2;
    const float fHalfPatchSz = (float)halfPatchSize;

    // BAD way to determine patch LOD
    constexpr int nearDist      = 100;
    constexpr int midDist       = 150;
    constexpr int farDist       = 200;
    constexpr int nearDistSqr   = nearDist * nearDist;
    constexpr int midDistSqr    = midDist * midDist;
    constexpr int farDistSqr    = farDist * farDist;

    // radius of each patch
    const float radius = sqrtf(SQR(fHalfPatchSz) + SQR(fHalfPatchSz) + SQR(fHalfPatchSz));
    

    // compute frustum by input camera's params
    Frustum frustum;

    frustum.Initialize(
        camParams.planes[0],
        camParams.planes[1],
        camParams.planes[2],
        camParams.planes[3],
        camParams.planes[4],
        camParams.planes[5]);


    const XMMATRIX translateView = DirectX::XMMatrixTranslation(0, 0, +10);
    const XMMATRIX view          = (XMMATRIX(camParams.viewMatrix) * translateView);

    const int numPerSide         = numPatchesPerSide_;

    // go through each patch and define if it is visible and
    // if so compute the squared distance to it 
    for (int z = 0; z < numPerSide; ++z)
    {
        // patch center by Z-axis
        const int pz = (z * patchSize) + halfPatchSize;

        for (int x = 0; x < numPerSide; ++x)
        {
            // compute patch center
            const int  px    = (x * patchSize) + halfPatchSize;
            const int  py    = (int)GetScaledHeightAtPoint(px, pz);
            GeomPatch& patch = patches_[GetPatchNumber(x, z)];

            // transform the patch center from world space into camera (view) space
            DirectX::XMVECTOR center = { (float)px, (float)py, (float)pz };
            center = DirectX::XMVector3Transform(center, view);
            DirectX::XMFLOAT3 c;                
            DirectX::XMStoreFloat3(&c, center);

            //---------------------------------------------
            // cull non-visible patches

            // first of all we execute frustum/sphere test because of simple computations
            patch.isVisible = frustum.SphereTest(c.x, c.y, c.z, radius);

#if 1
            // if we passed frustum/sphere test then we execute frustum/cube
            // test for higher precision
            if (patch.isVisible)
            {
                bool cubeTest = frustum.CubeTest(c.x, c.y, c.z, fHalfPatchSz);
                bool weInPatch = TestWeInPatch(camPosX, camPosZ, px, pz, halfPatchSize);

                patch.isVisible = cubeTest || weInPatch;
            }
            // we don't see this patch so just move to the next patch computation
            else
            {
                continue;
            }
#endif

            if (patch.isVisible)
            {
                // get the square of the distance from the camera to the patch
                patch.distSqr = (SQR(px-camPosX) + SQR(py-camPosY) + SQR(pz-camPosZ));

                const int distSqr = patch.distSqr;

                // if we see this patch then compute its LOD
                patch.LOD =                                                    // LOD_0: by default (when distSqr < nearDistSqr)
                    1 * ((distSqr >= nearDistSqr) & (distSqr < midDistSqr)) +  // LOD_1: distSqr in range [nearDistSqr, midDistSqr)
                    2 * ((distSqr >= midDistSqr)  & (distSqr < farDistSqr)) +  // LOD_2: distSqr in range [midDistSqr, farDistSqr)
                    3 *  (distSqr >= farDistSqr);                              // LOD_3: distSqr is >= farDistSqr
            }
        }
    }

    // recompute the geometry for the terrain
    ComputeTesselation();

    // compute normal vectors
    CalcNormals(vertices_, indices_, verticesOffset_, indicesOffset_);
}

//---------------------------------------------------------
// Desc:   compute geometry for each terrain's patch
//         according to its LOD
//---------------------------------------------------------
void TerrainGeomip::ComputeTesselation(void)
{
    // reset the counting variables
    verticesOffset_  = 0;
    indicesOffset_   = 0;
    patchesPerFrame_ = 0;
    vertsPerFrame_   = 0;
    trisPerFrame_    = 0;

    const int numPerSide = numPatchesPerSide_;

    // go through each patch
    for (int z = 0; z < numPerSide; ++z)
    {
        for (int x = 0; x < numPerSide; ++x)
        {
            int num = GetPatchNumber(x, z);

            if (patches_[num].isVisible)
            {
                ComputePatch(num, x, z);
                patchesPerFrame_++;
            }
        }
    }
}

//---------------------------------------------------------
// Desc:   compute vertices for this patch
// Args:   - currPatchNum: the number of current patch
//         - px: patch index by X-axis
//         - pz: patch index by Z-axis
//---------------------------------------------------------
void TerrainGeomip::ComputePatch(
    const int currPatchNum,
    const int px,
    const int pz)
{
    GeomNeighbor patchNeighbor;
    GeomNeighbor fanNeighbor;

    uint32 LOD = patches_[currPatchNum].LOD;

    int patchNumLeft  = GetPatchNumber(px-1, pz);
    int patchNumUp    = GetPatchNumber(px,   pz+1);
    int patchNumRight = GetPatchNumber(px+1, pz);
    int patchNumDown  = GetPatchNumber(px,   pz-1);

    // find out info about the patch to the current patch's left, if the patch is of
    // a greater detail or there is no patch to the left, we cant render the
    
    // the left patch
    patchNeighbor.left  = (patches_[patchNumLeft].LOD <= LOD || px == 0);

    // the upper patch
    patchNeighbor.up    = (patches_[patchNumUp].LOD <= LOD || pz == numPatchesPerSide_);

    // the right patch
    patchNeighbor.right = (patches_[patchNumRight].LOD <= LOD || px == numPatchesPerSide_);

    // lower patch
    patchNeighbor.down  = (patches_[patchNumDown].LOD <= LOD || pz == 0);


    // determine the distance btw each triangle-fan that we will be rendering
    const int   iPatchSize = patchSize_;
    const float fPatchSize = (float)iPatchSize;

    // find out how many fan divisions we are going to have
    const int   divisor    = (iPatchSize - 1) >> (LOD + 1);

    // the size btw the center of each triangle fan (for instance: LOD_3 --> divisor = 1, size = 17, halfSize = 8)
    const float size       = fPatchSize / divisor;
    
    // half the size btw the center of each triangle fan (this will be the size btw each vertex)
    const float halfSize   = size * 0.5f;
   

    // go through each subpatch
    for (float z = halfSize; (z + halfSize) < fPatchSize + 1; z += size)
    {

        // if this fan is in the bottom row, we may need to adjust it's rendering
        // to prevent cracks
        if (z == halfSize)
            fanNeighbor.down = patchNeighbor.down;
        else
            fanNeighbor.down = true;

        // if this fan is in the top row, we may need to adjust it's rendering
        // to prevent cracks
        if (z >= (fPatchSize - halfSize))
            fanNeighbor.up = patchNeighbor.up;
        else
            fanNeighbor.up = true;

        const float patchCenterZ = pz * fPatchSize + z;


        for (float x = halfSize; (x + halfSize) < fPatchSize + 1; x += size)
        {
            // if this fan is in the left row, we may need to adjust it's rendering
            // to prevent cracks
            if (x == halfSize)
                fanNeighbor.left = patchNeighbor.left;
            else
                fanNeighbor.left = true;

            // if this fan is in the right row, we may need to adjust it's rendering
            // to prevent cracks
            if (x >= (fPatchSize - halfSize))
                fanNeighbor.right = patchNeighbor.right;
            else
                fanNeighbor.right = true;


            // compute the triangle fan
            ComputeFan(px*fPatchSize + x, patchCenterZ, size, fanNeighbor);
        }
    }
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

//---------------------------------------------------------
// Desc:   compute a single fan geometry
// Args:   - cx, cz:   center of the triangle fan to render
//         - size:     the fan's entire size
//         - neighbor: the fan's neighbor structure (used to avoid cracking)
//---------------------------------------------------------
void TerrainGeomip::ComputeFan(
    const float cx,
    const float cz,
    const float size,
    const GeomNeighbor& neighbor)
{
    const float halfSize       = size * 0.5f;
    const float invTerrainSize = 1.0f / heightMap_.GetWidth();

    // vertices positions
    const float fLeft     = cx - halfSize;
    const float fRight    = cx + halfSize;
    const float fDown     = cz - halfSize;
    const float fUp       = cz + halfSize;

    // calc the texture coords
    const float texLeft   = fabsf(fLeft)  * invTerrainSize;
    const float texRight  = fabsf(fRight) * invTerrainSize;
    const float texBottom = fabsf(fDown)  * invTerrainSize;
    const float texTop    = fabsf(fUp)    * invTerrainSize;

    const float midX      = (texLeft + texRight) * 0.5f;
    const float midZ      = (texBottom + texTop) * 0.5f;


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

    float height = 0;
    Vertex3dTerrain vertices[9];

    // CENTER vertex (0)
    height               = GetScaledInterpolatedHeightAtPoint(cx, cz);
    vertices[0].position = { cx, height, cz };
    vertices[0].texture  = { midX, midZ };

    // LOWER-LEFT vertex (1)
    height               = GetScaledInterpolatedHeightAtPoint(fLeft, fDown);
    vertices[1].position = { fLeft, height, fDown};
    vertices[1].texture  = { texLeft, texBottom };

    // MID-LEFT vertex (2): use this vertex if the left patch is NOT of a lower LOD
    height               = GetScaledInterpolatedHeightAtPoint(fLeft, cz);
    vertices[2].position = { fLeft, height, cz };
    vertices[2].texture  = { texLeft, midZ };


    // UPPER-LEFT vertex (3)
    height               = GetScaledInterpolatedHeightAtPoint(fLeft, fUp);
    vertices[3].position = { fLeft, height, fUp };
    vertices[3].texture  = { texLeft, texTop };

    // UPPER-MID vertex (4): use this vertex if the upper patch is NOT of a lower LOD
    height               = GetScaledInterpolatedHeightAtPoint(cx, fUp);
    vertices[4].position = { cx, height, fUp };
    vertices[4].texture  = { midX, texTop };

    // UPPER-RIGHT vertex (5)
    height               = GetScaledInterpolatedHeightAtPoint(fRight, fUp);
    vertices[5].position = { fRight, height, fUp };
    vertices[5].texture  = { texRight, texTop };

     // MID-RIGHT vertex(6): use this vertex if the right patch is NOT of a lower LOD
    height               = GetScaledInterpolatedHeightAtPoint(fRight, cz);
    vertices[6].position = { fRight, height, cz };
    vertices[6].texture  = { texRight, midZ };
   
    // LOWER-RIGHT vertex (7)
    height               = GetScaledInterpolatedHeightAtPoint(fRight, fDown);
    vertices[7].position = { fRight, height, fDown };
    vertices[7].texture  = { texRight, texBottom };

     // LOWER-MID vertex (8): use this vertex if the bottom patch is NOT of a lower LOD
    height               = GetScaledInterpolatedHeightAtPoint(cx, fDown);
    vertices[8].position = { cx, height, fDown };
    vertices[8].texture  = { midX, texBottom };

    // ----------------------------------------------------


    UINT indices[24]{0};
    int numIdxs = 0;

    // (case 1) add vertex 2 (so we have two triangles on the left side fan)
    // (case 2) we have no mid-left vertex
    // 
    //         3                3
    //         | \              | \
    //   (1)   2--0        (2)  |  0
    //         | /              | /
    //         1                1
    if (neighbor.left)                          
    {
        AddTriangle(indices, numIdxs, 0, 1, 2);
        AddTriangle(indices, numIdxs, 0, 2, 3);
    }
    else
    {
        AddTriangle(indices, numIdxs, 0, 1, 3);
    }

    // (case 1) add vertex 4 (so we have two triangles on the upper side of the fan)
    // (case 2) we have no upper-mid vertex
    // 
    //         3--4--5             3-----5
    //   (1)    \ | /         (2)   \   /
    //            0                   0
    if (neighbor.up)
    {
        AddTriangle(indices, numIdxs, 0, 3, 4);
        AddTriangle(indices, numIdxs, 0, 4, 5);
    }
    else
    {
        AddTriangle(indices, numIdxs, 0, 3, 5);
    }

    // (case 1) add vertex 6 (so we have two triangles on the right side of the fan)
    // (case 2) we have no mid-right vertex
    //            5                   5
    //          / |                 / |
    //  (1)    0--6         (2)    0  |
    //          \ |                 \ |
    //            7                   7
    if (neighbor.right)
    {
        AddTriangle(indices, numIdxs, 0, 5, 6);
        AddTriangle(indices, numIdxs, 0, 6, 7);
    }
    else
    {
        AddTriangle(indices, numIdxs, 0, 5, 7);
    }

    // (case 1) add vertex 8 (so we have two triangles on the bottom side of the fan)
    // (case 2) we have no bottom-mid vertex
    //          0                   0
    //  (1)   / | \        (2)    /   \
    //       1--8--7             1-----7
    if (neighbor.down)
    {
        AddTriangle(indices, numIdxs, 0, 7, 8);
        AddTriangle(indices, numIdxs, 0, 8, 1);
    }
    else
    {
        AddTriangle(indices, numIdxs, 0, 7, 1);
    }

    // ----------------------------------------------------

    int baseVertexIdx = verticesOffset_;

    // make correction on indices
    for (int i = 0; i < numIdxs; ++i)
        indices[i] += baseVertexIdx;

    // fill in the CPU-side vertex buffer so later we will load vertices onto GPU
    memcpy(&vertices_[verticesOffset_], vertices, sizeof(Vertex3dTerrain) * 9);
    verticesOffset_ += 9;

    // fill in the CPU-side index buffer so later we will load indices onto GPU
    memcpy(&indices_[indicesOffset_], indices, sizeof(UINT)*numIdxs);
    indicesOffset_ += numIdxs;
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
    Material& mat = g_MaterialMgr.GetMaterialByID(materialID_);

    // NOTE: we used slightly different approach of terrain types:
    // for instance ambient texture type can be used for detail map or something like that
    mat.SetTexture(type, texID);
}



#if 0
//==================================================================================
bool TerrainGeomip::InitGeomipmapping(const int patchSize)
{
    int divisor = 0;
    int LOD = 0;
    int numAllPatches = 0;

    // since terrain is squared the width and height are equal
    const int terrainSize = terrainWidth_;

    if ((terrainSize - 1) % (patchSize - 1) != 0)
    {
        int patchSz = patchSize - 1;
        int recommendedTerrainSize = ((terrainSize - 1 + patchSz) / (patchSz)) * (patchSz)+1;
        LogErr(LOG, "terrain size minus 1 (%d) must be divisible by patch size minus 1 (%d)", terrainSize, patchSize);
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

    // release all the memory from prev patches data
    if (patches_)
        Shutdown();

    // initiate the patch info
    patchSize_ = patchSize;
    numPatchesPerSide_ = (terrainSize-1) / (patchSize-1);

    maxLOD_ = lodManager_.InitLodManager(patchSize, numPatchesPerSide_, numPatchesPerSide_);
    lodInfo_.resize(maxLOD_ + 1);
}
#endif

} // namespace
