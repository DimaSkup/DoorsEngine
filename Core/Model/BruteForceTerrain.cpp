#include "BruteForceTerrain.h"

namespace Core
{
  
bool BruteForceTerrain::InitBuffers(
    ID3D11Device* pDevice,
    const Vertex3dTerrain* vertices,
    const UINT* indices,
    const int numVertices,
    const int numIndices)
{
    // init vertex/index buffers with input data
    try
    {
        Assert::True(vertices,        "input ptr to arr of vertices == nullptr");
        Assert::True(indices,         "input ptr to arr of indices == nullptr");
        Assert::True(numVertices > 0, "input number of vertices must be > 0");
        Assert::True(numIndices > 0,  "input number of indices must be > 0");

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

void BruteForceTerrain::SetTexture(const int idx, const TexID texID)
{
    // set texture (its ID) by input idx

    if ((idx < 0) || (idx > MAX_NUM_TEXTURES_IN_TERRAIN))
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

    // everything is ok so set the texture
    texIDs_[idx] = texID;
}

} // namespace
