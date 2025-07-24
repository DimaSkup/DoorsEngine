// =================================================================================
// Filename:   TerrainQuadtree.cpp
// Created:    06.07.2025 by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include "TerrainQuadtree.h"


namespace Core
{

// =================================================================================
// Constants
// =================================================================================

// QT - quad tree
// LR - lower right
// LL - lower left
// UL - upper left
// UR - upper right
#define QT_LR_NODE 0
#define QT_LL_NODE 1
#define QT_UL_NODE 2
#define QT_UR_NODE 3

// bit codes
#define QT_COMPLETE_FAN 0
#define QT_LL_UR        10
#define QT_LR_UL        5
#define QT_NO_FAN       15


//---------------------------------------------------------
// Desc:   initialize the quadtree stuff
// Args:   none
// Ret:    - true: successful init
//         - false: unsuccessful init
//---------------------------------------------------------
bool TerrainQuadtree::Init(void)
{

    const int size = (int)heightMap_.GetWidth();
    terrainLength_ = size;

    if (!heightMap_.IsLoaded() || (size == 0))
    {
        LogErr(LOG, "can't init terrain's quadtree: the height map is wrong");
        return false;
    }

    // alloc memory for the quadtree matrix
    quadMatrix_ = new uint8[size*size]{0};
    if (!quadMatrix_)
    {
        LogErr("can't allocate memory for the quadtree matrix");
        return false;
    }


    // TODO: replace with memset?
    // 
    // initialize the quadtree matrix to 'true' (for all the possible nodes)
    memset(quadMatrix_, 1, size*size);
#if 0
    for (int z = 0; z < size; ++z)
    {
        for (int x = 0; x < size; ++x)
        {
            quadMatrix_[GetMatrixIdx(x, z)] = 1; // true
        }
    }
#endif


    //PropagateRoughness();


#if 0

    // calculate the center of the terrain mesh
    const float center = size * 0.5f;

    // build the mesh through top-down quadtree traversal
    CameraParams params;
    params.posX = 0;
    params.posY = 0;
    params.posZ = 0;
    RefineNode(center, center, size, params);

    GenerateNode(center, center, size);

    vb_.UpdateDynamic(g_pContext, vertices_, verticesOffset_);
#endif


    // initialization was successful
    LogMsg("The quadtree terrain has been successfully initialized");
    return true;
}

//---------------------------------------------------------
// Desc:   release the quadtree terrain's stuff
//---------------------------------------------------------
void TerrainQuadtree::Release(void)
{
    SafeDeleteArr(quadMatrix_);

    // release memory from the CPU copy of vertices/indices
    SafeDeleteArr(vertices_);
    SafeDeleteArr(indices_);

    // release memory from the vertex/index buffers
    vb_.Shutdown();
    ib_.Shutdown();

    // release memory from the height/tile/light/detail/etc. maps
    ClearMemoryFromMaps();
}

//---------------------------------------------------------
// Desc:  allocate memory for vertices/indices data arrays
//---------------------------------------------------------
bool TerrainQuadtree::AllocateMemory(const int numVertices, const int numIndices)
{
    assert(numVertices > 0);
    assert(numIndices > 0);

    // release memory from the CPU copy of vertices/indices if we have any
    SafeDeleteArr(vertices_);
    SafeDeleteArr(indices_);

    numVertices_ = numVertices;
    numIndices_ = numIndices;

    vertices_ = NEW Vertex3dTerrain[numVertices];
    if (!vertices_)
    {
        LogErr(LOG, "can't allocate memory for terrain vertices");
        return false;
    }

    indices_ = NEW UINT[numIndices];
    if (!indices_)
    {
        LogErr(LOG, "can't allocate memory for terrain indices");
        return false;
    }

    // init the arr of indices with zeros
    memset(indices_, 0, sizeof(UINT)*numIndices);

    return true;
}


//---------------------------------------------------------
// Desc:   initialize DirectX vertex/index buffers with input data
//---------------------------------------------------------
bool TerrainQuadtree::InitBuffers(
    const Vertex3dTerrain* vertices,
    const UINT* indices,
    const int numVertices,
    const int numIndices)
{
    // check input args
    if (!vertices || !indices)
    {
        LogErr(LOG, "input ptr to vertex or index buffer == nullptr");
        return false;
    }

    if ((numVertices <= 0) || (numIndices <= 0))
    {
        LogErr(LOG, "input number of vertices/indices <= 0");
        return false;
    }

    // release memory from the vertex/index buffers
    vb_.Shutdown();
    ib_.Shutdown();

    constexpr bool isDynamic = true;

    // initialize the vertex buffer
    if (!vb_.Initialize(g_pDevice, vertices, numVertices, isDynamic))
    {
        LogErr(LOG, "can't initialize a vertex buffer for terrain");
        Release();
        return false;
    }

    // initialize the index buffer
    if (!ib_.Initialize(g_pDevice, indices, numIndices, isDynamic))
    {
        LogErr(LOG, "can't initialize the index buffer for terrain");
        Release();
        return false;
    }

    return true;
}


//---------------------------------------------------------
// Desc:   update the quadtree
// Args:   - params:  camera's params for the current frame
//---------------------------------------------------------
void TerrainQuadtree::Update(const CameraParams& params)
{
    const int size = terrainLength_;

    // calculate the center of the terrain mesh
    const float center = (float)(size-1) * 0.5f;

    // reset the counting variables
    verticesOffset_ = 0;
    indicesOffset_ = 0;

    // build the mesh through top-down quadtree traversal
    RefineNode(center, center, size, params);

    GenerateNode(center, center, size);

    vb_.UpdateDynamic(g_pContext, vertices_, verticesOffset_);
}

//---------------------------------------------------------
// Desc:  setup a size of the CPU-side vertex buffer (array)
//---------------------------------------------------------
void TerrainQuadtree::SetNumVertices(const int num)
{
    if (num <= 0)
    {
        LogErr(LOG, "input number of vertices cannot be <= 0");
        return;
    }

    numVertices_ = (uint32)num;
}

//---------------------------------------------------------
// Desc:  setup a side of the CPU-side index buffer (array)
//---------------------------------------------------------
void TerrainQuadtree::SetNumIndices(const int num)
{
    if (num <= 0)
    {
        LogErr(LOG, "input number of indices cannot be <= 0");
        return;
    }

    numIndices_ = (uint32)num;
}

//---------------------------------------------------------
// Desc:   propagate the roughness of the height map (so more
//         triangles will get applied to rougher area of the map)
//---------------------------------------------------------
void TerrainQuadtree::PropagateRoughness(void)
{
    const int   terrainSize = terrainLength_;
    const float upperBound  = 1.0f * minResolution_ / (2.0f * (minResolution_ - 1.0f));

    int dh      = 0;
    int d2      = 0;
    int localD2 = 0;
    int localH  = 0;
    

    // set the edge length to 3 (lowest possible length)
    int edgeLen = 3;

    // start off at the lowest LOD, and traverse up to the highest node (lowest detail)
    while (edgeLen <= terrainSize)
    {
        // offset of node edges (since all the edges are the same length)
        const int edgeOffset = (edgeLen - 1) >> 1;

        // offset of the node's children's edges
        const int childOffset = (edgeLen - 1) >> 2;

        for (int z = edgeOffset; z < terrainSize; z += (edgeLen - 1))
        {
            for (int x = edgeOffset; x < terrainSize; x += (edgeLen - 1))
            {
                const int left  = x - edgeOffset;
                const int right = x + edgeOffset;
                const int up    = z + edgeOffset;
                const int down  = z - edgeOffset;

                const uint8 heightUpL    = GetTrueHeightAtPoint(left,  up);
                const uint8 heightUpM    = GetTrueHeightAtPoint(x,     up);
                const uint8 heightUpR    = GetTrueHeightAtPoint(right, up);

                const uint8 heightLowL   = GetTrueHeightAtPoint(left,  down);
                const uint8 heightLowM   = GetTrueHeightAtPoint(x,     down);
                const uint8 heightLowR   = GetTrueHeightAtPoint(right, down);

                const uint8 heightMidL   = GetTrueHeightAtPoint(left,  z);
                const uint8 heightCenter = GetTrueHeightAtPoint(x,     z);
                const uint8 heightMidR   = GetTrueHeightAtPoint(right, z);

                // TODO: replace abs/max/etc with functions which based on fast bithacks
                float absolute = 0;

                // compute "localD2" values for this node
                // upper-mid
                absolute = abs(((heightUpL + heightUpR) >> 1) - heightUpM);
                localD2  = (int)ceil(absolute);

                // right-mid
                absolute = abs(((heightUpR + heightLowR) >> 1) - heightMidR);
                dh       = (int)ceil(absolute);
                localD2  = max(localD2, dh);

                // bottom-mid
                absolute = abs(((heightLowL + heightLowR) >> 1) - heightLowM);
                dh       = (int)ceil(absolute);
                localD2  = max(localD2, dh);

                // left-mid
                absolute = abs(((heightLowL + heightUpL) >> 1) - heightMidL);
                dh       = (int)ceil(absolute);
                localD2  = max(localD2, dh);

                // bottom-left to top-right diagonal
                absolute = abs(((heightLowL + heightUpR) >> 1) - heightCenter);
                dh       = (int)ceil(absolute);
                localD2  = max(localD2, dh);

                // bottom-right to top left diagonal
                absolute = abs(((heightLowR + heightUpL) >> 1) - heightCenter);
                dh       = (int)ceil(absolute);
                localD2  = max(localD2, dh);

                // make localD2 a value btw 0-255
                localD2 = (int)ceil((localD2 * 3) / edgeLen);

                // test minimally sized block
                if (edgeLen == 3)
                {
                    d2 = localD2;

                    // compute the "localH" value
                    // upper right
                    localH = heightUpR;

                    // upper left/mid
                    localH = max(localH, heightUpL);
                    localH = max(localH, heightUpM);

                    // lower left/mid/right
                    localH = max(localH, heightLowL);
                    localH = max(localH, heightLowM);
                    localH = max(localH, heightLowR);

                    // middle left/center/right
                    localH = max(localH, heightMidL);
                    localH = max(localH, heightCenter);
                    localH = max(localH, heightMidR);

                    // store the maximum localH value in the matrix
                    quadMatrix_[GetMatrixIdx(x+1, z)] = localH;
                }

                else
                {
                    const float t1 = upperBound * (float)GetQuadMatrixData(x, z);
                    const float t2 = upperBound * (float)GetQuadMatrixData(left, z);
                    const float t3 = upperBound * (float)GetQuadMatrixData(right, z);
                    const float t4 = upperBound * (float)GetQuadMatrixData(x, up);
                    const float t5 = upperBound * (float)GetQuadMatrixData(x, down);

                    //float fD2 = 0;

                    // use d2 value from farther up on the quadtree
                    d2 = (int)ceilf(max(t1, (float)localD2));
                    d2 = (int)ceilf(max(t2, (float)d2));
                    d2 = (int)ceilf(max(t3, (float)d2));
                    d2 = (int)ceilf(max(t4, (float)d2));
                    d2 = (int)ceilf(max(t5, (float)d2));

                    //d2 = (int)fD2;

                    // get the max local height values of the 4 nodes (LL, LR, UL, UR)
                    const int childL = x - childOffset;
                    const int childR = x + childOffset;
                    const int childU = z + childOffset;
                    const int childD = z - childOffset;

                    localH =             GetTrueHeightAtPoint(childL, childU);
                    localH = max(localH, GetTrueHeightAtPoint(childR, childU));
                    localH = max(localH, GetTrueHeightAtPoint(childL, childD));
                    localH = max(localH, GetTrueHeightAtPoint(childR, childD));
                    
                    // store the max value in the quadtree matrix
                    quadMatrix_[GetMatrixIdx(x+1, z)] = localH;
                }

                // store the values we calculated for D2 into the quadtree matrix
                quadMatrix_[GetMatrixIdx(x-1, z)] = d2;
                quadMatrix_[GetMatrixIdx(x,   z)] = d2;

                // propagate the value up the quadtree
                const int matIdx1 = GetMatrixIdx(left,  down);
                const int matIdx2 = GetMatrixIdx(left,  up);
                const int matIdx3 = GetMatrixIdx(right, up);
                const int matIdx4 = GetMatrixIdx(right, down);

                quadMatrix_[matIdx1] = max(quadMatrix_[matIdx1], d2);
                quadMatrix_[matIdx2] = max(quadMatrix_[matIdx2], d2);
                quadMatrix_[matIdx3] = max(quadMatrix_[matIdx3], d2);
                quadMatrix_[matIdx4] = max(quadMatrix_[matIdx4], d2);
            }
        }

        // move up to the next quadtree level (lower level of detail)
        edgeLen = (edgeLen << 1) - 1;
    }
}

//---------------------------------------------------------
// Desc:   refine a quadtree node (update the quadtree matrix)
// Args:   - x, z:    center of the current node
//         - edgeLen: length of the current node's edge
//         - camera:  camera's params for the current frame (for calculating distance)
//---------------------------------------------------------
void TerrainQuadtree::RefineNode(
    const float x,
    const float z,
    const int edgeLen,
    const CameraParams& camera)
{
    const int iX = (int)x;
    const int iZ = (int)z;

    // calculate the distance from the current point (L1 NORM, which, essentially, is
    // faster version of usual distance equation you may be used to...)
    const float viewDistance = fabsf(camera.posX - x) +
                               fabsf(camera.posY - GetQuadMatrixData(iX+1, iZ)) +
                               fabsf(camera.posZ - z);

    //                                 l
    // compute the 'f' value == -----------------
    //                           d * C * max(c, 1)

    // TODO: simplify
    const float minRes = minResolution_;
    const float desRes = desiredResolution_;
    const float c      = desRes * GetQuadMatrixData(iX-1, iZ) / 3;

    const float f = viewDistance / ((float)edgeLen * minRes * max(c, 1.0f));

    const bool subdivide = (f < 1.0f);

    // store whether or not the current node gets subdivided
    quadMatrix_[GetMatrixIdx(iX, iZ)] = subdivide;

    if (subdivide)
    {
        // we need to recurse down farther into the quadtree
        // (3 is node's min edge length)
        if (!(edgeLen <= 3))
        {
            const float childOffset  = (float)((edgeLen-1) >> 2);
            const int   childEdgeLen = (edgeLen+1) >> 1;

            // refine the various child nodes
            // lower left 
            RefineNode(x-childOffset, z-childOffset, childEdgeLen, camera);

            // lower right 
            RefineNode(x+childOffset, z-childOffset, childEdgeLen, camera);

            // upper left
            RefineNode(x-childOffset, z+childOffset, childEdgeLen, camera);

            // upper right
            RefineNode(x+childOffset, z+childOffset, childEdgeLen, camera);
        }
    }
}

inline void AddTriangle(
    Vertex3dTerrain* vertices,
    const Vertex3dTerrain& v0,
    const Vertex3dTerrain& v1,
    const Vertex3dTerrain& v2)
{

}

//---------------------------------------------------------
// Desc:   generate leaf (no children) quadtree node
// Args:   - cx, cz: center of current node
//         - edgeLen: length of the current node's edge
//---------------------------------------------------------
void TerrainQuadtree::GenerateNode(
    const float cx,
    const float cz,
    const int edgeLen)
{
    const int iX = (int)cx;
    const int iZ = (int)cz;

    // get the blend factor from the current quadtree value
    const bool subdivide = GetQuadMatrixData(iX, iZ);

    if (!subdivide)
        return;


    // compute the offset to the nodes near the current node
    const int   adjOffset   = edgeLen - 1;

    // compute the edge offset of the current node
    const int   iEdgeOffset    = adjOffset >> 1;
    const float fEdgeOffset    = adjOffset * 0.5f;

    const int   terrainSize    = terrainLength_;
    const float invTerrainSize = 1.0f / (float)terrainSize;

    // compute the position coordinates
    float left  = cx-fEdgeOffset;
    float right = cx+fEdgeOffset;
    float up    = cz+fEdgeOffset;
    float down  = cz-fEdgeOffset;

    // clamp values if necessary
    if (left < 0)
        left = 0;

    if (right > terrainSize)
        right = terrainSize;

    if (up > terrainSize)
        up = terrainSize;

    if (down < 0)
        down = 0;

    // integer positions
    const int iLeft  = (int)left;
    const int iRight = (int)right;
    const int iUp    = (int)up;
    const int iDown  = (int)down;

    // calculate the texture coordinates
    const float texLeft   = fabsf(left)  * invTerrainSize;
    const float texRight  = fabsf(right) * invTerrainSize;
    const float texTop    = fabsf(up)    * invTerrainSize;
    const float texBottom = fabsf(down)  * invTerrainSize;

    const float texMidX   = (texLeft + texRight) * 0.5f;
    const float texMidY   = (texTop + texBottom) * 0.5f;

/*
    Fan visualization

    NOTE: Vertex by number in braces is optional according
          to LOD of its neighbor patch

          If a neighbour node is of a lower LOD we skip the middle vertex
          on that side of the fan;

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


    // is this the smallest node?
    if (edgeLen <= 3)
    {
#if 1
        // flags to define if we have optional vertices in the fan
        const bool hasMidLeft  = ((iX-adjOffset) < 0)            || GetQuadMatrixData(iX-adjOffset, iZ);
        const bool hasMidRight = ((iX+adjOffset) >= terrainSize) || GetQuadMatrixData(iX+adjOffset, iZ);
        const bool hasUpperMid = ((iZ+adjOffset) >= terrainSize) || GetQuadMatrixData(iX,           iZ+adjOffset);
        const bool hasLowerMid = ((iZ-adjOffset) < 0)            || GetQuadMatrixData(iX,           iZ-adjOffset);

        float height = 0;
        Vertex3dTerrain vertices[9];

        // CENTER vertex (0)
        height               = GetScaledHeightAtPoint(iX, iZ);
        vertices[0].position = { cx, height, cz };
        vertices[0].texture  = { texMidX, texMidY };

        // LOWER-LEFT vertex (1)
        height               = GetScaledHeightAtPoint(iLeft, iDown);
        vertices[1].position = { left, height, down };
        vertices[1].texture  = { texLeft, texBottom };

        // MID-LEFT vertex (2, skip if the adjacent node is of a lower detail level)
        if (hasMidLeft)
        {
            height               = GetScaledHeightAtPoint(iLeft, iZ);
            vertices[2].position = { left, height, cz };
            vertices[2].texture  = { texLeft, texMidY };
        }

        // UPPER-LEFT vertex (3)
        height               = GetScaledHeightAtPoint(iLeft, iUp);
        vertices[3].position = { left, height, up };
        vertices[3].texture  = { texLeft, texTop };

        // UPPER-MID vertex (4, skip if the adjacent node is of a lower detail level)
        if (hasUpperMid)
        {
            height               = GetScaledHeightAtPoint(iX, iUp);
            vertices[4].position = { cx, height, up };
            vertices[4].texture  = { texMidX, texTop };
        }

        // UPPER-RIGHT vertex (5)
        height               = GetScaledHeightAtPoint(iRight, iUp);
        vertices[5].position = { right, height, up };
        vertices[5].texture  = { texRight, texTop };

        // MID-RIGHT vertex (6, skip if the adjacent node is of a lower detail level)
        if (hasMidRight)
        {
            height               = GetScaledHeightAtPoint(iRight, iZ);
            vertices[6].position = { right, height, cz };
            vertices[6].texture  = { texRight, texMidY };
        }

        // LOWER-RIGHT vertex (7)
        height               = GetScaledHeightAtPoint(iRight, iDown);
        vertices[7].position = { right, height, down };
        vertices[7].texture  = { texRight, texBottom };

        // LOWER-MID vertex (8, skip if the adjacent node is of a lower detail level)
        if (hasLowerMid)
        {
            height               = GetScaledHeightAtPoint(iX, iDown);
            vertices[8].position = { cx, height, down };
            vertices[8].texture  = { texMidX, texBottom };
        }

        // ----------------------------------------------------

        // generate fan
        int numVerts = 0;               // number of vertices in this fan
        Vertex3dTerrain vertexBuf[24];

        // add vertex 2 (so we have two triangles on the left side of the fan)
        if (hasMidLeft)
        {
            vertexBuf[numVerts + 0] = vertices[0];
            vertexBuf[numVerts + 1] = vertices[1];      //  3
            vertexBuf[numVerts + 2] = vertices[2];      //  | \ 
                                                        //  2--0
            vertexBuf[numVerts + 3] = vertices[0];      //  | /
            vertexBuf[numVerts + 4] = vertices[2];      //  1
            vertexBuf[numVerts + 5] = vertices[3];

            numVerts += 6;
        }

        // we have no mid-left vertex
        else
        {                                               //  3
            vertexBuf[numVerts + 0] = vertices[0];      //  | \ 
            vertexBuf[numVerts + 1] = vertices[1];      //  |  0
            vertexBuf[numVerts + 2] = vertices[3];      //  | /
                                                        //  1
            numVerts += 3;
        }

        // ----------------------------------------------------

        // add vertex 4 (so we have two triangles on the upper side of the fan)
        if (hasUpperMid)
        {
            vertexBuf[numVerts + 0] = vertices[0];
            vertexBuf[numVerts + 1] = vertices[3];
            vertexBuf[numVerts + 2] = vertices[4];      //  3--4--5
                                                        //   \ | /
            vertexBuf[numVerts + 3] = vertices[0];      //     0
            vertexBuf[numVerts + 4] = vertices[4];
            vertexBuf[numVerts + 5] = vertices[5];

            numVerts += 6;
        }

        // we have no upper-mid vertex
        else
        {
            vertexBuf[numVerts + 0] = vertices[0];      //  3-----5
            vertexBuf[numVerts + 1] = vertices[3];      //   \   /
            vertexBuf[numVerts + 2] = vertices[5];      //     0

            numVerts += 3;
        }

        // ----------------------------------------------------

        // add vertex 6 (so we have two triangles on the right side of the fan)
        if (hasMidRight)
        {
            vertexBuf[numVerts + 0] = vertices[0];
            vertexBuf[numVerts + 1] = vertices[5];      //     5
            vertexBuf[numVerts + 2] = vertices[6];      //   / |
                                                        //  0--6
            vertexBuf[numVerts + 3] = vertices[0];      //   \ |
            vertexBuf[numVerts + 4] = vertices[6];      //     7
            vertexBuf[numVerts + 5] = vertices[7];

            numVerts += 6;
        }

        // we have no mid-right vertex
        else
        {                                               //     5
            vertexBuf[numVerts + 0] = vertices[0];      //   / |
            vertexBuf[numVerts + 1] = vertices[5];      //  0  |
            vertexBuf[numVerts + 2] = vertices[7];      //   \ |
                                                        //     7
            numVerts += 3;
        }

        // ----------------------------------------------------

        // add vertex 8 (so we have two triangles on the bottom side of the fan)
        if (hasLowerMid)
        {
            vertexBuf[numVerts + 0] = vertices[0];
            vertexBuf[numVerts + 1] = vertices[7];
            vertexBuf[numVerts + 2] = vertices[8];      //     0
                                                        //   / | \ 
            vertexBuf[numVerts + 3] = vertices[0];      //  1--8--7
            vertexBuf[numVerts + 4] = vertices[8];
            vertexBuf[numVerts + 5] = vertices[1];

            numVerts += 6;
        }

        // we have no bottom-mid vertex
        else
        {
            vertexBuf[numVerts + 0] = vertices[0];      //     0 
            vertexBuf[numVerts + 1] = vertices[7];      //   /   \ 
            vertexBuf[numVerts + 2] = vertices[1];      //  1-----7

            numVerts += 3;
        }

        // ----------------------------------------------------

        // copy vertices into the CPU-side vertices buffer so later we
        // will load all of them into GPU
        memcpy(&vertices_[verticesOffset_], vertexBuf, sizeof(Vertex3dTerrain)*numVerts);
        verticesOffset_ += numVerts;
#endif
    }

    // go down to smaller quadtrees nodes
    else
    {
        int fanCode = 0;

        // calculate the child node's offset values
        const int   iChildOffset = (edgeLen - 1) >> 2;
        const float fChildOffset = (float)iChildOffset;

        // calculate the edge length of the child nodes
        const int   childEdgeLen = (edgeLen + 1) >> 1;

        const int childL = iX - iChildOffset;   // child left
        const int childR = iX + iChildOffset;   // child right
        const int childD = iZ - iChildOffset;   // child down
        const int childU = iZ + iChildOffset;   // child up


        /*
                                   
                           |
                           |              
                           |
                UL Node*4  |   UR Node*8
                -----------+-----------
                           |
                           |              
                           |
                LL Node*2  |   LR Node*1
                                   
        */

        // calculate the bit-fanCode for the fan arrangement
        // (which fans need to be rendered)
      
        // upper left
        fanCode =  (GetQuadMatrixData(childL, childU) != 0) * 4;   // 4

        // upper right
        fanCode |= (GetQuadMatrixData(childR, childU) != 0) * 8;   // 8

        // lower left
        fanCode |= (GetQuadMatrixData(childL, childD) != 0) * 2;   // 2

        // lower right
        fanCode |= (GetQuadMatrixData(childR, childD) != 0);       // 1

        // now, use the previously calculate codes, and render some tri-fans :)
        // this node has four children, no rendering is needed (for this node at least),
        // but we need to recurse down to this node's children
        if (fanCode == QT_NO_FAN)
        {
            // lower left
            GenerateNode(childL, childD, childEdgeLen);

            // lower right
            GenerateNode(childR, childD, childEdgeLen);

            // upper left
            GenerateNode(childL, childU, childEdgeLen);

            // upper right
            GenerateNode(childR, childU, childEdgeLen);

            return;
        }

#if 1
        // generate the lower left and upper right fans (quads)
        if (fanCode == QT_LL_UR)
        {
            /*
                            1 ----- 2
                            |     / |
                            |   /   |
                            | /     |
                    6 ----- 0 ----- 3
                    |     / |
                    |   /   |
                    | /     |
                    5 ----- 4
            */

            Vertex3dTerrain vertices[7];
            float height = 0;

            // center vertex (0)
            height                  = GetScaledHeightAtPoint(iX, iZ);
            vertices[0].position    = { cx, height, cz };
            vertices[0].texture     = { texMidX, texMidY };

            // upper-mid vertex (1)
            height                  = GetScaledHeightAtPoint(iX, iUp);
            vertices[1].position    = { cx, height, up };
            vertices[1].texture     = { texMidX, texTop };

            // upper-right vertex (2)
            height                  = GetScaledHeightAtPoint(iRight, iUp);
            vertices[2].position    = { right, height, up };
            vertices[2].texture     = { texRight, texTop };

            // mid-right vertex (3)
            height                  = GetScaledHeightAtPoint(iRight, iZ);
            vertices[3].position    = { right, height, cz };
            vertices[3].texture     = { texRight, texMidY };

            // bottom-mid vertex (4)
            height                  = GetScaledHeightAtPoint(iX, iDown);
            vertices[4].position    = { cx, height, down };
            vertices[4].texture     = { texMidX, texBottom };

            // bottom-left vertex (5)
            height                  = GetScaledHeightAtPoint(iLeft, iDown);
            vertices[5].position    = { left, height, down };
            vertices[5].texture     = { texLeft, texBottom };

            // mid-left vertex (6)
            height                  = GetScaledHeightAtPoint(iLeft, iZ);
            vertices[6].position    = { left, height, cz };
            vertices[6].texture     = { texLeft, texMidY };

            // --------------------------------------------

            // make triangles (3 vertices per each triangle)
            constexpr int numVerts = 12;
            Vertex3dTerrain vertexBuf[numVerts];

            constexpr UINT indices[numVerts] =
            {
                0,1,2,   0,2,3,   // first fan  (quad)
                0,4,5,   0,5,6    // second fan (quad)
            };

            // TODO: optimize
            // 
            // fill in the final array
            for (int i = 0; i < numVerts; ++i)
            {
                const UINT idx = indices[i];
                vertexBuf[i] = vertices[idx];
            }

            // copy vertices into the CPU-side vertices buffer so later we
            // will load all of them into GPU
            memcpy(&vertices_[verticesOffset_], vertexBuf, sizeof(Vertex3dTerrain) * numVerts);
            verticesOffset_ += numVerts;

            // --------------------------------------------

            // recurse further down to the upper left and lower right nodes
            GenerateNode(childL, childU, childEdgeLen);
            GenerateNode(childR, childD, childEdgeLen);

            return;
        }
#endif


#if 1

        // generate the lower-right and upper left triangles fans
        if (fanCode == QT_LR_UL)
        {
            /*
                    2 ----- 3
                    | \     |
                    |   \   |
                    |     \ |
                    1 ----- 0 ----- 4
                            | \     |
                            |   \   |
                            |     \ |
                            6 ----- 5
            */

            Vertex3dTerrain vertices[7];
            float height = 0;

            // center vertex (0)
            height                  = GetScaledHeightAtPoint(iX, iZ);
            vertices[0].position    = { cx, height, cz };
            vertices[0].texture     = { texMidX, texMidY };

            // mid-left vertex (1)
            height                  = GetScaledHeightAtPoint(iLeft, iZ);
            vertices[1].position    = { left, height, cz };
            vertices[1].texture     = { texLeft, texMidY };

            // upper-left vertex (2)
            height                  = GetScaledHeightAtPoint(iLeft, iUp);
            vertices[2].position    = { left, height, up };
            vertices[2].texture     = { texLeft, texTop };

            // upper-mid vertex (3)
            height                  = GetScaledHeightAtPoint(iX, iUp);
            vertices[3].position    = { cx, height, up };
            vertices[3].texture     = { texMidX, texTop };

            // mid-right vertex (4)
            height                  = GetScaledHeightAtPoint(iRight, iZ);
            vertices[4].position    = { right, height, cz };
            vertices[4].texture     = { texRight, texMidY };

            // lower-right vertex (5)
            height                  = GetScaledHeightAtPoint(iRight, iDown);
            vertices[5].position    = { right ,height, down };
            vertices[5].texture     = { texRight, texBottom };

            // lower-mid vertex (6)
            height                  = GetScaledHeightAtPoint(iX, iDown);
            vertices[6].position    = { cx, height, down };
            vertices[6].texture     = { texMidX, texBottom };

            // --------------------------------------------

            // make triangles (3 vertices per each triangle)
            constexpr int numVerts = 12;
            Vertex3dTerrain vertexBuf[numVerts];

            constexpr UINT indices[numVerts] =
            {
                0,1,2,   0,2,3,    // first fan (quad)
                0,4,5,   0,5,6,    // second fan (quad)
            };

            // TODO: optimize
            // 
            // fill in the final array
            for (int i = 0; i < numVerts; ++i)
            {
                const UINT idx = indices[i];
                vertexBuf[i] = vertices[idx];
            }

            // copy vertices into the CPU-side vertices buffer so later we
            // will load all of them into GPU
            memcpy(&vertices_[verticesOffset_], vertexBuf, sizeof(Vertex3dTerrain)* numVerts);
            verticesOffset_ += numVerts;

            // --------------------------------------------

            // resurse further down to the upper-right and lower-left nodes
            GenerateNode(childR, childU, childEdgeLen);
            GenerateNode(childL, childD, childEdgeLen);

            return;
        }
#endif


#if 1
        // this node is a leaf-node, generate a complete fan
        if (fanCode == QT_COMPLETE_FAN)
        {
            /*
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

  
            // flags to define if we have optional vertices in the fan
            const bool hasMidLeft  = ((iX-adjOffset) < 0)            || GetQuadMatrixData(iX-adjOffset, iZ);
            const bool hasMidRight = ((iX+adjOffset) >= terrainSize) || GetQuadMatrixData(iX+adjOffset, iZ);
            const bool hasUpperMid = ((iZ+adjOffset) >= terrainSize) || GetQuadMatrixData(iX,           iZ+adjOffset);
            const bool hasLowerMid = ((iZ-adjOffset) < 0)            || GetQuadMatrixData(iX,           iZ-adjOffset);


            float height = 0;
            Vertex3dTerrain vertices[9];

            // center vertex (0)
            height                   = GetScaledHeightAtPoint(iX, iZ);
            vertices[0].position     = { cx, height, cz };
            vertices[0].texture      = { texMidX, texMidY };

            // lower-left vertex (1)
            height                   = GetScaledHeightAtPoint(iLeft, iDown);
            vertices[1].position     = { left, height, down };
            vertices[1].texture      = { texLeft, texBottom };

            // mid-left vertex (2, skip if the adjacent node is of a lower detail level)
            if (hasMidLeft)
            {
                height               = GetScaledHeightAtPoint(iLeft, iZ);
                vertices[2].position = { left, height, cz };
                vertices[2].texture  = { texLeft, texMidY };
            }

            // upper-left vertex (3)
            height                   = GetScaledHeightAtPoint(iLeft, iUp);
            vertices[3].position     = { left, height, up };
            vertices[3].texture      = { texLeft, texTop };

            // upper-mid vertex (4, skip if the adjacent node is of a lower detail level)
            if (hasUpperMid)
            {
                height               = GetScaledHeightAtPoint(iX, iUp);
                vertices[4].position = { cx, height, up };
                vertices[4].texture  = { texMidX, texTop };
            }

            // upper-right vertex (5)
            height                   = GetScaledHeightAtPoint(iRight, iUp);
            vertices[5].position     = { right, height, up };
            vertices[5].texture      = { texRight, texTop };

            // mid-right vertex (6, skip if the adjacent node is of a lower detail level)
            if (hasMidRight)
            {
                height               = GetScaledHeightAtPoint(iRight, iZ);
                vertices[6].position = { right, height, cz };
                vertices[6].texture  = { texRight, texMidY };
            }

            // lower-right vertex (7)
            height                   = GetScaledHeightAtPoint(iRight, iDown);
            vertices[7].position     = { right, height, down };
            vertices[7].texture      = { texRight, texBottom };

            // lower-mid vertex (8, skip if the adjacent node is of a lower detail level)
            if (hasLowerMid)
            {
                height               = GetScaledHeightAtPoint(iX, iDown);
                vertices[8].position = { cx, height, down };
                vertices[8].texture  = { texMidX, texBottom };
            }

            // --------------------------------------------

            // make triangles (3 vertices per each triangle)

            Vertex3dTerrain vertexBuf[24];
            int numVerts = 0;

            // add vertex 2 (so we have two triangles on the left side of the fan)
            if (hasMidLeft)
            {
                vertexBuf[numVerts + 0] = vertices[0];
                vertexBuf[numVerts + 1] = vertices[1];      //  3
                vertexBuf[numVerts + 2] = vertices[2];      //  | \ 
                                                            //  2--0
                vertexBuf[numVerts + 3] = vertices[0];      //  | /
                vertexBuf[numVerts + 4] = vertices[2];      //  1
                vertexBuf[numVerts + 5] = vertices[3];

                numVerts += 6;
            }

            // we have no mid-left vertex
            else
            {                                               //  3
                vertexBuf[numVerts + 0] = vertices[0];      //  | \ 
                vertexBuf[numVerts + 1] = vertices[1];      //  |  0
                vertexBuf[numVerts + 2] = vertices[3];      //  | /
                                                            //  1
                numVerts += 3;
            }


            // ----------------------------------------------------

            // add vertex 4 (so we have two triangles on the upper side of the fan)
            if (hasUpperMid)
            {
                vertexBuf[numVerts + 0] = vertices[0];
                vertexBuf[numVerts + 1] = vertices[3];
                vertexBuf[numVerts + 2] = vertices[4];      //  3--4--5
                                                            //   \ | /
                vertexBuf[numVerts + 3] = vertices[0];      //     0
                vertexBuf[numVerts + 4] = vertices[4];
                vertexBuf[numVerts + 5] = vertices[5];

                numVerts += 6;
            }

            // we have no upper-mid vertex
            else
            {
                vertexBuf[numVerts + 0] = vertices[0];      //  3-----5
                vertexBuf[numVerts + 1] = vertices[3];      //   \   /
                vertexBuf[numVerts + 2] = vertices[5];      //     0

                numVerts += 3;
            }

            // ----------------------------------------------------

            // add vertex 6 (so we have two triangles on the right side of the fan)
            if (hasMidRight)
            {
                vertexBuf[numVerts + 0] = vertices[0];
                vertexBuf[numVerts + 1] = vertices[5];      //     5
                vertexBuf[numVerts + 2] = vertices[6];      //   / |
                                                            //  0--6
                vertexBuf[numVerts + 3] = vertices[0];      //   \ |
                vertexBuf[numVerts + 4] = vertices[6];      //     7
                vertexBuf[numVerts + 5] = vertices[7];

                numVerts += 6;
            }

            // we have no mid-right vertex
            else
            {                                               //     5
                vertexBuf[numVerts + 0] = vertices[0];      //   / |
                vertexBuf[numVerts + 1] = vertices[5];      //  0  |
                vertexBuf[numVerts + 2] = vertices[7];      //   \ |
                                                            //     7
                numVerts += 3;
            }

            // ----------------------------------------------------

            // add vertex 8 (so we have two triangles on the bottom side of the fan)
            if (hasLowerMid)
            {
                vertexBuf[numVerts + 0] = vertices[0];
                vertexBuf[numVerts + 1] = vertices[7];
                vertexBuf[numVerts + 2] = vertices[8];      //     0
                                                            //   / | \ 
                vertexBuf[numVerts + 3] = vertices[0];      //  1--8--7
                vertexBuf[numVerts + 4] = vertices[8];
                vertexBuf[numVerts + 5] = vertices[1];

                numVerts += 6;
            }

            // we have no bottom-mid vertex
            else
            {
                vertexBuf[numVerts + 0] = vertices[0];      //     0 
                vertexBuf[numVerts + 1] = vertices[7];      //   /   \ 
                vertexBuf[numVerts + 2] = vertices[1];      //  1-----7

                numVerts += 3;
            }

            // ----------------------------------------------------

            // copy vertices into the CPU-side vertices buffer so later we
            // will load all of them into GPU
            memcpy(&vertices_[verticesOffset_], vertexBuf, sizeof(Vertex3dTerrain)*numVerts);
            verticesOffset_ += numVerts;

            return;
        }

#endif

        /*
                           |
                           |
                           |
                UL Node*4  |   UR Node*8
            	-----------+-----------   
                           |
                           |
                           |
                LL Node*2  |   LR Node*1
        */

   
        constexpr uint8 s_FanStart[] = { 3,  3, 0,  3, 1, 0,  0,  3, 2,  2, 0,  2,  1,  1,  0, 0 };
        constexpr uint8 s_FanCode[]  = { 10, 8, 8, 12, 8, 0, 12, 14, 8, 12, 0, 14, 12, 14, 14, 0 };

        // the remaining cases are only partial fans, so we need to figure out what to render
        int start    = s_FanStart[fanCode];
        int fanLen   = 0;
        long code    = s_FanCode[fanCode];
        float height = 0;

        // calculate the fan length by computing the index of
        // the first non-zero bit in s_FanCode array
        while (!(code & (1 << fanLen)) && fanLen<8)
            fanLen++;

        //-------------------------------------------------

        // generate a triangle fan

#if 1
        height                 = GetScaledHeightAtPoint(iX, iZ);
        Vertex3dTerrain center = { cx, height, cz, texMidX, texMidY };

        const int adjLX      = iX-adjOffset;   // adjacent left X
        const int adjRX      = iX+adjOffset;   // adjacent right X
        const int adjDZ      = iZ-adjOffset;   // adjacent down (lower) Z
        const int adjUZ      = iZ+adjOffset;   // adjacent upper Z

        const bool hasLowerM = ((adjDZ < 0)            || GetQuadMatrixData(iX, adjDZ));
        const bool hasMidL   = ((adjLX < 0)            || GetQuadMatrixData(adjLX, iZ));
        const bool hasMidR   = ((adjRX >= terrainSize) || GetQuadMatrixData(adjRX, iZ));
        const bool hasUpperM = ((adjUZ >= terrainSize) || GetQuadMatrixData(iX, adjUZ));


        for (int fanPos = fanLen; fanPos > 0; fanPos--)
        {
            const bool isBegin = (fanPos == fanLen);

            switch (start)
            {
                // lower right node
                case QT_LR_NODE:
                {
                    // lower right vertex
                    height = GetScaledHeightAtPoint(iRight, iDown);
                    Vertex3dTerrain lowerR = { right, height, down, texRight, texBottom };

                    // lower mid, skip if the adjacent node is of a lower detail level
                    //
                    //      0
                    //      | \ 
                    //      |   \
                    //      2----1
                    //
                    if (hasLowerM || isBegin)
                    {
                        height = GetScaledHeightAtPoint(iX, iDown);
                        Vertex3dTerrain lowerM = { cx, height, down, texMidX, texBottom };

                        vertices_[verticesOffset_ + 0] = center;
                        vertices_[verticesOffset_ + 1] = lowerR;
                        vertices_[verticesOffset_ + 2] = lowerM;

                        verticesOffset_ += 3;
                    }

                    // finish off the fan with a right mid vertex
                    //
                    //      0----1
                    //       \   |
                    //         \ |
                    //           2
                    //
                    if (fanPos == 1)
                    {
                        height = GetScaledHeightAtPoint(iRight, iZ);
                        Vertex3dTerrain midR = { right, height, cz, texRight, texMidY };

                        vertices_[verticesOffset_ + 0] = center;
                        vertices_[verticesOffset_ + 1] = midR;
                        vertices_[verticesOffset_ + 2] = lowerR;

                        verticesOffset_ += 3;
                    }

                    break;
                }

                // lower left node
                case QT_LL_NODE:
                {
                    height = GetScaledHeightAtPoint(iLeft, iDown);
                    Vertex3dTerrain lowerL = { left, height, down, texLeft, texBottom };

                    // left mid, skip if the adjacent node is of a lower detail level
                    //
                    //      2----0
                    //      |   /
                    //      | /  
                    //      1    
                    //
                    if (hasMidL || isBegin)
                    {
                        height = GetScaledHeightAtPoint(iLeft, iZ);
                        Vertex3dTerrain midL = { left, height, cz, texLeft, texMidY };

                        vertices_[verticesOffset_ + 0] = center;
                        vertices_[verticesOffset_ + 1] = lowerL;
                        vertices_[verticesOffset_ + 2] = midL;

                        verticesOffset_ += 3;
                    }

                    // finish off the fan with a lower mid vertex
                    //
                    //           0
                    //         / |
                    //       /   |
                    //      2----1
                    //
                    if (fanPos == 1)
                    {
                        height = GetScaledHeightAtPoint(iX, iDown);
                        Vertex3dTerrain lowerM = { cx, height, down, texMidX, texBottom };

                        vertices_[verticesOffset_ + 0] = center;
                        vertices_[verticesOffset_ + 1] = lowerM;
                        vertices_[verticesOffset_ + 2] = lowerL;

                        verticesOffset_ += 3;
                    }

                    break;
                }

                // upper left node
                case QT_UL_NODE:
                {
                    height = GetScaledHeightAtPoint(iLeft, iUp);
                    Vertex3dTerrain upperL = { left, height, up, texLeft, texTop };

                    // upper mid, skip if the adjacent node is of a lower detail level
                    //
                    //    1----2
                    //     \   |
                    //       \ |
                    //         0
                    //
                    if (hasUpperM || isBegin)
                    {
                        height = GetScaledHeightAtPoint(iX, iUp);
                        Vertex3dTerrain upperM = { cx, height, up, texMidX, texTop };

                        vertices_[verticesOffset_ + 0] = center;
                        vertices_[verticesOffset_ + 1] = upperL;
                        vertices_[verticesOffset_ + 2] = upperM;

                        verticesOffset_ += 3;
                    }

                    // finish off the fan with a left mid vertex
                    //
                    //    2
                    //    | \
                    //    |   \
                    //    1----0
                    //
                    if (fanPos == 1)
                    {
                        height = GetScaledHeightAtPoint(iLeft, iZ);
                        Vertex3dTerrain midL = { left, height, cz, texLeft, texMidY };

                        vertices_[verticesOffset_ + 0] = center;
                        vertices_[verticesOffset_ + 1] = midL;
                        vertices_[verticesOffset_ + 2] = upperL;

                        verticesOffset_ += 3;
                    }

                    break;
                }

                // upper right node
                case QT_UR_NODE:
                {
                    height = GetScaledHeightAtPoint(iRight, iUp);
                    Vertex3dTerrain upperR = { right, height, up, texRight, texTop };

                    // right mid, skip if the adjacent node is of a lower detail level
                    //
                    //         1
                    //       / |
                    //     /   |
                    //    0----2
                    //
                    if (hasMidR || isBegin)
                    {
                        height = GetScaledHeightAtPoint(iRight, iZ);
                        Vertex3dTerrain midR = { right, height, cz, texRight, texMidY };

                        vertices_[verticesOffset_ + 0] = center;
                        vertices_[verticesOffset_ + 1] = upperR;
                        vertices_[verticesOffset_ + 2] = midR;

                        verticesOffset_ += 3;
                    }

                    // finish off the fan with a top mid vertex
                    //
                    //     1----2
                    //     |   /
                    //     | /
                    //     0
                    //
                    if (fanPos == 1)
                    {
                        height = GetScaledHeightAtPoint(iX, iUp);
                        Vertex3dTerrain upperM = { cx, height, up, texMidX, texTop };

                        vertices_[verticesOffset_ + 0] = center;
                        vertices_[verticesOffset_ + 1] = upperM;
                        vertices_[verticesOffset_ + 2] = upperR;

                        verticesOffset_ += 3;
                    }

                    break;

                } // case
            } // switch

            start--;
            start &= 3;   // -1 & 3  == 3

        } // for


#endif

#if 1
        // now, recurse down to children (special cases that were'nt handled earlier)
        for (int fanPos = (4-fanLen); fanPos > 0; fanPos--)
        {
            switch (start)
            {
                // lower right node
                case QT_LR_NODE:
                    GenerateNode(childR, childD, childEdgeLen);
                    break;

                // lower left node
                case QT_LL_NODE:
                    GenerateNode(childL, childD, childEdgeLen);
                    break;

                // upper left node
                case QT_UL_NODE:
                    GenerateNode(childL, childU, childEdgeLen);
                    break;

                // upper right node
                case QT_UR_NODE:
                    GenerateNode(childR, childU, childEdgeLen);
                    break;
            }

            start--;
            start &= 3;    //  -1 & 3  = 3
        }

#endif
        return;
    }
}

//---------------------------------------------------------
// Desc:   set axis-aligned bounding box for the terrain
// Args:   - center:   3d point which define the center of AABB
//         - extents:  3d vector from center to any point of AABB
//---------------------------------------------------------
void TerrainQuadtree::SetAABB(const DirectX::XMFLOAT3& center, const DirectX::XMFLOAT3& extents)
{
    center_  = center;
    extents_ = extents;
}

//---------------------------------------------------------
// Desc:   setup the material ID for the terrain
// Args:   - matID:   material identifier (for details look at: MaterialMgr)
//---------------------------------------------------------
void TerrainQuadtree::SetMaterial(const MaterialID matID)
{
    if (matID != INVALID_MATERIAL_ID)
    {
        materialId_ = matID;
    }
    else
    {
        LogErr("can't setup material ID because input ID == 0");
        return;
    }
}

}; // namespace Core
