// =================================================================================
// Filename:  TerrainGeomipmapped.cpp
// Desc:      implementation of terrain geomimapping CLOD algorithm
//            (CLOD - continuous level of detail)
//
// Created:   10.06.2025 by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include <CoreCommon/Frustum.h>
#include "TerrainGeomipmapped.h"
#include "../Mesh/MaterialMgr.h"

#include <DirectXMath.h>
#include <DirectXCollision.h>

using namespace DirectX;

#define COMPUTE_NORMALS                 1
#define COMPUTE_AVERAGED_MID_NORMALS    1
#define COMPUTE_AVERAGED_CENTER_NORMALS 1

//---------------------------------------
// Desc:   helpers to get a square of input value
//---------------------------------------
inline int   SQR(const int number)   { return number * number; }
inline float SQR(const float number) { return number * number; }


namespace Core
{

void TerrainGeomipmapped::ClearMemory()
{
    // release memory from the CPU copy of vertices/indices
    SafeDeleteArr(vertices_);
    SafeDeleteArr(indices_);
}

///////////////////////////////////////////////////////////

void TerrainGeomipmapped::Shutdown()
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

///////////////////////////////////////////////////////////

void TerrainGeomipmapped::AllocateMemory(const int numVertices, const int numIndices)
{
    // allocate memory for vertices/indices data arrays
    try
    {
        assert(numVertices > 0);
        assert(numIndices > 0);

        // prepare memory
        ClearMemory();

        numVertices_ = numVertices;
        numIndices_  = numIndices;

        vertices_    = new Vertex3dTerrain[numVertices_]{};
        indices_     = new UINT[numIndices_]{ 0 };
    }
    catch (const std::bad_alloc& e)
    {
        Shutdown();

        LogErr(e.what());
        LogErr("can't allocate memory for some data of the model");
        return;
    }
}

//---------------------------------------------------------
// Desc:   initiate the geomipmapping system
// Args:   - patchSize:  the size of the patch (in vertices)
//                       a good size is usually around 17 (17x17 verts)
//---------------------------------------------------------
bool TerrainGeomipmapped::InitGeomipmapping(const int patchSize)
{
    int divisor = 0;
    int LOD = 0;
    int numAllPatches = 0;

    try
    {
        // since terrain is squared the width and height are equal
        const int terrainSize = heightMap_.GetWidth();

        // check some stuff
        CAssert::True(terrainSize != 0, "terrain's height map size cannot be 0");
        CAssert::True(patchSize > 0,    "input patch size must be > 0");

        // release all the memory from prev patches data
        if (patches_)
            Shutdown();

        // initiate the patch info
        patchSize_         = patchSize;
        numPatchesPerSide_ = terrainSize / patchSize;

        numAllPatches      = numPatchesPerSide_ * numPatchesPerSide_;
        patches_           = new GeomPatch[numAllPatches];

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

        LogMsg("Geomipmapping system successfully initialized");
        return true;
    }
    catch (const std::bad_alloc& e)
    {
        LogErr(e.what());
        LogErr("can't allocate memory for geomipmapping patches");
        return false;
    }
}

///////////////////////////////////////////////////////////

bool TerrainGeomipmapped::InitBuffers(
    ID3D11Device* pDevice,
    const Vertex3dTerrain* vertices,
    const UINT* indices,
    const int numVertices,
    const int numIndices)
{
    // init vertex/index buffers with input data
    try
    {
        CAssert::True(vertices,         "input ptr to arr of vertices == nullptr");
        CAssert::True(indices,          "input ptr to arr of indices == nullptr");
        CAssert::True(numVertices > 0,  "input number of vertices must be > 0");
        CAssert::True(numIndices > 0,   "input number of indices must be > 0");

        constexpr bool isDynamic = true;
        vb_.Initialize(pDevice, vertices, numVertices, isDynamic);
        ib_.Initialize(pDevice, indices, numIndices);

        return true;
    }
    catch (EngineException& e)
    {
        LogErr(e);
        return false;
    }
}

#define USE_DX_FRUSTUM false

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
// Desc:   update the geomipmapping system
// Args:   - cameraPos: camera position in the world
//---------------------------------------------------------
void TerrainGeomipmapped::Update(const CameraParams& camParams)
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


    const DirectX::XMMATRIX view(camParams.viewMatrix);
    const int numPerSide = numPatchesPerSide_;

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
}

//---------------------------------------------------------
// Desc:   compute geometry for each terrain's patch
//         according to its LOD
//---------------------------------------------------------
void TerrainGeomipmapped::ComputeTesselation(void)
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
//         - px: patch location by X-axis
//         - pz: patch location by Z-axis
//---------------------------------------------------------
void TerrainGeomipmapped::ComputePatch(
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
// Desc:   compute normal vector by 3 input positions
// Args:   - pos0, pos1, pos2: positions in 3D space
// Ret:    - vec3 value:       3D normal vector
//---------------------------------------------------------
XMFLOAT3 ComputeFaceNormal(
    const XMFLOAT3& pos0,
    const XMFLOAT3& pos1,
    const XMFLOAT3& pos2)
{
    // compute face normal for the first triangle
    XMVECTOR v0 = DirectX::XMLoadFloat3(&pos0);
    XMVECTOR v1 = DirectX::XMLoadFloat3(&pos1);
    XMVECTOR v2 = DirectX::XMLoadFloat3(&pos2);

    XMVECTOR e0 = v1 - v0;
    XMVECTOR e1 = v2 - v0;

    XMVECTOR normal = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(e0, e1));

    float* n = normal.m128_f32;
    return { n[0], n[1], n[2] };
}

//---------------------------------------------------------
// Desc:   compute a single fan geometry
// Args:   - cx, cz:   center of the triangle fan to render
//         - size:     the fan's entire size
//         - neighbor: the fan's neighbor structure (used to avoid cracking)
//---------------------------------------------------------
void TerrainGeomipmapped::ComputeFan(
    const float cx,
    const float cz,
    const float size,
    const GeomNeighbor& neighbor)
{
    const float halfSize       = size * 0.5f;
    const float invTerrainSize = 1.0f / heightMap_.GetWidth();

    // calc the texture coords
    const float texLeft   = fabsf(cx - halfSize) * invTerrainSize;
    const float texBottom = fabsf(cz - halfSize) * invTerrainSize;
    const float texRight  = fabsf(cx + halfSize) * invTerrainSize;
    const float texTop    = fabsf(cz + halfSize) * invTerrainSize;

    const float midX      = (texLeft + texRight) * 0.5f;
    const float midZ      = (texBottom + texTop) * 0.5f;

    // vertices positions
    const int   centerX   = (int)cx;
    const int   centerZ   = (int)cz;

    const float fLeft     = cx - halfSize;
    const float fRight    = cx + halfSize;
    const float fUp       = cz + halfSize;
    const float fDown     = cz - halfSize;

    const int   iLeft     = (int)(fLeft);
    const int   iRight    = (int)(fRight);
    const int   iUp       = (int)(fUp);
    const int   iDown     = (int)(fDown);


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
    //height               = GetScaledHeightAtPoint((int)cx, (int)cz);
    height               = GetScaledInterpolatedHeightAtPoint(cx, cz);
    vertices[0].position = { cx, height, cz };
    vertices[0].texture  = { midX, midZ };

    // LOWER-LEFT vertex (1)
    //height               = GetScaledHeightAtPoint(iLeft, iDown);
    height               = GetScaledInterpolatedHeightAtPoint(fLeft, fDown);
    vertices[1].position = { fLeft, height, fDown};
    vertices[1].texture  = { texLeft, texBottom };

    // UPPER-LEFT vertex (3)
    //height               = GetScaledHeightAtPoint(iLeft, iUp);
    height               = GetScaledInterpolatedHeightAtPoint(fLeft, fUp);
    vertices[3].position = { fLeft, height, fUp };
    vertices[3].texture  = { texLeft, texTop };

    // UPPER-RIGHT vertex (5)
    //height               = GetScaledHeightAtPoint(iRight, iUp);
    height               = GetScaledInterpolatedHeightAtPoint(fRight, fUp);
    vertices[5].position = { fRight, height, fUp };
    vertices[5].texture  = { texRight, texTop };
   
    // LOWER-RIGHT vertex (7)
    //height               = GetScaledHeightAtPoint(iRight, iDown);
    height               = GetScaledInterpolatedHeightAtPoint(fRight, fDown);
    vertices[7].position = { fRight, height, fDown };
    vertices[7].texture  = { texRight, texBottom };

    // ----------------------------------------------------

    Vertex3dTerrain vertexBuf[24];
    int   numVerts = 0;  // number of vertices in this fan

    // add vertex 2 (so we have two triangles on the left side fan)
    if (neighbor.left)                          
    {
        // MID-LEFT vertex (2): use this vertex if the left patch is NOT of a lower LOD
        //height               = GetScaledHeightAtPoint(iLeft, centerZ);
        height               = GetScaledInterpolatedHeightAtPoint(fLeft, cz);
        vertices[2].position = { fLeft, height, cz };
        vertices[2].texture  = { texLeft, midZ };

        vertexBuf[numVerts + 0] = vertices[0];    
        vertexBuf[numVerts + 1] = vertices[1];    //  3
        vertexBuf[numVerts + 2] = vertices[2];    //  | \ 
                                                  //  2--0
        vertexBuf[numVerts + 3] = vertices[0];    //  | /
        vertexBuf[numVerts + 4] = vertices[2];    //  1
        vertexBuf[numVerts + 5] = vertices[3];

#if COMPUTE_NORMALS
        // compute normal vectors for the first and second triangle
        XMFLOAT3 normal1 = ComputeFaceNormal(
                                vertexBuf[numVerts + 0].position,
                                vertexBuf[numVerts + 1].position,
                                vertexBuf[numVerts + 2].position);

        XMFLOAT3 normal2 = ComputeFaceNormal(
                                vertexBuf[numVerts + 3].position,
                                vertexBuf[numVerts + 4].position,
                                vertexBuf[numVerts + 5].position);

        // --------------------------------------

        vertexBuf[numVerts + 0].normal = normal1;
        vertexBuf[numVerts + 1].normal = normal1;
        vertexBuf[numVerts + 2].normal = normal1;

        vertexBuf[numVerts + 3].normal = normal2;
        vertexBuf[numVerts + 4].normal = normal2;
        vertexBuf[numVerts + 5].normal = normal2;

        // average the normal vector for vertex 2 in this triangle
        XMFLOAT3 avgNormal = vertexBuf[numVerts+2].normal + vertexBuf[numVerts+4].normal;
        avgNormal *= 0.5f;
        const XMFLOAT3 normalizedAvgNormal = DirectX::XMFloat3Normalize(avgNormal);

        vertexBuf[numVerts + 2].normal = normalizedAvgNormal;
        vertexBuf[numVerts + 4].normal = normalizedAvgNormal;

        // --------------------------------------
#endif
        numVerts += 6;
    }
    // we have no mid-left vertex
    else
    {                                           //  3
        vertexBuf[numVerts+0] = vertices[0];    //  | \ 
        vertexBuf[numVerts+1] = vertices[1];    //  |  0
        vertexBuf[numVerts+2] = vertices[3];    //  | /
                                                //  1
#if COMPUTE_NORMALS
         // compute normal vectors for the triangle
        XMFLOAT3 normal = ComputeFaceNormal(
            vertexBuf[numVerts + 0].position,
            vertexBuf[numVerts + 1].position,
            vertexBuf[numVerts + 2].position);

        vertexBuf[numVerts + 0].normal = normal;
        vertexBuf[numVerts + 1].normal = normal;
        vertexBuf[numVerts + 2].normal = normal;
#endif
        numVerts += 3;
    }

    // add vertex 4 (so we have two triangles on the upper side of the fan)
    if (neighbor.up)
    {
        // UPPER-MID vertex (4): use this vertex if the upper patch is NOT of a lower LOD
        //height               = GetScaledHeightAtPoint(centerX, iUp);
        height               = GetScaledInterpolatedHeightAtPoint(cx, fUp);
        vertices[4].position = { cx, height, fUp };
        vertices[4].texture  = { midX, texTop };

        vertexBuf[numVerts + 0] = vertices[0];
        vertexBuf[numVerts + 1] = vertices[3];    
        vertexBuf[numVerts + 2] = vertices[4];    //  3--4--5
                                                  //   \ | /
        vertexBuf[numVerts + 3] = vertices[0];    //     0
        vertexBuf[numVerts + 4] = vertices[4];    
        vertexBuf[numVerts + 5] = vertices[5];

#if COMPUTE_NORMALS
        // compute normal vectors for the first and second triangle
        XMFLOAT3 normal1 = ComputeFaceNormal(
            vertexBuf[numVerts + 0].position,
            vertexBuf[numVerts + 1].position,
            vertexBuf[numVerts + 2].position);

        XMFLOAT3 normal2 = ComputeFaceNormal(
            vertexBuf[numVerts + 3].position,
            vertexBuf[numVerts + 4].position,
            vertexBuf[numVerts + 5].position);

        // --------------------------------------

        vertexBuf[numVerts + 0].normal = normal1;
        vertexBuf[numVerts + 1].normal = normal1;
        vertexBuf[numVerts + 2].normal = normal1;

        vertexBuf[numVerts + 3].normal = normal2;
        vertexBuf[numVerts + 4].normal = normal2;
        vertexBuf[numVerts + 5].normal = normal2;

        // average the normal vector for vertex 4 in this triangle
        XMFLOAT3 avgNormal = (normal1 + normal2);
        avgNormal *= 0.5f;
        const XMFLOAT3 normalizedAvgNormal = DirectX::XMFloat3Normalize(avgNormal);

        vertexBuf[numVerts + 2].normal = normalizedAvgNormal;
        vertexBuf[numVerts + 4].normal = normalizedAvgNormal;

        // --------------------------------------
#endif

        numVerts += 6;
    }
    // we have no upper-mid vertex
    else
    {
        vertexBuf[numVerts+0] = vertices[0];    //  3-----5
        vertexBuf[numVerts+1] = vertices[3];    //   \   /
        vertexBuf[numVerts+2] = vertices[5];    //     0


#if COMPUTE_NORMALS
        // compute normal vectors for the triangle
        XMFLOAT3 normal = ComputeFaceNormal(
            vertexBuf[numVerts + 0].position,
            vertexBuf[numVerts + 1].position,
            vertexBuf[numVerts + 2].position);

        vertexBuf[numVerts + 0].normal = normal;
        vertexBuf[numVerts + 1].normal = normal;
        vertexBuf[numVerts + 2].normal = normal;
#endif

        numVerts += 3;
    }

    // add vertex 6 (so we have two triangles on the right side of the fan)
    if (neighbor.right)
    {
        // MID-RIGHT vertex(6): use this vertex if the right patch is NOT of a lower LOD
        //height               = GetScaledHeightAtPoint(iRight, centerZ);
        height               = GetScaledInterpolatedHeightAtPoint(fRight, cz);
        vertices[6].position = { fRight, height, cz };
        vertices[6].texture  = { texRight, midZ };

        vertexBuf[numVerts + 0] = vertices[0];
        vertexBuf[numVerts + 1] = vertices[5];      //     5
        vertexBuf[numVerts + 2] = vertices[6];      //   / |
                                                    //  0--6
        vertexBuf[numVerts + 3] = vertices[0];      //   \ |
        vertexBuf[numVerts + 4] = vertices[6];      //     7
        vertexBuf[numVerts + 5] = vertices[7];

#if COMPUTE_NORMALS
        // compute normal vectors for the first and second triangle
        XMFLOAT3 normal1 = ComputeFaceNormal(
            vertexBuf[numVerts + 0].position,
            vertexBuf[numVerts + 1].position,
            vertexBuf[numVerts + 2].position);

        XMFLOAT3 normal2 = ComputeFaceNormal(
            vertexBuf[numVerts + 3].position,
            vertexBuf[numVerts + 4].position,
            vertexBuf[numVerts + 5].position);

        // --------------------------------------

        vertexBuf[numVerts + 0].normal = normal1;
        vertexBuf[numVerts + 1].normal = normal1;
        vertexBuf[numVerts + 2].normal = normal1;

        vertexBuf[numVerts + 3].normal = normal2;
        vertexBuf[numVerts + 4].normal = normal2;
        vertexBuf[numVerts + 5].normal = normal2;

        // average the normal vector for vertex 6 in this triangle
        XMFLOAT3 avgNormal = vertexBuf[numVerts + 2].normal + vertexBuf[numVerts + 4].normal;
        avgNormal *= 0.5f;
        const XMFLOAT3 normalizedAvgNormal = DirectX::XMFloat3Normalize(avgNormal);

        vertexBuf[numVerts + 2].normal = normalizedAvgNormal;
        vertexBuf[numVerts + 4].normal = normalizedAvgNormal;

#endif
        // --------------------------------------

        numVerts += 6;
    }
    // we have no mid-right vertex
    else
    {                                           //     5
        vertexBuf[numVerts+0] = vertices[0];    //   / |
        vertexBuf[numVerts+1] = vertices[5];    //  0  |
        vertexBuf[numVerts+2] = vertices[7];    //   \ |
                                                //     7

#if COMPUTE_NORMALS
        // compute normal vectors for the triangle
        XMFLOAT3 normal = ComputeFaceNormal(
            vertexBuf[numVerts + 0].position,
            vertexBuf[numVerts + 1].position,
            vertexBuf[numVerts + 2].position);

        vertexBuf[numVerts + 0].normal = normal;
        vertexBuf[numVerts + 1].normal = normal;
        vertexBuf[numVerts + 2].normal = normal;
#endif
        numVerts += 3;

    }

    // add vertex 8 (so we have two triangles on the bottom side of the fan)
    if (neighbor.down)
    {
        // LOWER-MID vertex (8): use this vertex if the bottom patch is NOT of a lower LOD
        //height               = GetScaledHeightAtPoint(centerX, iDown);
        height               = GetScaledInterpolatedHeightAtPoint(cx, fDown);
        vertices[8].position = { cx, height, fDown };
        vertices[8].texture  = { midX, texBottom };

        vertexBuf[numVerts + 0] = vertices[0];
        vertexBuf[numVerts + 1] = vertices[7];
        vertexBuf[numVerts + 2] = vertices[8];      //     0
                                                    //   / | \ 
        vertexBuf[numVerts + 3] = vertices[0];      //  1--8--7
        vertexBuf[numVerts + 4] = vertices[8];    
        vertexBuf[numVerts + 5] = vertices[1];

#if COMPUTE_NORMALS
        // compute normal vectors for the first and second triangle
        XMFLOAT3 normal1 = ComputeFaceNormal(
            vertexBuf[numVerts + 0].position,
            vertexBuf[numVerts + 1].position,
            vertexBuf[numVerts + 2].position);

        XMFLOAT3 normal2 = ComputeFaceNormal(
            vertexBuf[numVerts + 3].position,
            vertexBuf[numVerts + 4].position,
            vertexBuf[numVerts + 5].position);

        // --------------------------------------

        vertexBuf[numVerts + 0].normal = normal1;
        vertexBuf[numVerts + 1].normal = normal1;
        vertexBuf[numVerts + 2].normal = normal1;

        vertexBuf[numVerts + 3].normal = normal2;
        vertexBuf[numVerts + 4].normal = normal2;
        vertexBuf[numVerts + 5].normal = normal2;

        // average the normal vector for vertex 8 in this triangle
        XMFLOAT3 avgNormal = vertexBuf[numVerts + 2].normal + vertexBuf[numVerts + 4].normal;
        avgNormal *= 0.5f;
        const XMFLOAT3 normalizedAvgNormal = DirectX::XMFloat3Normalize(avgNormal);

        vertexBuf[numVerts + 2].normal = normalizedAvgNormal;
        vertexBuf[numVerts + 4].normal = normalizedAvgNormal;
#endif

        // --------------------------------------

        numVerts += 6;
    }
    // we have no bottom-mid vertex
    else
    {
        vertexBuf[numVerts+0] = vertices[0];    //     0
        vertexBuf[numVerts+1] = vertices[7];    //   /   \ 
        vertexBuf[numVerts+2] = vertices[1];    //  1-----7

#if COMPUTE_NORMALS
           // compute normal vectors for the triangle
        XMFLOAT3 normal = ComputeFaceNormal(
            vertexBuf[numVerts + 0].position,
            vertexBuf[numVerts + 1].position,
            vertexBuf[numVerts + 2].position);

        vertexBuf[numVerts + 0].normal = normal;
        vertexBuf[numVerts + 1].normal = normal;
        vertexBuf[numVerts + 2].normal = normal;
#endif
        numVerts += 3;

    }

#if COMPUTE_AVERAGED_CENTER_NORMALS
    XMFLOAT3 centralNormal{0,0,0};

    // compute averaged normal for central vertex
    for (int i = 0; i < numVerts/3; ++i)
    {
        centralNormal += vertexBuf[i*3].normal;
    }
    centralNormal *= 0.125f;

    // normalize averaged central normal vector
    DirectX::XMFloat3Normalize(centralNormal);

    // set averaged central normal for each triangle
    for (int i = 0; i < numVerts / 3; ++i)
    {
        vertexBuf[i*3+ 0].normal = centralNormal;
    }
#endif


    // copy vertices into the main buffer so later we will load all of them into GPU
    memcpy(&vertices_[verticesOffset_], vertexBuf, sizeof(Vertex3dTerrain) * numVerts);
    verticesOffset_ += numVerts;
}

///////////////////////////////////////////////////////////

void TerrainGeomipmapped::SetAABB(const DirectX::XMFLOAT3& center, const DirectX::XMFLOAT3& extents)
{
    // set axis-aligned bounding box for the terrain
    center_ = center;
    extents_ = extents;
}

///////////////////////////////////////////////////////////

void TerrainGeomipmapped::SetMaterial(const MaterialID matID)
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

///////////////////////////////////////////////////////////

void TerrainGeomipmapped::SetTexture(const int idx, const TexID texID)
{
    // set texture (its ID) by input idx

    if ((idx < 0) || (idx >= NUM_TEXTURE_TYPES))
    {
        sprintf(g_String, "wrong input idx: %d", idx);
        LogErr(g_String);
        return;
    }

    if (texID == 0)
    {
        sprintf(g_String, "wrong input texture ID: %ld", texID);
        LogErr(g_String);
        return;
    }

    // everything is ok so set the texture for terrain's material
    Material& mat = g_MaterialMgr.GetMaterialByID(materialID_);

    // NOTE: we used slightly different approach of terrain types:
    // for instance ambient texture type can be used for detail map or something like that
    mat.SetTexture(eTexType(idx), texID);
}

} // namespace
