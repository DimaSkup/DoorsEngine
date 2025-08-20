// =================================================================================
// Filename:  TerrainGeomip.h
// Desc:      implementation of terrain geomimapping CLOD algorithm
//            (CLOD - continuous level of detail)
//
// 
//
// NOTE:      we have some limitations for geomipmapping:
//
// Limitation #1:
//            Support only patches of complete triangles fans (look at visualisation):
//            
//              3 -------- (4) -------- 5
//              | \         |         / |
//              |    \      |      /    |
//              |      \    |    /      |
//              |         \ | /         |
//             (2) -------- 0 -------- (6)
//              |         / | \         |
//              |      /    |    \      |
//              |    /      |      \    |
//              | /         |         \ |
//              1 -------- (8) -------- 7
//
// Limitation #2:
//            The size of the patch must be 2^n+1
//
// Limitation #3:
//            We can go no more than one LOD (level of detail) at a time,
//            so from LOD_0 we can switch to only LOD_1 but not to LOD_2
//
//                   *------*
//                   | LOD1 |
//                   |      |
//            *------*------*------*
//            | LOD1 | LOD0 | LOD1 |
//            |      |      |      |
//            *------*------*------*
//                   | LOD1 |
//                   |      |
//                   *------*
// 
// Created:   10.06.2025 by DimaSkup
// =================================================================================
#pragma once

#include "../Mesh/Vertex3dTerrain.h"
#include "../Mesh/VertexBuffer.h"
#include "../Mesh/IndexBuffer.h"
#include "TerrainBase.h"
#include "TerrainLodMgr.h"
#include "../Texture/TextureTypes.h"
#include <CoreCommon/Frustum.h>
#include <cstring>
#include <DirectXCollision.h>


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
class TerrainGeomip : public TerrainBase
{
public:
    TerrainGeomip()  { }
    ~TerrainGeomip() { Shutdown(); }

    void ReleaseBuffers();
    void Shutdown();

    bool InitGeomipmapping(const int patchSize);

    void Update(const CameraParams& camParams);

    bool IsPatchInsideViewFrustum(
        const int x,
        const int z,
        const DirectX::BoundingFrustum& frustum);

    // ------------------------------------------
    // getters
    // ------------------------------------------
    inline const cvector<int>&    GetVisiblePatches()    const { return visiblePatches_; }

    inline int                    GetNumVertices()       const { return numVertices_; }
    inline int                    GetNumIndices()        const { return numIndices_; }
    inline uint                   GetVertexStride()      const { return vb_.GetStride(); }

    inline const Vertex3dTerrain* GetVertices()          const { return vertices_; }

    inline ID3D11Buffer*          GetVertexBuffer()      const { return vb_.Get(); }
    inline ID3D11Buffer*          GetIndexBuffer()       const { return ib_.Get(); }

    inline const TerrainLodMgr&   GetLodMgr()            const { return lodMgr_; }
    inline int                    GetNumPatchesPerSide() const { return lodMgr_.numPatchesPerSide_; }


    // get the number of patches being rendered per frame
    //inline int GetNumPatchesPerFrame(void)    const { return patchesPerFrame_;   }

    // get the current patch number by input coords
    inline int GetPatchNumber(int px, int pz) const { return (pz*lodMgr_.numPatchesPerSide_) + px;   }


    inline void GetLodInfoByPatch(
        const TerrainLodMgr::PatchLod& plod,
        UINT& outBaseIndex,
        UINT& outIndexCount)
    {
        const int c = plod.core;
        const int l = plod.left;
        const int r = plod.right;
        const int t = plod.top;
        const int b = plod.bottom;

        const SingleLodInfo& info = lodInfo_[c].info[l][r][t][b];

        outBaseIndex  = info.start;
        outIndexCount = info.count;
    }

    // ------------------------------------------
    // setters
    // ------------------------------------------
    void SetAABB    (const DirectX::XMFLOAT3& center, const DirectX::XMFLOAT3& extents);
    void SetMaterial(const MaterialID matID);
    void SetTexture (const eTexType type, const TexID texID);

private:
    void InitVertices(Vertex3dTerrain* vertices, const int numVertices);
    int  CalcNumIndices();
    int  InitIndices(cvector<UINT>& indices);
    int  InitIndicesLOD(int idx, cvector<UINT>& indices, const int lod);
    void PopulateBuffers();

    int InitIndicesLODSingle(
        int idx,
        cvector<UINT>& indices,
        const int lodCore,
        const int lodLeft,
        const int lodRight,
        const int lodTop,
        const int lodBottom);

    int CreateTriangleFan(
        int idx,
        cvector<UINT>& indices,
        const int lodCore,
        const int lodLeft,
        const int lodRight,
        const int lodTop,
        const int lodBottom,
        const int x,
        const int z);

    bool InitBuffers(
        const Vertex3dTerrain* vertices,
        const UINT* indices,
        const int numVertices,
        const int numIndices);

    void ComputeTesselation(void);

    void ComputePatch(
        const int currPatchNum,
        const int px,
        const int pz,
        const int numPatchesPerSide);

    void ComputeFan(
        const float cx,
        const float cz,
        const float size,
        const GeomNeighbor& neighbor);

    void CalcNormals(
        Vertex3dTerrain* vertices,
        const UINT* indices,
        const int numVertices,
        const int numIndices);

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

    //int                 verticesOffset_ = 0;
    //int                 indicesOffset_ = 0;

    //GeomPatch*          patches_            = nullptr;  // array of terrain's patches (geometry sets)
    //int                 patchSize_          = 17;       // size (width and depth) of a single patch
    //int                 numPatchesPerSide_  = 0;        // for instance: 256 + 1 (terrain width) / 16 + 1 (patch size)
    //int                 maxLOD_             = 4;        // the number of LOD's for this terrain
    //int                 patchesPerFrame_    = 0;

    //bool                wantDebug_          = false;

private:
    #define LEFT   2
    #define RIGHT  2
    #define TOP    2
    #define BOTTOM 2

    struct SingleLodInfo
    {
        int start = 0;
        int count = 0;
    };

    struct LodInfo
    {
        SingleLodInfo info[LEFT][RIGHT][TOP][BOTTOM];
    };

    cvector<LodInfo> lodInfo_;
    TerrainLodMgr    lodMgr_;

    cvector<int>     visiblePatches_;
};

} // namespaceû
