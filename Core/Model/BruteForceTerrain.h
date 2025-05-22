#pragma once

#include "../Mesh/Vertex3dTerrain.h"
#include "../Mesh/VertexBuffer.h"
#include "../Mesh/IndexBuffer.h"
#include "TerrainBase.h"

namespace Core
{

// limit of textures for the terrain
static constexpr int MAX_NUM_TEXTURES_IN_TERRAIN = 6;        


class BruteForceTerrain : public TerrainBase
{
public:
    BruteForceTerrain() {};
    ~BruteForceTerrain() {};

    bool InitBuffers(
        ID3D11Device* pDevice,
        const Vertex3dTerrain* vertices,
        const UINT* indices,
        const int numVertices,
        const int numIndices);

    // ------------------------------------------
    // getters
    // ------------------------------------------

    inline int GetNumVertices()             const { return vb_.GetVertexCount(); }
    inline int GetNumIndices()              const { return ib_.GetIndexCount(); }
    inline int GetVertexStride()            const { return vb_.GetStride(); }

    inline ID3D11Buffer* GetVertexBuffer()  const { return vb_.Get(); }
    inline ID3D11Buffer* GetIndexBuffer()   const { ib_.Get(); }
    inline const TexID* GetTexIDs()         const { return texIDs_; }


    // ------------------------------------------
    // setters
    // ------------------------------------------

    void SetTexture(const int idx, const TexID texID);

private:
    char      name_[8] = "terrain";
    TexID     texIDs_[MAX_NUM_TEXTURES_IN_TERRAIN]{ 0 };

    VertexBuffer<Vertex3dTerrain> vb_;
    IndexBuffer<UINT> ib_;

};

} // namespace
