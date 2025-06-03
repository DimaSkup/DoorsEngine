#include <CoreCommon/pch.h>
#include "Terrain.h"
#include "../Mesh/MaterialMgr.h"

namespace Core
{



void Terrain::ClearMemory()
{
    // release memory from the CPU copy of vertices/indices
    SafeDeleteArr(vertices_);
    SafeDeleteArr(indices_);
}

///////////////////////////////////////////////////////////

void Terrain::Shutdown()
{
    ClearMemory();

    // release memory from the vertex buffer and index buffer
    vb_.Shutdown();
    ib_.Shutdown();
}

///////////////////////////////////////////////////////////

void Terrain::AllocateMemory(const int numVertices, const int numIndices)
{
    // allocate memory for vertices/indices data arrays
    try
    {
        assert(numVertices > 0);
        assert(numIndices > 0);

        // prepare memory
        ClearMemory();

        numVertices_ = numVertices;
        numIndices_ = numIndices;

        vertices_ = new Vertex3dTerrain[numVertices_]{};
        indices_ = new UINT[numIndices_]{ 0 };
    }
    catch (const std::bad_alloc& e)
    {
        Shutdown();

        LogErr(e.what());
        LogErr("can't allocate memory for some data of the model");
        return;
    }
}

///////////////////////////////////////////////////////////
  
bool Terrain::InitBuffers(
    ID3D11Device* pDevice,
    const Vertex3dTerrain* vertices,
    const UINT* indices,
    const int numVertices,
    const int numIndices)
{
    // init vertex/index buffers with input data
    try
    {
        CAssert::True(vertices,        "input ptr to arr of vertices == nullptr");
        CAssert::True(indices,         "input ptr to arr of indices == nullptr");
        CAssert::True(numVertices > 0, "input number of vertices must be > 0");
        CAssert::True(numIndices > 0,  "input number of indices must be > 0");

        constexpr bool isDynamic = false;
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

///////////////////////////////////////////////////////////

void Terrain::SetAABB(const DirectX::XMFLOAT3& center, const DirectX::XMFLOAT3& extents)
{
    // set axis-aligned bounding box for the terrain
    center_ = center;
    extents_ = extents;
}

///////////////////////////////////////////////////////////

void Terrain::SetMaterial(const MaterialID matID)
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

void Terrain::SetTexture(const int idx, const TexID texID)
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
