// =================================================================================
// Filename:  Terrain.h
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
#include <math/math_helpers.h>
#include <geometry/frustum.h>
#include <geometry/rect3d.h>
#include <camera_params.h>

#include "../Texture/enum_texture_types.h"
#include "../Mesh/vertex3d_terrain.h"
#include "../Mesh/vertex_buffer.h"
#include "../Mesh/index_buffer.h"
#include "TerrainBase.h"
#include "TerrainLodMgr.h"

#include <string.h>
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
class Terrain : public TerrainBase
{
public:
    Terrain()  { }
    ~Terrain() { Shutdown(); }

    void ReleaseBuffers();
    void Shutdown();

    bool InitGeomipmapping(const int patchSize);

    void Update(
        const CameraParams& camParams,
        const Frustum& worldFrustum,
        const float distFogged);

    bool IsPatchInsideViewFrustum(const int patchIdx, const Frustum& frustum);

    bool TestRayIntersection(
        const Vec3& rayOrig,
        const Vec3& rayDir,
        IntersectionData& outData) const;

    void ComputeBoundings();


    // ------------------------------------------
    // getters
    // ------------------------------------------
    Rect3d                        GetAABB()              const;
    inline const cvector<Rect3d>& GetPatchesAABBs()      const { return patchesAABBs_; }
    inline int                    GetNumMaxLOD()         const { return lodMgr_.maxLOD_; }

    inline const cvector<int>& GetAllVisiblePatches()    const { return visiblePatches_; }
    inline const cvector<int>& GetHighDetailedPatches()  const { return highDetailedPatches_; }
    inline const cvector<int>& GetMidDetailedPatches()   const { return midDetailedPatches_; }
    inline const cvector<int>& GetLowDetailedPatches()   const { return lowDetailedPatches_; }

    inline int                    GetNumVertices()       const { return numVertices_; }
    inline int                    GetNumIndices()        const { return numIndices_; }
    inline uint                   GetVertexStride()      const { return vb_.GetStride(); }

    inline const Vertex3dTerrain* GetVertices()          const { return vertices_.data(); }

    inline ID3D11Buffer*          GetVB()                const { return vb_.Get(); }
    inline ID3D11Buffer* const*   GetVbAddr()            const { return vb_.GetAddrOf(); }
    inline ID3D11Buffer*          GetIB()                const { return ib_.Get(); }

    inline TerrainLodMgr&         GetLodMgr()                  { return lodMgr_; }
    inline int                    GetNumPatchesPerSide() const { return lodMgr_.numPatchesPerSide_; }
    inline int                    GetPatchSize()         const { return lodMgr_.patchSize_; }
    inline int                    GetNumAllPatches()     const { return SQR(GetNumPatchesPerSide()); }


    // get the current patch number by input coords
    inline int GetPatchNumber(int px, int pz)            const { return (pz*lodMgr_.numPatchesPerSide_) + px;   }
    inline int GetDistanceToLOD(const int lod)           const { return lodMgr_.GetDistanceToLOD(lod); }


    inline void GetLodInfoByPatch(
        const TerrainLodMgr::PatchLod& plod,
        UINT& outBaseIndex,
        UINT& outIndexCount) const
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
    void CalcAABB   (void);
    void SetMaterial(const MaterialID matID);
    void SetTexture (const eTexType type, const TexID texID);

    bool SetDistanceToLOD(const int lod, const int dist);

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

    void CalcNormals(
        Vertex3dTerrain* vertices,
        const UINT* indices,
        const int numVertices,
        const int numIndices);

public:
    char                name_[32]           = "terrain_geomipmapped";
    uint32              numVertices_        = 0;
    uint32              numIndices_         = 0;
    MaterialID          materialID_         = 0;

    VertexBuffer<Vertex3dTerrain> vb_;
    IndexBuffer<UINT>   ib_;
    Rect3d              aabb_;              // axis-aligned bounding box for the whole terrain

    // keep CPU copy of the vertices/indices data to read from
    cvector<Vertex3dTerrain> vertices_;
    cvector<UINT>            indices_;

    uint32 vertexStride_ = sizeof(Vertex3dTerrain);

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

    cvector<Rect3d>  patchesAABBs_;
    cvector<Sphere>  patchesBoundSpheres_;

    cvector<LodInfo> lodInfo_;
    TerrainLodMgr    lodMgr_;

    cvector<int>     visiblePatches_;
    cvector<int>     highDetailedPatches_;
    cvector<int>     midDetailedPatches_;
    cvector<int>     lowDetailedPatches_;
};

//---------------------------------------------------------
// inline functions
//---------------------------------------------------------

//---------------------------------------------------------
// Desc:  return an axis-aligned bounding box for the whole terrain
//---------------------------------------------------------
inline Rect3d Terrain::GetAABB() const
{
    return aabb_;
}

//-----------------------------------------------------
// Desc:   set a distance from the camera where LOD starts
// Args:   - lod:   the number of LOD to change
//         - dist:  new distance to lod
// Ret:    true if everything is ok
//-----------------------------------------------------
inline bool Terrain::SetDistanceToLOD(const int lod, const int dist)
{
    return lodMgr_.SetDistanceToLOD(lod, dist);
}

} // namespace
