/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: grass_mgr.h
    Desc:     grass patches (sectors) manager
              (I hope it will optimize my grass rendering :)

    Created:  26.10.2025  by DimaSkup
\**********************************************************************************/
#pragma once
#include <cvector.h>
#include <Mesh/vertex.h>
#include <Mesh/vertex_buffer.h>
#include <geometry/rect_3d.h>
#include <geometry/sphere.h>

// forward declarations
class Frustum;
struct CameraParams;


namespace Core
{

// each grass sector (patch) has its own vertex buffer,
// so we are able to render such sectors separately
struct GrassPatch
{
    cvector<VertexGrass> vertices;
    VertexBuffer<VertexGrass> vb;
};

//---------------------------------------------------------

class GrassMgr
{
public:
    GrassMgr() {}

    void Init(const int patchSize, const int numPatchesPerSide);
    void Update(const CameraParams* pCamParams, Frustum* pFrustum);

    void AddGrassVertices(const cvector<VertexGrass>& newVertices);
    void AddGrassVertex(const VertexGrass& vertex);

    void SetGrassRange(const float range);

    inline size                   GetNumPatches()     const { return patches_.size(); }
    inline const cvector<uint32>& GetVisPatchesIdxs() const { return visPatchesIdxs_; }

    const GrassPatch&          GetPatchByIdx    (const uint32 idx);
    VertexBuffer<VertexGrass>& GetVertexBufByIdx(const uint32 idx);
    cvector<VertexGrass>&      GetVerticesByIdx (const uint32 idx);

private:
    void UpdateBoundingsForPatch(const int i);
    void RebuildVB              (const int i);


private:
    cvector<uint32>     visPatchesIdxs_;
    cvector<GrassPatch> patches_;

    // bounding shapes: one per each grass patch (sector)
    cvector<Rect3d>     aabbs_;
    cvector<Sphere>     boundSpheres_;

    int patchSize_         = 0;
    int numPatchesPerSide_ = 0;

    float grassRange_      = 0;
};

//---------------------------------------------------------
// GLOBAL instance of the grass manager
//---------------------------------------------------------
extern GrassMgr g_GrassMgr;

} // namespace
