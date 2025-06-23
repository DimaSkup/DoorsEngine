// =================================================================================
// Filename:  TerrainGeomipmapped.h
// Desc:      implementation of terrain geomimapping CLOD algorithm
//            (CLOD - continuous level of detail)
//
// Created:   10.06.2025 by DimaSkup
// =================================================================================
#pragma once

#include "../Mesh/Vertex3dTerrain.h"
#include "../Mesh/VertexBuffer.h"
#include "../Mesh/IndexBuffer.h"
#include "TerrainBase.h"

namespace Core
{

// =================================================================================
// Data structures
// =================================================================================
struct GeomPatch
{
    uint32 isVisible : 1;
    uint32 distSqr   : 27;
    uint32 LOD       : 4;
};

///////////////////////////////////////////////////////////

struct GeomNeighbor
{
    uint8 left  : 1;
    uint8 up    : 1;
    uint8 right : 1;
    uint8 down  : 1;
};

// =================================================================================
// Class
// =================================================================================
class TerrainGeomipmapped : public TerrainBase
{
public:
    TerrainGeomipmapped()  {}
    ~TerrainGeomipmapped() { Shutdown(); }

    ///////////////////////////////////////////////////////////

    void ClearMemory();
    void Shutdown();

    void AllocateMemory(
        const int numVertices,
        const int numIndices);

    bool InitGeomipmapping(const int patchSize);

    bool InitBuffers(
        ID3D11Device* pDevice,
        const Vertex3dTerrain* vertices,
        const UINT* indices,
        const int numVertices,
        const int numIndices);

    void Update(const CameraParams& camParams);

    void ComputeTesselation(void);
    void ComputePatch(const int currPatchNum, const int px, const int pz);

    void ComputeFan(
        const float cx,
        const float cz,
        const float size,
        const GeomNeighbor& neighbor);

    void ComputeFanWithIndices(
        const float cx,
        const float cz,
        const float size,
        const GeomNeighbor& neighbor);

    // ------------------------------------------
    // getters
    // ------------------------------------------

    inline int GetNumVertices()                 const { return numVertices_; }
    inline int GetNumIndices()                  const { return numIndices_; }
    inline int GetVertexStride()                const { return vb_.GetStride(); }

    inline ID3D11Buffer* GetVertexBuffer()      const { return vb_.Get(); }
    inline ID3D11Buffer* GetIndexBuffer()       const { return ib_.Get(); }

    // get the number of patches being rendered per frame
    inline int GetNumPatchesPerFrame(void) const
    {   return patchesPerFrame_;   }

    // get the current patch number by input coords
    inline int GetPatchNumber(int px, int pz) const
    {   return (pz*numPatchesPerSide_) + px;   }

    // ------------------------------------------
    // setters
    // ------------------------------------------

    void SetAABB(const DirectX::XMFLOAT3& center, const DirectX::XMFLOAT3& extents);
    void SetMaterial(const MaterialID matID);
    void SetTexture(const int idx, const TexID texID);

public:
    char                name_[32]           = "terrain_geomipmapped";
    uint8_t             vertexStride_       = sizeof(Vertex3dTerrain);
    uint32_t            numVertices_        = 0;
    uint32_t            numIndices_         = 0;
    MaterialID          materialID_         = 0;

    VertexBuffer<Vertex3dTerrain> vb_;
    IndexBuffer<UINT>   ib_;
    DirectX::XMFLOAT3   center_;
    DirectX::XMFLOAT3   extents_;

    // keep CPU copy of the vertices/indices data to read from
    Vertex3dTerrain*    vertices_           = nullptr;
    UINT*               indices_            = nullptr;

    int verticesOffset_ = 0;
    int indicesOffset_ = 0;

    GeomPatch*          patches_            = nullptr;  // array of terrain's patches (geometry sets)
    int                 patchSize_          = 17;       // size (width and depth) of a single patch
    int                 numPatchesPerSide_  = 0;        // for instance: 256 + 1 (terrain width) / 16 + 1 (patch size)
    int                 maxLOD_             = 4;        // the number of LOD's for this terrain
    int                 patchesPerFrame_    = 0;

    bool                wantDebug_          = false;
};

} // namespace
