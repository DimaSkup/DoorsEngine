/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: grass_mgr.cpp

    Created:  26.10.2025  by DimaSkup
\**********************************************************************************/

#include <CoreCommon/pch.h>
#include "grass_mgr.h"
#include <Render/d3dclass.h>
#include <geometry/frustum.h>
#include <camera_params.h>


namespace Core
{

// GLOBAL instance of the grass manager
GrassMgr g_GrassMgr;

//---------------------------------------------------------
// Desc:   do some primary initialization for grass manager
// Args:   - patchSize:           we must know how big is our grass patch (sector)
//         - numPatchesPerSide:   how many patches we have along X and Z axis
//---------------------------------------------------------
void GrassMgr::Init(const int patchSize, const int numPatchesPerSide)
{
    assert(patchSize > 0);
    assert(numPatchesPerSide > 0);

    patchSize_         = patchSize;
    numPatchesPerSide_ = numPatchesPerSide;

    const int numAllPatches = SQR(numPatchesPerSide);

    patches_.resize(numAllPatches);
    visPatchesIdxs_.resize(numAllPatches);
    aabbs_.resize(numAllPatches);
    boundSpheres_.resize(numAllPatches);
}

//---------------------------------------------------------
// Desc:   make frustum test for each grass patch
//         if it is visible we will render it later
//---------------------------------------------------------
void GrassMgr::Update(const CameraParams* pCamParams, Frustum* pWorldFrustum)
{
    vsize numVisPatches = 0;
    const CameraParams& cam = *pCamParams;

    //const float sqrGrassRange = SQR(grassRange_);
    const float grassRange = grassRange_;
    const Vec3 camPos = { cam.posX, cam.posY, cam.posZ };


    for (uint32 idx = 0; idx < (uint32)patches_.size(); ++idx)
    {
        // check if the patch is in the grass visibility range
        Sphere& sph = boundSpheres_[idx];
        const float distToPatchCenter = Vec3Length(camPos - sph.center) - sph.radius;

        if (distToPatchCenter > grassRange)
            continue;

        // check if patch is in view frustum
        if (!pWorldFrustum->TestSphere(boundSpheres_[idx]))
            continue;

        //if (!pWorldFrustum->TestRect(aabbs_[idx]))
        //    continue;

        visPatchesIdxs_[numVisPatches] = idx;
        ++numVisPatches;
    }

    visPatchesIdxs_.resize(numVisPatches);
}

//---------------------------------------------------------
// Desc:   add a bunch of new grass vertices (central point of each grass instance),
//         separate them by its related grass patches (sectors),
//         and update vertex buffers for these patches
//---------------------------------------------------------
void GrassMgr::AddGrassVertices(const cvector<VertexGrass>& newVertices)
{
    if (newVertices.size() == 0)
        return;

    const float invPatchSize    = 1.0f / patchSize_;
    const int numPatchesPerSide = numPatchesPerSide_;

    std::set<int> updatedPatchIdxs;

    for (const VertexGrass& v : newVertices)
    {
        // compute patch index
        const int px  = (int)(v.pos.x * invPatchSize);
        const int pz  = (int)(v.pos.z * invPatchSize);
        const int idx = (pz * numPatchesPerSide) + px;

        patches_[idx].vertices.push_back(v);
        updatedPatchIdxs.insert(idx);
    }

    // use only necessary memory
    for (int i : updatedPatchIdxs)
        patches_[i].vertices.shrink_to_fit();

    for (int i : updatedPatchIdxs)
        UpdateBoundingsForPatch(i);

    for (int i : updatedPatchIdxs)
        RebuildVB(i);
}

//---------------------------------------------------------
// Desc:   add a single grass vertex (central point of the instance)
//         put it into related grass patch (sector) and update vertex buffer
//---------------------------------------------------------
void GrassMgr::AddGrassVertex(const VertexGrass& vertex)
{
    assert(0 && "TODO ?");
}

//---------------------------------------------------------
// Desc:   setup grass visibility distance
//         (when grass patch is farther we don't render it
//          even if it is in the view frustum)
//---------------------------------------------------------
void GrassMgr::SetGrassRange(const float range)
{
    if (range < 0.0f)
    {
        LogErr(LOG, "can't setup a grass visibility distance: input values is < 0");
        return;
    }

    grassRange_ = range;
}

//---------------------------------------------------------
// Desc:   go through each patch and check if we need to rebuild relative
//         vertex buffer (for instance we do it when initialize the scene or
//         when add/remove grass in the editor)
// Args:   - idx:   an index of patch which we will updated
//---------------------------------------------------------
void GrassMgr::RebuildVB(const int idx)
{
    constexpr bool isDynamic = true;

    const size numVertices        = patches_[idx].vertices.size();
    VertexGrass* vertices         = patches_[idx].vertices.data();
    VertexBuffer<VertexGrass>& vb = patches_[idx].vb;

    vb.Shutdown();

    if (!vb.Initialize(Render::g_pDevice, vertices, (int)numVertices, isDynamic))
    {
        LogErr(LOG, "can't rebuild a grass vertex buffer: %d", idx);
        exit(0);
    }
}
//---------------------------------------------------------
// Desc:   return a grass patch (sector) by input index
//---------------------------------------------------------
const GrassPatch& GrassMgr::GetPatchByIdx(const uint32 idx)
{
    assert(idx < patches_.size());
    return patches_[idx];
}

//---------------------------------------------------------
// Desc:   return a vertex buffer by input index of grass patch
//---------------------------------------------------------
VertexBuffer<VertexGrass>& GrassMgr::GetVertexBufByIdx(const uint32 idx)
{
    assert(idx < patches_.size());
    return patches_[idx].vb;
}

//---------------------------------------------------------
// Desc:   return an array of vertices by input index of grass patch
//---------------------------------------------------------
cvector<VertexGrass>& GrassMgr::GetVerticesByIdx(const uint32 idx)
{
    assert(idx < patches_.size());
    return patches_[idx].vertices;
}

//---------------------------------------------------------
// Desc:   recompute bounding shapes (box + sphere) for grass patch by index
//---------------------------------------------------------
void GrassMgr::UpdateBoundingsForPatch(const int i)
{
    assert(i < (int)patches_.size());

    Vec3 minPoint = { +FLT_MAX, +FLT_MAX, +FLT_MAX };
    Vec3 maxPoint = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    // define min and max point for this patch
    for (const VertexGrass& v : patches_[i].vertices)
    {
        minPoint.x = min(v.pos.x, minPoint.x);
        minPoint.y = min(v.pos.y, minPoint.y);
        minPoint.z = min(v.pos.z, minPoint.z);

        maxPoint.x = max(v.pos.x, maxPoint.x);
        maxPoint.y = max(v.pos.y, maxPoint.y);
        maxPoint.z = max(v.pos.z, maxPoint.z);
    }

    // setup bounding box and bounding sphere
    const Vec3 center       = (maxPoint + minPoint) * 0.5f;
    boundSpheres_[i].radius = Vec3Length(maxPoint - center);
    boundSpheres_[i].center = center;

    aabbs_[i] = {
        minPoint.x, maxPoint.x,
        minPoint.y, maxPoint.y,
        minPoint.z, maxPoint.z };
}

} // namespace
