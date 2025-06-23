#pragma once

#include "../Mesh/Vertex3dTerrain.h"
#include "../Mesh/VertexBuffer.h"
#include "../Mesh/IndexBuffer.h"
#include "TerrainBase.h"

namespace Core
{


class Terrain : public TerrainBase
{
public:
    Terrain()  {}
    ~Terrain() { Shutdown(); }

    ///////////////////////////////////////////////////////////

    void ClearMemory();
    void Shutdown();

    void AllocateMemory(
        const int numVertices,
        const int numIndices);

    bool InitBuffers(
        ID3D11Device* pDevice,
        const Vertex3dTerrain* vertices,
        const UINT* indices,
        const int numVertices,
        const int numIndices);

    // ------------------------------------------
    // getters
    // ------------------------------------------

    inline int GetNumVertices()             const { return numVertices_; }
    inline int GetNumIndices()              const { return numIndices_; }
    inline int GetVertexStride()            const { return vb_.GetStride(); }

    inline ID3D11Buffer* GetVertexBuffer()  const { return vb_.Get(); }
    inline ID3D11Buffer* GetIndexBuffer()   const { return ib_.Get(); }
    //inline const TexID* GetTexIDs()         const { return texIDs_; }


    // ------------------------------------------
    // setters
    // ------------------------------------------

    void SetAABB(const DirectX::XMFLOAT3& center, const DirectX::XMFLOAT3& extents);
    void SetMaterial(const MaterialID matID);
    void SetTexture(const int idx, const TexID texID);

public:
    char                name_[8] = "terrain";
    uint8_t             vertexStride_ = sizeof(Vertex3dTerrain);
    uint32_t            numVertices_ = 0;
    uint32_t            numIndices_ = 0;
    MaterialID          materialID_ = 0;

    VertexBuffer<Vertex3dTerrain> vb_;
    IndexBuffer<UINT>   ib_;
    DirectX::XMFLOAT3   center_;
    DirectX::XMFLOAT3   extents_;

    // keep CPU copy of the vertices/indices data to read from
    Vertex3dTerrain*    vertices_ = nullptr;
    UINT*               indices_  = nullptr;
};

} // namespace
