////////////////////////////////////////////////////////////////////////////////////////////
// Filename:    GeometryGenerator.cpp
// Description: implementation of the GeometryClass functional;
//
// Created:     13.03.24
////////////////////////////////////////////////////////////////////////////////////////////
#include <CoreCommon/pch.h>
#include "GeometryGenerator.h"

#include "../Model/ModelMath.h"
#include "../Render/Color.h"

#pragma warning (disable : 4996)


using XMVECTOR = DirectX::XMVECTOR;
using XMFLOAT2 = DirectX::XMFLOAT2;
using XMFLOAT3 = DirectX::XMFLOAT3;
using XMFLOAT4 = DirectX::XMFLOAT4;
using XMCOLOR = DirectX::PackedVector::XMCOLOR;


namespace Core
{

void GeometryGenerator::ComputeTangents(
    Vertex3D* vertices,
    UINT* indices,
    const int numIndices)
{
    // compute tangent for each input vertex
    try
    {
        CAssert::True(vertices,       "input ptr to vertices arr == nullptr");
        CAssert::True(indices,        "input ptr to indices arr == nullptr");
        CAssert::True(numIndices > 0, "input number of indices must be > 0");

        for (int i = 0; i < numIndices;)
        {
            // get indices
            const UINT i0 = indices[i++];
            const UINT i1 = indices[i++];
            const UINT i2 = indices[i++];

            // get vertices by indices
            Vertex3D& v0 = vertices[i0];
            Vertex3D& v1 = vertices[i1];
            Vertex3D& v2 = vertices[i2];

            // compute edge vectors of the triangle
            const XMFLOAT3 e0 = {
                v1.position.x - v0.position.x,
                v1.position.y - v0.position.y,
                v1.position.z - v0.position.z };

            const XMFLOAT3 e1 = {
                v2.position.x - v0.position.x,
                v2.position.y - v0.position.y,
                v2.position.z - v0.position.z };

            // compute deltas of texture coords
            const float du0 = v1.texture.x - v0.texture.x;
            const float dv0 = v1.texture.y - v0.texture.y;
            const float du1 = v2.texture.x - v0.texture.x;
            const float dv1 = v2.texture.y - v0.texture.y;

            // compute interse matrix of texture coords
            const float invDet = 1.0f / (du0 * dv1 - dv0 * du1);

            //  | m00 m01 |
            //  | m10 m11 |
            const float m00 = invDet * +dv1;
            const float m01 = invDet * -dv0;
            const float m10 = invDet * -du1;
            const float m11 = invDet * +du0;

            // compute tangent coords
            float Tx = (m00 * e0.x) + (m01 * e1.x);
            float Ty = (m00 * e0.y) + (m01 * e1.y);
            float Tz = (m00 * e0.z) + (m01 * e0.z);

            // normalize the tangent
            const float invTangLen = 1.0f / sqrtf(Tx * Tx + Ty * Ty + Tz * Tz);
            Tx *= invTangLen;
            Ty *= invTangLen;
            Tz *= invTangLen;

            // store tangent into vertices
            v0.tangent = { Tx, Ty, Tz };
            v1.tangent = { Tx, Ty, Tz };
            v2.tangent = { Tx, Ty, Tz };
        }
    }
    catch (EngineException& e)
    {
        LogErr(e);
    }
}

///////////////////////////////////////////////////////////

void GeometryGenerator::GenerateCube(BasicModel& model)
{
    // manually generate cube data 

    constexpr int numFaces    = 6;
    constexpr int numVertices = 24;
    constexpr int numIndices  = 36;
    constexpr int numSubsets  = 1;     // only one mesh

    model.AllocateMemory(numVertices, numIndices, numSubsets);

    // ---------------------------------------------

    // 
    // create vertices of the cube 
    //

    // define positions:
    // - right side
    constexpr XMFLOAT3 pos0 = { 1,  1, -1 };  // near top
    constexpr XMFLOAT3 pos1 = { 1, -1, -1 };  // near bottom
    constexpr XMFLOAT3 pos2 = { 1,  1,  1 };  // far top
    constexpr XMFLOAT3 pos3 = { 1, -1,  1 };  // far bottom

    // - left side
    constexpr XMFLOAT3 pos4 = { -1,  1, -1 }; // near top
    constexpr XMFLOAT3 pos5 = { -1, -1, -1 }; // near bottom
    constexpr XMFLOAT3 pos6 = { -1,  1,  1 }; // far top 
    constexpr XMFLOAT3 pos7 = { -1, -1,  1 }; // far bottom

    // define texture coords
    constexpr XMFLOAT2 tex[4] = { {0, 1}, {0, 0}, {1, 0}, {1, 1} };

    // define normal vectors
    constexpr XMFLOAT3 normFront  = { 0,  0, -1 };
    constexpr XMFLOAT3 tangFront  = { 0, +1,  0 };
    constexpr XMFLOAT3 normBack   = { 0,  0, +1 };
    constexpr XMFLOAT3 normLeft   = { -1, 0,  0 };
    constexpr XMFLOAT3 normRight  = { +1, 0,  0 };
    constexpr XMFLOAT3 normTop    = { 0, +1,  0 };
    constexpr XMFLOAT3 normBottom = { 0, -1,  0 };

    Vertex3D* vertices = model.vertices_;

    // front
    vertices[0] = { pos5, tex[0], normFront };
    vertices[1] = { pos4, tex[1], normFront };
    vertices[2] = { pos0, tex[2], normFront };
    vertices[3] = { pos1, tex[3], normFront };

    // back
    vertices[4] = { pos3, tex[0], normBack };
    vertices[5] = { pos2, tex[1], normBack };
    vertices[6] = { pos6, tex[2], normBack };
    vertices[7] = { pos7, tex[3], normBack };

    // left
    vertices[8]  = { pos7, tex[0], normLeft };
    vertices[9]  = { pos6, tex[1], normLeft };
    vertices[10] = { pos4, tex[2], normLeft };
    vertices[11] = { pos5, tex[3], normLeft };

    // right
    vertices[12] = { pos1, tex[0], normRight };
    vertices[13] = { pos0, tex[1], normRight };
    vertices[14] = { pos2, tex[2], normRight };
    vertices[15] = { pos3, tex[3], normRight };

    // top
    vertices[16] = { pos4, tex[0], normTop };
    vertices[17] = { pos6, tex[1], normTop };
    vertices[18] = { pos2, tex[2], normTop };
    vertices[19] = { pos0, tex[3], normTop };

    // bottom
    vertices[20] = { pos7, tex[0] };
    vertices[21] = { pos5, tex[1] };
    vertices[22] = { pos1, tex[2] };
    vertices[23] = { pos3, tex[3] };

#if 0
    // generate a unique colour for each vertex of each side of the cube
    for (int idx = 0; idx < numVertices; ++idx)
    {
        // stored as a 32-bit ARGB color vector
        const XMCOLOR color(MathHelper::RandF(), MathHelper::RandF(), MathHelper::RandF(), 1.0f);
        
        // ATTENTION: the color member has a DirectX::PackedVector::XMCOLOR type
        vertices[idx].color = Convert::ArgbToAbgr(color);
    }
#endif

    // ---------------------------------------------

    // setup the indices of the cube
    constexpr UINT indicesData[numIndices] =
    {
        0,1,2,    0,2,3,    // front
        4,5,6,    4,6,7,    // back
        8,9,10,   8,10,11,  // left
        12,13,14, 12,14,15, // right			
        16,17,18, 16,18,19, // top			
        20,21,22, 20,22,23  // bottom
    };

    model.CopyIndices(indicesData, numIndices);

    ComputeTangents(model.vertices_, model.indices_, numIndices);

    // ---------------------------------------------

    // setup subset (mesh) data (for cube we have only one subset)
    model.meshes_.SetSubsetName(0, "cube_mesh");
}

///////////////////////////////////////////////////////////

void BuildSkySphereVertices(
    Vertex3DPos* vertices,
    int& vIdx,              // index of the vertex into the vertices array
    const float radius,
    const int sliceCount,
    const int stackCount)
{
    //
    // a helper function for sky sphere vertices generation;
    // (is called from the GenerateSkySphere() method)
    //

    const float dTheta = DirectX::XM_2PI / sliceCount;   // horizontal ring angles delta
    const float dAlpha = DirectX::XM_PI  / stackCount;    // vertical angle delta

    // allocate memory for the transient data
    float* thetaSines   = new float[sliceCount + 1];
    float* thetaÑosines = new float[sliceCount + 1];

    // precompute sin/cos of Theta 
    for (int j = 0; j <= sliceCount; ++j)
        thetaSines[j] = sinf(j * dTheta);

    for (int j = 0; j <= sliceCount; ++j)
        thetaÑosines[j] = cosf(j * dTheta);


    // build vertices
    for (int i = 0; i < stackCount; ++i)
    {
        // vertical angle from bottom to top
        const float curAlpha = -DirectX::XM_PIDIV2 + (float)(i + 1) * dAlpha;

        // radius and height of the current ring
        const float r = radius * cosf(curAlpha);
        const float y = radius * sinf(curAlpha);

        // make vertices for the current ring
        for (int j = 0; j <= sliceCount; ++j)
        {
            vertices[vIdx++] = DirectX::XMFLOAT3(r * thetaÑosines[j], y, r * thetaSines[j]);
        }
    }

    // make the lowest vertex of the sphere
    vertices[vIdx++].position = { 0, -radius, 0 };

    // release memory from the transient data
    SafeDeleteArr(thetaSines);
    SafeDeleteArr(thetaÑosines);
}

///////////////////////////////////////////////////////////

void BuildSkySphereIndices(
    USHORT* indices,
    const int centerIdx,   // index of the lowest center vertex
    const int sliceCount,
    const int stackCount)
{
    int indexIdx = 0;

    //
    // a helper function for sky sphere indices generation;
    // (is called from the GenerateSkySphere() method)
    //

    // Add one because we duplicate the first and last vertex per ring
    // since the texture coordinates are different
    const USHORT ringVertexCount = (USHORT)sliceCount + 1;

    for (USHORT i = 0; i < (USHORT)stackCount - 1; ++i)
    {
        for (USHORT j = 0; j < (USHORT)sliceCount; ++j)
        {
            const USHORT idx_1 = i * ringVertexCount + j;
            const USHORT idx_2 = (i + 1) * ringVertexCount + j;
            const USHORT idx_3 = idx_2 + 1;
            const USHORT idx_4 = idx_1 + 1;

            // first triangle of the sphere face
            indices[indexIdx + 0] = idx_1;
            indices[indexIdx + 1] = idx_2;
            indices[indexIdx + 2] = idx_3;

            // second triangle of the sphere face
            indices[indexIdx + 3] = idx_1;
            indices[indexIdx + 4] = idx_3;
            indices[indexIdx + 5] = idx_4;

            indexIdx += 6;
        }
    }

    // build faces for bottom of the sphere
    for (USHORT i = 0; i < (USHORT)sliceCount; ++i)
    {
        indices[indexIdx + 0] = centerIdx;
        indices[indexIdx + 1] = i;
        indices[indexIdx + 2] = i + 1;

        indexIdx += 3;
    }
}

///////////////////////////////////////////////////////////

void GeometryGenerator::GenerateSkySphere(
    ID3D11Device* pDevice, 
    SkyModel& sky,
    const float radius_,
    const int sliceCount_,
    const int stackCount_)
{
    // check input params and set wrong params to default
    const float radius   = (radius_ > 0.0f) ? radius_ : 1.0f;
    const int sliceCount = (sliceCount_ > 2) ? sliceCount_ : 3;
    const int stackCount = (stackCount_ > 2) ? stackCount_ : 3;

    // compute the number of vertices and indices
    const int numVertices = stackCount * sliceCount + 1 + sliceCount;  // +1 because of lowest center vertex
    const int numIndices  = numVertices * 6 + (sliceCount * 3);         // + (sliceCount * 3) -- because of the lowest part of sphere
    int vertexIdx = 0;

    // prepare memory for the model's data
    Vertex3DPos* vertices = new Vertex3DPos[numVertices];
    USHORT*      indices  = new USHORT[numIndices]{0};

    BuildSkySphereVertices(vertices, vertexIdx, radius, sliceCount, stackCount);
    BuildSkySphereIndices(indices, vertexIdx - 1, sliceCount, stackCount);

    // init vertex/index buffer of the sky
    sky.InitializeBuffers(pDevice, vertices, indices, numVertices, numIndices);

    SafeDeleteArr(vertices);
    SafeDeleteArr(indices);
}

///////////////////////////////////////////////////////////

void GeometryGenerator::GenerateSkyBoxForCubeMap(
    ID3D11Device* pDevice, 
    SkyModel& sky,
    const float height)
{
    // generate a sky cube model for the cube map texture (not altas)

    constexpr int numVertices = 24; 
    constexpr int numIndices = 36;

    // 
    // create vertices of the cube 
    //

    // half height
    const float hh = 0.5f * height;     

    // right side
    const XMFLOAT3 pos0 = { +hh,  hh, -hh };  // near top
    const XMFLOAT3 pos1 = { +hh, -hh, -hh };  // near bottom
    const XMFLOAT3 pos2 = { +hh,  hh,  hh };  // far top
    const XMFLOAT3 pos3 = { +hh, -hh,  hh };  // far bottom

    // left side
    const XMFLOAT3 pos4 = { -hh,  hh, -hh };  // near top
    const XMFLOAT3 pos5 = { -hh, -hh, -hh };  // near bottom
    const XMFLOAT3 pos6 = { -hh,  hh,  hh };  // far top 
    const XMFLOAT3 pos7 = { -hh, -hh,  hh };  // far bottom

    // set position for each vertex
    const Vertex3DPos vertices[numVertices] =
    {
        pos5, pos4, pos0, pos1,   // front
        pos3, pos2, pos6, pos7,   // back

        pos7, pos6, pos4, pos5,   // left
        pos1, pos0, pos2, pos3,   // right

        pos4, pos6, pos2, pos0,   // top
        pos7, pos5, pos1, pos3,   // bottom
    };


    //
    // setup the indices
    //
    const USHORT indices[numIndices] =
    {
        0,1,2,     0,2,3,    // front
        4,5,6,     4,6,7,    // back
        8,9,10,    8,10,11,  // left
        12,13,14,  12,14,15, // right			
        16,17,18,  16,18,19, // top			
        20,21,22,  20,22,23  // bottom
    };

    // initialize vb/ib
    sky.InitializeBuffers(pDevice, vertices, indices, numVertices, numIndices);
}

///////////////////////////////////////////////////////////

void GeometryGenerator::GenerateLineBox(BasicModel& model)
{
    // creates a line box which is used to visualize a bouding boxes;
    // It is made up of series of lines creating a box for debugging purposes;

    constexpr int numVertices = 8; 
    constexpr int numIndices = 72;
    constexpr int numSubsets = 1;

    model.AllocateMemory(numVertices, numIndices, numSubsets);

    // setup vertices position of the line box:
    Vertex3D* vertices = model.vertices_;

    constexpr DirectX::XMFLOAT3 minDimensions{ -1, -1, -1 };
    constexpr DirectX::XMFLOAT3 maxDimensions{ +1, +1, +1 };

    // bottom side of the box
    vertices[0].position = { minDimensions.x, minDimensions.y, minDimensions.z };  // near left
    vertices[1].position = { maxDimensions.x, minDimensions.y, minDimensions.z };  // near right
    vertices[2].position = { maxDimensions.x, minDimensions.y, maxDimensions.z };  // far right
    vertices[3].position = { minDimensions.x, minDimensions.y, maxDimensions.z };

    // top side of the box
    vertices[4].position = { minDimensions.x, maxDimensions.y, minDimensions.z };  // near left
    vertices[5].position = { maxDimensions.x, maxDimensions.y, minDimensions.z };  // near right
    vertices[6].position = { maxDimensions.x, maxDimensions.y, maxDimensions.z };  // far right
    vertices[7].position = { minDimensions.x, maxDimensions.y, maxDimensions.z };

    // setup the indices for the cell lines box
    constexpr UINT indices[numIndices] = {

        // bottom
        0, 1, 0,
        1, 2, 1,
        2, 3, 2,
        3, 0, 3,

        // front
        4, 5, 4,
        5, 1, 5,
        1, 0, 1,
        0, 4, 0,

        // top
        7, 6, 7,
        6, 5, 6,
        5, 4, 5,
        4, 7, 4,

        // back
        6, 7, 6,
        7, 3, 7,
        3, 2, 3,
        2, 6, 2,

        // left
        7, 4, 7,
        4, 0, 4,
        0, 3, 0,
        3, 7, 3,

        // right
        5, 6, 5,
        6, 2, 6,
        2, 1, 2,
        1, 5, 1
    };

    model.CopyIndices(indices, numIndices);

    // ---------------------------------------------

    // setup subset (mesh) data (for cube we have only one subset)
    model.meshes_.SetSubsetName(0, "line_box");
}

//////////////////////////////////////////////////////////

void GeometryGenerator::GeneratePlane(
    const float width,
    const float height,
    BasicModel& model)
{
    // if input params is wrong we use the default params
    const float w = (width > 0) ? width : 1;
    const float h = (height > 0) ? height : 1;

    const float halfWidth  = 0.5f * w;
    const float halfHeight = 0.5f * h;

    const int numVertices = 4;
    const int numIndices = 6;
    const int numSubsets = 1;      // this model has only one subset (mesh)

    DirectX::BoundingBox aabb;

    model.AllocateMemory(numVertices, numIndices, numSubsets);
    Vertex3D*& vertices = model.vertices_;

    // top left / bottom right
    vertices[0].position = { -halfWidth, +halfHeight,  0 };
    vertices[1].position = { +halfWidth, -halfHeight,  0 };

    // bottom left / top right
    vertices[2].position = { -halfWidth, -halfHeight,  0 };
    vertices[3].position = { +halfWidth, +halfHeight,  0 };

    // setup the texture coords of each vertex
    vertices[0].texture = { 0, 0 };
    vertices[1].texture = { 1, 1 };
    vertices[2].texture = { 0, 1 };
    vertices[3].texture = { 1, 0 };

    // setup the normal vectors
    for (int i = 0; i < numVertices; ++i)
        vertices[i].normal = {0,0,-1};

    // setup the indices
    const UINT indicesData[numIndices] = { 0, 1, 2, 0, 3, 1 };
    model.CopyIndices(indicesData, numIndices);

    // ---------------------------------------------

    // setup subset (mesh) data (for plane we have only one subset)
    model.meshes_.SetSubsetName(0, "plane_mesh");
}

//////////////////////////////////////////////////////////

bool BuildFlatGridVertices(
    Vertex3dTerrain* vertices,  // array of vertices (must be already allocated)
    const float width,          // grid size by X
    const float depth)          // grid size by Z
{
    try
    {
        CAssert::True(vertices,  "input arr of vertices == nullptr");
        CAssert::True(width > 0, "input width must be > 0");
        CAssert::True(depth > 0, "input depth must be > 0");

        const XMFLOAT2 offsets[4] =
        {
            XMFLOAT2(0,0),
            XMFLOAT2(0,1),
            XMFLOAT2(1,1),
            XMFLOAT2(1,0)
        };

        const float du = 1.0f / width; // texture stride by X for each quad
        const float dv = 1.0f / depth; // texture stride by Y for each quad
        const float dx = 1.0f;
        const float dz = 1.0f;

        int vIdx = 0;

        // push quads by Z
        for (int i = 0; i < (int)depth; ++i)
        {
            // push quads by X
            for (int j = 0; j < (int)width; ++j)
            {
                Vertex3dTerrain& v1 = vertices[vIdx++];
                Vertex3dTerrain& v2 = vertices[vIdx++];
                Vertex3dTerrain& v3 = vertices[vIdx++];
                Vertex3dTerrain& v4 = vertices[vIdx++];

                float x0 = offsets[0].x + j;
                float z0 = offsets[0].y + i;

                v1.position = { x0,   0.0f, z0   };
                v2.position = { x0,   0.0f, z0+1 };
                v3.position = { x0+1, 0.0f, z0+1 };
                v4.position = { x0+1, 0.0f, z0   };

                v1.texture = { du * x0,     dv * z0 };
                v2.texture = { du * x0,     dv * (z0+1) };
                v3.texture = { du * (x0+1), dv * (z0+1) };
                v4.texture = { du * (x0+1), dv * z0 };
            }
        }

        return true;
    }
    catch (EngineException& e)
    {
        LogErr(e);
        return false;
    }
}

//////////////////////////////////////////////////////////

void BuildFlatGridVertices(
    Vertex3D* vertices,
    const int width,
    const int depth,
    const int vertByX,   // vertices count by X
    const int vertByZ)
{
    try
    {
        //
        // Create grid vertices
        //
        const float halfWidth = 0.5f * (float)width;
        const float halfDepth = 0.5f * (float)depth;

        const int quadsByX    = vertByX - 1;
        const int quadsByZ    = vertByZ - 1;
        const float du        = 1.0f / (float)quadsByX; // texture stride by X for each quad
        const float dv        = 1.0f / (float)quadsByZ; // texture stride by Y for each quad
        const float dx        = (float)width * du;             // how many quads we can put in such width
        const float dz        = (float)depth * dv;             // how many quads we can put in such depth

        // precompute X-coords (of position) and tu-component (of texture) for each quad 
        float* quadsXCoords = new float[vertByX];
        float* quadsTU      = new float[vertByX];

        for (int j = 0; j < vertByX; ++j)
            quadsXCoords[j] = j * dx - halfWidth;

        for (int j = 0; j < vertByX; ++j)
            quadsTU[j] = j * du;


        // make vertices for the grid
        for (int i = 0; i < vertByZ; ++i)
        {
            const float z = halfDepth - i * dz;
            const float tv = i * dv;
            const int lineIdx = i * vertByZ;

            for (int j = 0; j < vertByX; ++j)
            {
                Vertex3D& vertex = vertices[lineIdx + j];

                vertex.position = DirectX::XMFLOAT3(quadsXCoords[j], 0.0f, z);
                vertex.texture  = DirectX::XMFLOAT2(quadsTU[j], tv);
                vertex.normal   = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
                vertex.tangent  = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
            }
        }

        // delete transient data
        SafeDeleteArr(quadsXCoords);
        SafeDeleteArr(quadsTU);
    }
    catch (const std::bad_alloc& e)
    {
        LogErr(e.what());
        LogErr("can't allocate memory during a flat grid vertices creation");
        throw EngineException("can't allocate memory during a flat grid vertices creation");
    }
}

//////////////////////////////////////////////////////////

#if 0
void BuildFlatGridIndices(
    UINT* indices,
    const int verticesByX,
    const int verticesByZ)
{
    //
    // Create grid indices 
    // 
    // (row: i, column: j)
    // ABC = ((i*n) + j,    (i*n) + j+1,  (i+1)*n + j)
    // CBD = ((i+1)*n +j,   (i*n) + j+1,  (i+1)*n + j+1)
    //
    //  A(i,j)  _______________ B(i,j+1)  
    //          |          /  |
    //          |        /    |
    //          |      /      | ij-th QUAD
    //          |    /        |
    //          |  /          |
    // C(i+1,j) |/____________| D(i+1,j+1)
    //

    // iterate over each quad and compute indices
    int k = 0;
    const int m = verticesByX;
    const int n = verticesByZ;

    for (int i = 0; i < m - 1; ++i)
    {
        for (int j = 0; j < n - 1; ++j)
        {
            // first triangle
            indices[k] = i * n + j;
            indices[k + 1] = i * n + j + 1;
            indices[k + 2] = (i + 1) * n + j;

            // second triangle
            indices[k + 3] = indices[k + 2];     // (i+1)*n + j
            indices[k + 4] = indices[k + 1];     // i*n+j+1
            indices[k + 5] = indices[k + 3] + 1; // (i + 1)*n + j + 1;

            k += 6;  // next quad
        }
    }
}
#endif

//////////////////////////////////////////////////////////

void BuildFlatGridIndices(
    UINT* indices,
    const int width,
    const int depth)
{
    int k = 0;
    int quadIdx = 0;

    for (int i = 0; i < (int)depth; ++i)
    {
        // push indices for quads by X
        for (int j = 0; j < (int)width; ++j)
        {
            int offset = (4 * quadIdx);
        
            // first triangle
            indices[k]   = offset + 0;
            indices[k+1] = offset + 1;
            indices[k+2] = offset + 2;

            indices[k+3] = offset + 0;
            indices[k+4] = offset + 2;
            indices[k+5] = offset + 3;

            k += 6;
            quadIdx++;
        }
    }
}

//////////////////////////////////////////////////////////

#if 0
void GeometryGenerator::GenerateFlatGrid(
    const int w,                          // width
    const int d,                          // depth
    BasicModel& model)                    // output model with generated vertices/indices
{
    const int width = (w > 0) ? w : 1;
    const int depth = (d > 0) ? d : 1;
    const int vertByX = width + 1;
    const int vertByZ = depth + 1;

    const int quadsByX = width;
    const int quadsByZ = depth;
    const int numFaces = quadsByX * quadsByZ * 2;   // 2 - two faces per each quad
    const int numVertices = vertByX * vertByZ;
    const int numIndices = numFaces * 3;          // 3 - three indices per each face (triangle)
    const int numSubsets = 1;                     // grid is a solid mesh

    try
    {
        // alloc memory for vertices/indices/AABBs of the grid
        model.AllocateMemory(numVertices, numIndices, numSubsets);

        BuildFlatGridVertices(model.vertices_, (float)width, (float)depth);

    }
    catch (std::bad_alloc& e)
    {
        LogErr(e.what());
        throw EngineException("can't allocate memory for a flat grid");
    }
}
#endif


void GeometryGenerator::GenerateTerrainFlatGrid(
    const int width,              // length by X-axis
    const int depth,              // length by Z-axis
    const int verticesByX,        // vertices count by X
    const int verticesByZ,        // vertices count by Z
    Terrain& terrain)
{
    // build the grid in the XZ-plane. A grid of (m * n) vertices includes
    // (m - 1) * (n - 1) quads (or cells). Each cell will be covered by two triangles, 
    // so there is total of 2 * (m - 1) * (n - 1) triangles. 
    //
    // If the grid has width w and depth d, 
    // the cell spacing along the x-asis is dx = w/(n - 1) and 
    // the cell spacing along the z-axis is dz = d/(m - 1).
    // To generate vertices, we start at the upper left corner and incrementally compute
    // the vertex coordinates row-by-row.
    //
    // The coordinates of the ij-th grid vertex in the xz-plane are given by:
    //              Vij = [-0.5*width + j*dx, 0.0, 0.5*depth - i*dz];

    try 
    {
        // check input params
        CAssert::True(width > 0,        "input width must be > 0");
        CAssert::True(depth > 0,        "input depth must be > 0");
        CAssert::True(verticesByX >= 2, "input number of vertices by X-axis must be >= 2");
        CAssert::True(verticesByZ >= 2, "input number of vertices by Z-axis must be >= 2");

        const int quadsByX    = verticesByX - 1;
        const int quadsByZ    = verticesByZ - 1;
        const int faceCount   = quadsByX * quadsByZ * 2;        // quad_num_by_X * quad_num_by_Z * 2_triangles_per_quad
        const int numVertices = verticesByX * verticesByZ * 4;  // 4 vertices per quad
        const int numIndices  = numVertices / 4 * 6;

        // allocate memory for data of the grid
        terrain.AllocateMemory(numVertices, numIndices);

        BuildFlatGridVertices(terrain.vertices_, (float)width, (float)depth);
        BuildFlatGridIndices(terrain.indices_, verticesByX, verticesByZ);
    }
    catch (std::bad_alloc& e)
    {
        LogErr(e.what());
        LogErr("can't allocate memory for a terrain's flat grid");
        return;
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr("can't generate flat grid for terrain");
        return;
    }
}

//////////////////////////////////////////////////////////

void GeometryGenerator::GeneratePyramid(
    const float height,                                // height of the pyramid
    const float baseWidth,                             // width (length by X) of one of the base side
    const float baseDepth,                             // depth (length by Z) of one of the base side
    BasicModel& model)
{
    // construct a pyramid by the input height, baseWidth, baseDepth,
    // and stores its vertices and indices into the meshData variable;

    constexpr int verticesOfSides = 12;
    constexpr int numVertices = 16;
    constexpr int numIndices = 18;
    constexpr int numSubsets = 1;        // only one mesh

    const float halfBaseWidth = 0.5f * baseWidth;
    const float halfBaseDepth = 0.5f * baseDepth;

    // -------------------------------------------------- //

    // create a tip vertex 
    Vertex3D tipVertex;

    tipVertex.position = { 0, height, 0 };
    tipVertex.texture = { 0.5f, 0 };   // upper center of texture

    // -------------------------------------------------- //

    // allocate memory for the pyramid data
    model.AllocateMemory(numVertices, numIndices, numSubsets);
    Vertex3D* vertices = model.vertices_;

    const DirectX::XMFLOAT3 basePositions[4] =
    {
        { -baseWidth, 0, +baseDepth },
        { -baseWidth, 0, -baseDepth },
        { +baseWidth, 0, -baseDepth },
        { +baseWidth, 0, +baseDepth }
    };

    // first side
    vertices[0] = tipVertex;
    vertices[1].position = basePositions[0];
    vertices[2].position = basePositions[1];

    // second side
    vertices[3] = tipVertex;
    vertices[4].position = basePositions[1];
    vertices[5].position = basePositions[2];

    // third side
    vertices[6] = tipVertex;
    vertices[7].position = basePositions[2];
    vertices[8].position = basePositions[3];

    // fourth side
    vertices[9] = tipVertex;
    vertices[10].position = basePositions[3];
    vertices[11].position = basePositions[0];

    // -------------------------------------------------- //

    // set texture coords and colors for vertices of the pyramid's sides
    for (int idx = 0; idx < verticesOfSides; idx += 3)
    {
        vertices[idx + 1].texture = { 0, 1 };   // bottom left of texture
        vertices[idx + 2].texture = { 1, 1 };   // bottom right of texture
    }

    // bottom
    const DirectX::XMFLOAT2 bottomTexCoords[4] = {{1, 0},{1, 1},{0, 1},{0, 0}};

    for (int v_idx = verticesOfSides, data_idx = 0; v_idx < numVertices; ++v_idx, ++data_idx)
    {
        vertices[v_idx].position = basePositions[data_idx];
        vertices[v_idx].texture  = bottomTexCoords[data_idx];
        vertices[v_idx].normal   = { 0, -1, 0 };               // bottom normal vector
    }

    //
    // create indices data for the pyramid
    //
    const UINT indicesData[numIndices] = {

        // sides
        0, 2, 1,
        3, 5, 4,
        6, 8, 7,
        9, 11, 10,

        // bottom
        13, 14, 12,
        14, 15, 12,
    };

    model.CopyIndices(indicesData, numIndices);
}

//////////////////////////////////////////////////////////

void GeometryGenerator::GenerateCylinder(
    const MeshCylinderParams& params,
    BasicModel& model)
{
    // generate a cylinder centered at the origin, parallel to the Y-axis.
    // All the vertices lie on the "rings" of the cylinder, where there are stackCount + 1
    // rings, and each ring has sliceCount unique vertices. The difference in radius between
    // consecutive rings is 
    //               delta_r = (toRadius - bottomRadius) / stackCount;
    //
    // If we start at the bottom ring with index 0, then the radius of the i-th ring is
    //                      Ri = bottomRadius + i*delta_r
    // and height of the i-th ring is
    //                      Hi = -(h/2) + i*delta_h,
    // where delta_h is the stack height and h is the cylinder height. So the basic idea is
    // to iterato over each ring and generate the vertices that lie on that ring.

    try
    {
        // check input data
        assert(params.bottomRadius_ > 0);
        assert(params.topRadius_ > 0);
        assert(params.height_ > 0);
        assert(params.sliceCount_ > 0);
        assert(params.stackCount_ > 0);

        // Add one because we duplicate the first and last vertex per ring
        // since the texture coordinates are different
        const int ringVertexCount = params.sliceCount_ + 1;

        const float du = 1.0f / params.sliceCount_;
        const float dTheta = DirectX::XM_2PI * du;        // delta theta

        CylinderTempData tempData;
        tempData.tu           = new float[ringVertexCount]{0.0f};
        tempData.thetaSines   = new float[ringVertexCount]{0.0f};
        tempData.thetaCosines = new float[ringVertexCount]{0.0f};

        // precompute texture coords by X
        for (int j = 0; j <= params.sliceCount_; ++j)
            tempData.tu[j] = (float)j * du;

        // precompute sines/cosines of Theta
        for (int j = 0; j <= params.sliceCount_; ++j)
            tempData.thetaSines[j] = sinf(j * dTheta);

        for (int j = 0; j <= params.sliceCount_; ++j)
            tempData.thetaCosines[j] = cosf(j * dTheta);


        // allocate memory for the model data
        const int numRings = params.stackCount_ + 1;
        const int numVerticesInRing = params.sliceCount_ + 1;

        const int numVerticesCylSide = ringVertexCount * numVerticesInRing;
        const int numVerticesTopCap = numVerticesInRing + 1;   // ring + center
        const int numVerticesBottomCap = numVerticesTopCap;

        int numVertices = numVerticesCylSide + numVerticesTopCap + numVerticesBottomCap;
        int numIndices = numVertices * 6;
        int numSubsets = 1;                                    // only one mesh

        model.AllocateMemory(numVertices, numIndices, numSubsets);


        //
        // create 3 main parts of cylinder: side, top cap, bottom cap
        //
        BuildCylinderStacks(params, tempData, model);
        BuildCylinderTopCap(params, tempData, model);
        BuildCylinderBottomCap(params, tempData, model);

        // ----------------------------------- 

        // release the memory
        SafeDeleteArr(tempData.tu);
        SafeDeleteArr(tempData.thetaSines);
        SafeDeleteArr(tempData.thetaCosines);

    }
    catch (const std::bad_alloc& e)
    {
        LogErr(e.what());
        LogErr("can't allocate memory for some data");
    }

}

///////////////////////////////////////////////////////////

void BuildSphereVertices(
    Vertex3D* vertices, 
    int& idx, 
    const MeshSphereParams& params,
    const int numVertices)
{
    const float radius = params.radius_;
    const int sliceCount = params.sliceCount_;
    const int stackCount = params.stackCount_;

    const float du = 1.0f / sliceCount;
    const float dv = 1.0f / stackCount;

    const float dTheta = DirectX::XM_2PI * du;   // horizontal ring angles delta
    const float dAlpha = DirectX::XM_PI * dv;    // vertical angle delta

    // allocate memory for the transient data
    float* tu = new float[sliceCount + 1];               // texture X coords
    float* thetaSines = new float[sliceCount + 1];
    float* thetaÑosines = new float[sliceCount + 1];

    // ------------------------------------------

    // precompute texture X coord
    for (int j = 0; j <= sliceCount; ++j)
        tu[j] = j * du;

    // precompute sin/cos of Theta
    for (int j = 0; j <= sliceCount; ++j)
        thetaSines[j] = sinf(j * dTheta);

    for (int j = 0; j <= sliceCount; ++j)
        thetaÑosines[j] = cosf(j * dTheta);


    // build vertices
    for (int i = 0; i < stackCount; ++i)
    {
        // vertical angle from bottom to top
        const float curAlpha = -DirectX::XM_PIDIV2 + (float)(i + 1) * dAlpha;

        // radius and height of the current ring
        const float r = radius * cosf(curAlpha);
        const float y = radius * sinf(curAlpha);

        // Y coord of the texture
        const float tv = 1.0f - (float)(i + 1) * dv;

        // make vertices for this ring
        for (int j = 0; j <= sliceCount; ++j)
        {
            vertices[idx].position = { r * thetaÑosines[j], y, r * thetaSines[j] };
            vertices[idx].texture = { tu[j], tv };

            ++idx;
        }
    }

    // make the lowest vertex of the sphere
    vertices[idx].position = { 0, -radius, 0 };
    vertices[idx].texture = { 0.5f, 1.0f };
    ++idx;

    // ------------------------------------------

    // compute normalized normal vectors based on a vector
    // from the center to some vertex of the sphere
    for (int i = 0; i < numVertices; ++i)
    {
        Vertex3D& v = vertices[i];
        const XMVECTOR normVec = DirectX::XMVector3Normalize({ v.position.x, v.position.y, v.position.z });
        DirectX::XMStoreFloat3(&v.normal, normVec);
    }

    // release the memory from transient data
    SafeDeleteArr(tu);
    SafeDeleteArr(thetaSines);
    SafeDeleteArr(thetaÑosines);
}

///////////////////////////////////////////////////////////

void BuildSphereIndices(
    UINT* indices, 
    const int centerIdx,   // index of the lowest center vertex
    const int sliceCount,
    const int stackCount)
{
    int indexIdx = 0;

    //
    // build indices for the sphere
    //

    // Add one because we duplicate the first and last vertex per ring
    // since the texture coordinates are different
    const UINT ringVertexCount = sliceCount + 1;

    for (int i = 0; i < stackCount - 1; ++i)
    {
        for (int j = 0; j < sliceCount; ++j)
        {
            const UINT idx_1 = i * ringVertexCount + j;
            const UINT idx_2 = (i + 1) * ringVertexCount + j;
            const UINT idx_3 = idx_2 + 1;
            const UINT idx_4 = idx_1 + 1;

            // first triangle of the sphere face
            indices[indexIdx + 0] = idx_1;
            indices[indexIdx + 1] = idx_2;
            indices[indexIdx + 2] = idx_3;

            // second triangle of the sphere face
            indices[indexIdx + 3] = idx_1;
            indices[indexIdx + 4] = idx_3;
            indices[indexIdx + 5] = idx_4;

            indexIdx += 6;
        }
    }

    //
    // setup indices of the sphere's lowest part
    //

    // build faces for bottom of the sphere
    for (int i = 0; i < sliceCount; ++i)
    {
        indices[indexIdx + 0] = centerIdx;
        indices[indexIdx + 1] = i;
        indices[indexIdx + 2] = i + 1;

        indexIdx += 3;
    }
}

///////////////////////////////////////////////////////////

void GeometryGenerator::GenerateSphere(
    const MeshSphereParams& params,
    BasicModel& model)
{
    // create data for the sphere mesh by specifying its radius, and
    // the slice and stack count. The algorithm for generation the sphere is very similar to 
    // that of the cylinder, except that the radius per ring changes is a nonlinear way
    // based on trigonometric functions

    // check input params
    assert(params.radius_ > 0);
    assert(params.sliceCount_ > 0);
    assert(params.stackCount_ > 0);

    const float radius = params.radius_;
    const int sliceCount = params.sliceCount_;
    const int stackCount = params.stackCount_;

    const int numVertices = stackCount * sliceCount + 1 + sliceCount;  // +1 because of lowest center vertex
    const int numIndices = numVertices * 6 + (sliceCount * 3);         // + (sliceCount * 3) -- because of the lowest part of sphere
    const int numSubsets = 1;                                          // only one mesh
    int vertexIdx = 0;

    // prepare memory for the model's data
    model.AllocateMemory(numVertices, numIndices, numSubsets);

    BuildSphereVertices(model.vertices_, vertexIdx, params, numVertices);
    BuildSphereIndices(model.indices_, vertexIdx - 1, sliceCount, stackCount);

    ComputeTangents(model.vertices_, model.indices_, model.GetNumIndices());
}

///////////////////////////////////////////////////////////

void GeometryGenerator::GenerateSkyDome(
    const float radius_,
    const int sliceCount_,
    const int stackCount_,
    BasicModel& model)
{
    using namespace DirectX;

    // if some input param is invalid we set it to the default
    const float radius = (radius_ > 0) ? radius_ : 1;
    const int sliceCount = (sliceCount_ > 2) ? sliceCount_ : 3;
    const int stackCount = (stackCount_ > 2) ? stackCount_ : 3;

    // Poles: note that there will be texture coordinate distortion as there is
    // not a unique point on the texture map to assign to the pole when mapping
    // a rectangular texture onto a sphere.
    Vertex3D topVertex; 
    Vertex3D bottomVertex; 

    topVertex.position    = { 0.0f, +radius, 0.0f };
    bottomVertex.position = { 0.0f, -radius, 0.0f };

    const float phiStep   = XM_PIDIV2  / stackCount;  // vertical
    const float thetaStep = XM_2PI / sliceCount;  // horizontal

#if 0
    float* thetaSines   = new float[sliceCount + 1];
    float* thetaCosines = new float[sliceCount + 1];

    // precompute sin/cos of Theta
    for (int j = 0; j <= sliceCount; ++j)
        thetaSines[j] = sinf(j * thetaStep);

    for (int j = 0; j <= sliceCount; ++j)
        thetaCosines[j] = cosf(j * thetaStep);
#endif

    const int numVertices = stackCount * (sliceCount + 1);
    const int numIndices = numVertices * 6 + (sliceCount * 3);

    model.AllocateMemory(numVertices, numIndices, 1);

    int verticesCount = 0;
    int vIdx = 0;            // index of the current vertex
    int iIdx = 0;      

    const float du = 1.0f / sliceCount;
    const float dv = 1.0f / stackCount;

    Vertex3D* vertices = model.vertices_;
    UINT* indices = model.indices_;


    vertices[vIdx++] = topVertex;

    // compute vertices for each stack ring (do not count the poles as rights)
    for (int i = 1; i <= stackCount-1; ++i)
    {
        float phi = i * phiStep;

        // vertices of ring
        for (int j = 0; j <= sliceCount; ++j)
        {
            float theta = j * thetaStep;

            Vertex3D& v = vertices[vIdx++];
            verticesCount++;

            v.position.x = radius * sinf(phi) * cosf(theta);
            v.position.y = radius * cosf(phi);
            v.position.z = radius * sinf(phi) * sinf(theta);

            const float tu = j * du;
            const float tv = (float)(i + 1) * dv;
            
            v.texture = { tu, tv };

            //v.texture.x = theta / XM_2PI;
            //v.texture.y = phi / XM_PIDIV2;
        }
    }

    vertices[vIdx] = bottomVertex;

    sprintf(g_String, "sky dome has %d vertices", verticesCount);
    LogErr(g_String);


    //
    // Compute indices for top stack.  The top stack was written first to the vertex buffer
    // and connects the top pole to the first ring.
    //

    for (UINT i = 1; i <= (UINT)sliceCount; ++i)
    {
        indices[iIdx++] = 0;
        indices[iIdx++] = i + 1;
        indices[iIdx++] = i;
    }

    //
    // Compute indices for inner stacks (not connected to poles).
    //

    // Offset the indices to the index of the first vertex in the first ring.
    // This is just skipping the top pole vertex.
    UINT baseIndex = 1;
    UINT ringVertexCount = sliceCount+1;

    for (UINT i = 0; i < (UINT)stackCount - 2; ++i)
    {
        for (UINT j = 0; j < (UINT)sliceCount; ++j)
        {
            indices[iIdx++] = baseIndex + i*ringVertexCount + j;
            indices[iIdx++] = baseIndex + i * ringVertexCount + j+1;
            indices[iIdx++] = baseIndex + (i+1) * ringVertexCount + j;

            indices[iIdx++] = baseIndex + (i + 1) * ringVertexCount + j;
            indices[iIdx++] = baseIndex + i * ringVertexCount + j+1;
            indices[iIdx++] = baseIndex + (i+1) * ringVertexCount + j+1;
        }
    }

    //
    // Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
    // and connects the bottom pole to the bottom ring.
    //

    // South pole vertex was added last.
    UINT southPoleIndex = (UINT)vIdx;

    // Offset the indices to the index of the first vertex in the last ring.
    baseIndex = southPoleIndex - ringVertexCount;

    for (UINT i = 0; i <= (UINT)sliceCount; ++i)
    {
        indices[iIdx++] = southPoleIndex;
        indices[iIdx++] = baseIndex + i;
        indices[iIdx++] = baseIndex + i + 1;
    }


    //SafeDeleteArr(thetaSines);
    //SafeDeleteArr(thetaCosines);
}

///////////////////////////////////////////////////////////

void GeometryGenerator::GenerateGeosphere(
    const float radius,
    int numSubdivisions,
    BasicModel& model)
{
    // A geosphere approximates a sphere using 
    // triangles with almost equal areas as well as equal side length.
    // To generate a geosphere, we start with an icosahedron, subdivide the triangles, and
    // then project the new vertices onto the sphere with the given radius. We can repeat
    // this process to improve the tessellation.
    // The new vertices are found just by taking the midpoints along the edges of the 
    // original triangle. The new vertices can then be projected onto a sphere of radius R
    // by projection the vertices onto the unit sphere an then scalar multiplying by R:
    //                          v' = R * normalize(v)

    // put a cap on the number of subdivisition
    numSubdivisions = min(numSubdivisions, 5u);

    // approximate a sphere by tesselating an icosahedron
    const float X = 0.525731f;
    const float Z = 0.850651f;

    const int numVertices = 12;
    const int numIndices = 60;
    const int numSubsets = 1;    // only one mesh

    const DirectX::XMFLOAT3 pos[numVertices] =
    {
        {-X, 0, Z},  {X, 0, Z},
        {-X, 0, -Z}, {X, 0, -Z},
        {0, Z, X},   {0, Z, -X},
        {0, -Z, X},  {0, -Z, -X},
        {Z, X, 0},   {-Z, X, 0},
        {Z, -X, 0},  {-Z, -X, 0}
    };

    const UINT indicesData[numIndices] =
    {
        1,4,0,   4,9,0,   4,5,9,  8,5,4,   1,8,4,
        1,10,8,  10,3,8,  8,3,5,  3,2,5,   3,7,2,
        3,10,7,  10,6,7,  6,11,7, 6,0,11,  6,1,0,
        10,1,6,  11,0,9,  2,11,9, 5,2,9,   11,2,7
    };

    
    // allocate memory for the geosphere's data
    model.AllocateMemory(numVertices, numIndices, numSubsets);

    // setup vertices positions
    //memcpy(model.vertices_, pos, sizeof(XMFLOAT3) * numVertices);
    //std::copy(pos, pos + numVertices, model.vertices_);
    for (int i = 0; i < numVertices; ++i)
        model.vertices_[i].position = pos[i];

    // setup indices data
    model.CopyIndices(indicesData, numIndices);

    // divide each triangle of sphere into smaller ones
    for (int i = 0; i < numSubdivisions; ++i)
        Subdivide(model);

    Vertex3D* vertices = model.vertices_;

    // project vertices onto the sphere and scale
    for (u32 i = 0; i < model.numVertices_; ++i)
    {
        DirectX::XMFLOAT3& pos = vertices[i].position;

        // project onto unit sphere
        const DirectX::XMVECTOR N = DirectX::XMLoadFloat3(&pos);
        const DirectX::XMVECTOR n = DirectX::XMVector3Normalize(N);

        // store the normal vector
        DirectX::XMStoreFloat3(&vertices[i].normal, n);

        // compute and store position of vertex
        DirectX::XMStoreFloat3(&pos, DirectX::XMVectorScale(n, radius));

        // derive texture coordinates from spherical coordinates
        const float theta = MathHelper::AngleFromXY(pos.x, pos.z);
        const float phi = acosf(pos.y / radius);

        vertices[i].texture.x = theta / DirectX::XM_2PI;
        vertices[i].texture.y = phi / DirectX::XM_PI;

        // partial derivative of P with respect to theta
        const float tangX = radius * sinf(phi);
        const DirectX::XMVECTOR T = { -tangX * sinf(theta), 0, +tangX * cosf(theta) };

        // normalize the tangent
        DirectX::XMStoreFloat3(&vertices[i].tangent, DirectX::XMVector3Normalize(T));
    }
}

// *********************************************************************************
// 
//                               PRIVATE FUNCTIONS
// 
// *********************************************************************************

void GeometryGenerator::ComputeAABB(
    const Vertex3D* vertices,
    const int numVertices,
    DirectX::BoundingBox& aabb)
{
    using namespace DirectX;

    // compute the bounding box of the mesh
    XMVECTOR vMin{ FLT_MAX, FLT_MAX, FLT_MAX };
    XMVECTOR vMax{ FLT_MIN, FLT_MIN, FLT_MIN };

    for (int i = 0; i < numVertices; ++i)
    {
        XMVECTOR P = XMLoadFloat3(&vertices[i].position);
        vMin = DirectX::XMVectorMin(vMin, P);
        vMax = DirectX::XMVectorMax(vMax, P);
    }

    // convert min/max representation to center and extents representation
    DirectX::XMStoreFloat3(&aabb.Center, 0.5f * (vMin + vMax));
    DirectX::XMStoreFloat3(&aabb.Extents, 0.5f * (vMax - vMin));
}

///////////////////////////////////////////////////////////

void GeometryGenerator::SetupCubeVerticesPositions(DirectX::XMFLOAT3* positions)
{
    // setup the vertices positions of a cube
    
    // right side
    positions[0] = { 1,  1, -1 };  // near top
    positions[1] = { 1, -1, -1 };  // near bottom
    positions[2] = { 1,  1,  1 };  // far top
    positions[3] = { 1, -1,  1 };  // far bottom

    // left side
    positions[4] = { -1,  1, -1 }; // near top
    positions[5] = { -1, -1, -1 }; // near bottom
    positions[6] = { -1,  1,  1 }; // far top 
    positions[7] = { -1, -1,  1 }; // far bottom
}

///////////////////////////////////////////////////////////

void GeometryGenerator::SetupCubeFacesNormals(DirectX::XMFLOAT3* facesNormals)
{
    // setup the faces normals of a cube

    facesNormals[0] = { 0,  0, -1}, // front
    facesNormals[1] = { 0,  0, +1}, // back
    facesNormals[2] = {-1,  0,  0}, // left
    facesNormals[3] = {+1,  0,  0}, // right
    facesNormals[4] = { 0, +1,  0}, // top
    facesNormals[5] = { 0, -1,  0}; // bottom
    
}

///////////////////////////////////////////////////////////

void GeometryGenerator::BuildCylinderStacks(
    const MeshCylinderParams& params,
    CylinderTempData& tempData,
    BasicModel& model)
{
    const float dv = 1.0f / params.stackCount_;

    // (delta_h)
    const float stackHeight = params.height_ * dv;

    // (delta_r) amount to increment radius as we move up each stack level from bottom to top 
    const float dr = (params.topRadius_ - params.bottomRadius_);

    const float radiusStep = dr * dv;
    const float halfHeight = -0.5f * params.height_;
    
    const int ringCount = params.stackCount_ + 1;

    
    // Add one because we duplicate the first and last vertex per ring
    // since the texture coordinates are different
    const int ringVertexCount = params.sliceCount_ + 1;

    int& vertexIdx = tempData.lastVertexIdx;
    int& indexIdx = tempData.lastIndexIdx;

    // compute vertices for each stack ring starting at the bottom and moving up
    for (int i = 0; i < ringCount; ++i)
    {
        const float y = halfHeight + i*stackHeight;              // Hi = -(h/2) + i*delta_h,
        const float r = params.bottomRadius_ + i*radiusStep;     // Ri = bottomRadius + i*delta_r
        const float tv = 1.0f - (float)(i * dv);                 // Y (vertical) coord of the texture

        // vertices of ring
        for (int j = 0; j < ringVertexCount; ++j)
        {
            Vertex3D vertex;

            const float c = tempData.thetaCosines[j];
            const float s = tempData.thetaSines[j];

            vertex.position = DirectX::XMFLOAT3(r*c, y, r*s);

            vertex.texture.x = tempData.tu[j];
            vertex.texture.y = tv;

            // Cylinder can be parametrized as follows, where we introduce v parameter
            // that does in the same direction as the v tex-coord so that the bitangent
            // goes in the same direction as the v tex-coord
            //    Let r0 be the bottom radius and let r1 be the top radius.
            //    y(v) = h - hv for v in [0,1]
            //    r(v) = r1 + (r0-r1)*v
            //
            //    x(t, v) = r(v)*cos(t)
            //    y(t, v) = h - hv
            //    z(t, v) = r(v)*sin(t)
            //
            // dx/dt = -r(v)*sin(t)
            // dy/dt = 0
            // dz/dt = +r(v)*cos(t)
            //
            // dx/dv = (r0-r1)*cos(t)
            // dy/dv = -h
            // dz/dv = (r0-r1)*sin(t)

            // set tangent is unit length, and set the binormal
            vertex.tangent = DirectX::XMFLOAT3(-s, 0.0f, c);

            //const float dr = bottomRadius - topRadius;
            const XMFLOAT3 binormal = DirectX::XMFLOAT3(-dr * c, -params.height_, -dr * s);

            // compute the normal vector
            const DirectX::XMVECTOR T = DirectX::XMLoadFloat3(&vertex.tangent);
            const DirectX::XMVECTOR B = DirectX::XMLoadFloat3(&binormal);
            const DirectX::XMVECTOR N = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(T, B));
            DirectX::XMStoreFloat3(&vertex.normal, N);

            // store this vertex
            model.vertices_[vertexIdx++] = vertex;
        }
    }


    //
    // Create cylinder stacks' indices 
    // (ring: i, slice: j)
    // ABC = (i*n + j,   (i+1)*n + j,    (i+1)*n + j+1)
    // ACD = (i*n + j,   (i+1)*n + j+1,  i*n + j+1)
    //
    //  B(i+1,j)  _______________ C(i+1,j+1)  
    //            |          /  |
    //            |        /    |
    //            |      /      | ij-th QUAD OF CYLINDER
    //            |    /        |
    //            |  /          |
    //   A(i,j)   |/____________| D(i,j+1)

    UINT* indices = model.indices_;

    // compute indices for each stack
    for (int i = 0; i < params.stackCount_; ++i)
    {
        for (int j = 0; j < params.sliceCount_; ++j)
        {
            const int idx_1 = i*ringVertexCount + j;
            const int idx_2 = (i + 1)*ringVertexCount + j;
            const int idx_3 = (i + 1)*ringVertexCount + j + 1;
            const int idx_4 = i*ringVertexCount + j + 1;

            // ABC
            indices[indexIdx + 0] = idx_1;
            indices[indexIdx + 1] = idx_2;
            indices[indexIdx + 2] = idx_3;

            // ACD
            indices[indexIdx + 3] = idx_1;
            indices[indexIdx + 4] = idx_3;
            indices[indexIdx + 5] = idx_4;

            indexIdx += 6;
        }
    }
}

///////////////////////////////////////////////////////////

void GeometryGenerator::BuildCylinderCapRingVertices(
    const MeshCylinderParams& params,
    const bool isTopCap,
    CylinderTempData& tempData,
    BasicModel& model)
{
    // generate vertices of the top/bottom cap of cylinder

    int& vertexIdx = tempData.lastVertexIdx;
    const float inv_height = 1.0f / params.height_;
    const float halfHeight = 0.5f * params.height_;
    float posY = 0.0f;
    float radius = 1.0f;
    float normalY = 1.0f;

    // setup some params accoring to the cap type
    if (isTopCap)
    {
        posY = halfHeight;
        radius = params.topRadius_;
        normalY = 1.0f;
    }
    // we will build a bottom cap
    else
    {
        posY = -halfHeight;
        radius = params.bottomRadius_;
        normalY = -1.0f;
    }
    
    // duplicate bottom cap ring vertices because the texture coordinates and normals differ
    for (int i = 0; i <= params.sliceCount_; ++i)
    {
        const float posX = radius * tempData.thetaCosines[i];
        const float posZ = radius * tempData.thetaSines[i];

        // scale down by the height to try and make cap texture coord
        // area proportional to base
        const float u = posX * inv_height + 0.5f;
        const float v = posZ * inv_height + 0.5f;

        // make a vertex of the cap
        Vertex3D vertex;
        vertex.position = { posX, posY, posZ };
        vertex.texture  = { u, v };
        vertex.normal   = { 0, normalY, 0 };
        vertex.tangent  = { 1, 0, 0 };

        // store this vertex
        model.vertices_[vertexIdx++] = vertex;
    }

    // cap center vertex
    Vertex3D center;
    center.position = { 0, posY, 0 };
    center.texture  = { 0.5f, 0.5f };
    center.normal   = { 0, normalY, 0 };
    center.tangent  = { 1, 0, 0 };

    model.vertices_[vertexIdx++] = center;
}

///////////////////////////////////////////////////////////

void GeometryGenerator::BuildCylinderTopCap(
    const MeshCylinderParams& params,
    CylinderTempData& tempData,
    BasicModel& model)
{
    // generate the cylinder cap geometry amounts to generating the slice
    // triangles of the top/bottom rings to approximate a circle

    const int baseIndex = tempData.lastVertexIdx;
    int& indexIdx = tempData.lastIndexIdx;
    UINT* indices = model.indices_;
    
    BuildCylinderCapRingVertices(params, true, tempData, model);

    // index of center vertex
    const int centerIndex = tempData.lastVertexIdx - 1;

    // make top cap faces
    for (int i = 0; i < params.sliceCount_; ++i)
    {
        indices[indexIdx + 0] = centerIndex;
        indices[indexIdx + 1] = baseIndex + i + 1;
        indices[indexIdx + 2] = baseIndex + i;

        indexIdx += 3;
    }
}

///////////////////////////////////////////////////////////

void GeometryGenerator::BuildCylinderBottomCap(
    const MeshCylinderParams& params,
    CylinderTempData& tempData,
    BasicModel& model)
{
    // generate the cylinder cap geometry amounts to generating the slice
    // triangles of the top/bottom rings to approximate a circle

    const int baseIndex = tempData.lastVertexIdx;
    int& indexIdx = tempData.lastIndexIdx;
    UINT* indices = model.indices_;

    BuildCylinderCapRingVertices(params, false, tempData, model);

    // index of center vertex
    const int centerIndex = tempData.lastVertexIdx - 1;

    // build bottom cap faces
    for (int i = 0; i < params.sliceCount_; ++i)
    {
        indices[indexIdx + 0] = baseIndex + i;
        indices[indexIdx + 1] = baseIndex + i + 1;
        indices[indexIdx + 2] = centerIndex;

        indexIdx += 3;
    }
}

///////////////////////////////////////////////////////////

void GeometryGenerator::Subdivide(BasicModel& model)
{
    // subdivide the input geosphere mesh into a smaller triangles

    int vertexIdx = 0;
    int indexIdx = 0;

    const int numVertices = model.numVertices_;
    const int numIndices = model.numIndices_;
    const int newNumVertices = numIndices * 2;      // the number of vertices/indices of the subdivided geometry
    const int newNumIndices = numIndices * 4;      
    
    // move ownership over old vertices/indices
    Vertex3D* oldVertices = model.vertices_;
    UINT* oldIndices = model.indices_;

    model.vertices_ = nullptr;
    model.indices_ = nullptr;

    // allocate memory for the new subdivided geometry
    model.AllocateVertices(newNumVertices);
    model.AllocateIndices(newNumIndices);

    model.meshes_.subsets_[0].vertexCount = newNumVertices;
    model.meshes_.subsets_[0].indexCount = newNumIndices;

    //       v1
    //       *
    //      / \
    //     /   \
    //  m0*-----*m1
    //   / \   / \
    //  /   \ /   \
    // *-----*-----*
    // v0    m2     v2

    Vertex3D* newVertices = model.vertices_;
    UINT* newIndices = model.indices_;
    
    // go through each origin triangle and divide it
    for (int i = 0; i < numIndices / 3; ++i)
    {
        // make vertices of subdivided triangle
        Vertex3D& v0 = newVertices[vertexIdx++];
        Vertex3D& v1 = newVertices[vertexIdx++];
        Vertex3D& v2 = newVertices[vertexIdx++];

        Vertex3D& m0 = newVertices[vertexIdx++];
        Vertex3D& m1 = newVertices[vertexIdx++];
        Vertex3D& m2 = newVertices[vertexIdx++];

        // copy 3 old main vertices
        const int baseIdx = i * 3;
        v0 = oldVertices[oldIndices[baseIdx + 0] ];
        v1 = oldVertices[oldIndices[baseIdx + 1] ];
        v2 = oldVertices[oldIndices[baseIdx + 2] ];

        //
        // generate midpoints
        //

        // For subdivision, we just care about the position component. We derive the other
        // vertex components in GenerateGeosphereMesh.
        m0.position = DirectX::XMFLOAT3(
            0.5f * (v0.position.x + v1.position.x),
            0.5f * (v0.position.y + v1.position.y),
            0.5f * (v0.position.z + v1.position.z));

        m1.position = DirectX::XMFLOAT3(
            0.5f * (v1.position.x + v2.position.x),
            0.5f * (v1.position.y + v2.position.y),
            0.5f * (v1.position.z + v2.position.z));

        m2.position = DirectX::XMFLOAT3(
            0.5f * (v0.position.x + v2.position.x),
            0.5f * (v0.position.y + v2.position.y),
            0.5f * (v0.position.z + v2.position.z));

        //
        // add new indices
        //
        
        const int newGeoIdx = i * 6;
    
        // make indices of subdivided triangle
        newIndices[indexIdx++] = newGeoIdx + 0;
        newIndices[indexIdx++] = newGeoIdx + 3;
        newIndices[indexIdx++] = newGeoIdx + 5;

        newIndices[indexIdx++] = newGeoIdx + 3;
        newIndices[indexIdx++] = newGeoIdx + 4;
        newIndices[indexIdx++] = newGeoIdx + 5;

        newIndices[indexIdx++] = newGeoIdx + 5;
        newIndices[indexIdx++] = newGeoIdx + 4;
        newIndices[indexIdx++] = newGeoIdx + 2;

        newIndices[indexIdx++] = newGeoIdx + 3;
        newIndices[indexIdx++] = newGeoIdx + 1;
        newIndices[indexIdx++] = newGeoIdx + 4;

    }

    SafeDeleteArr(oldVertices);
    SafeDeleteArr(oldIndices);
}

} // namespace Core
