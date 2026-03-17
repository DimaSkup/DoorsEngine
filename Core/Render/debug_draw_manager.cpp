/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: debug_draw_manager.cpp
    Desc:     implementation of the debug drawing API

    Created:  06.10.2025  by DimaSkup
\**********************************************************************************/
#include <CoreCommon/pch.h>
#include "debug_draw_manager.h"
#include <math/matrix.h>


namespace Core
{

//---------------------------------------------------------
// global instances of the manager
//---------------------------------------------------------
DbgDrawMgr g_DebugDrawMgr;
DbgDrawMgr g_DebugDrawMgr2D;

//---------------------------------------------------------
// convert float3 RGB color into uint32
//---------------------------------------------------------
inline uint32 GetU32Color(const Vec3& color)
{
    const uint8 r = (uint8)(color.r * 255);
    const uint8 g = (uint8)(color.g * 255);
    const uint8 b = (uint8)(color.b * 255);
    const uint32 c = (r << 24) | (g << 16) | (b << 8);
    return c;
}

//---------------------------------------------------------
// default constructor
//---------------------------------------------------------
DbgDrawMgr::DbgDrawMgr() : pData_(nullptr)
{
}

//---------------------------------------------------------
// do it when we turn on the debug rendering
//---------------------------------------------------------
void DbgDrawMgr::Init()
{
    pData_ = NEW DbgDrawData();
    assert(pData_);

    // reserve ahead some memory for vertices
    pData_->aabbsVerts_.reserve(256);
    pData_->sphereVerts_.reserve(1024);
    pData_->terrainAABBsVerts_.reserve(1024 * 8);

    // precompute stuff for some debug shapes
    PrecomputeAABB();
    PrecomputeSphere();
}

//---------------------------------------------------------
// do it when we turn off the debug rendering
//---------------------------------------------------------
void DbgDrawMgr::Shutdown()
{
    SafeDelete(pData_);
}

//---------------------------------------------------------
// reset arrays from the previous frame's data
//---------------------------------------------------------
void DbgDrawMgr::Reset()
{
    assert(pData_);

    memset(pData_->frustumLines_, 0, sizeof(pData_->frustumLines_));
    pData_->aabbsVerts_.clear();
    pData_->sphereVerts_.clear();
    pData_->terrainAABBsVerts_.clear();

    pData_->currNumAABBs_        = 0;
    pData_->currNumTerrainAABBs_ = 0;
    pData_->currNumSpheres_      = 0;
}

//---------------------------------------------------------
// do we want to render a debug geometry of specific type?
//---------------------------------------------------------
bool DbgDrawMgr::IsRenderableType(const eDbgGeomType type) const
{
    assert(pData_);
    using enum eDbgGeomType;

    switch (type)
    {
        case LINE:              return pData_->bRenderLines_;
        case CROSS:             return pData_->bRenderCross_;
        case SPHERE:            return pData_->bRenderSphere_;
        case CIRCLE:            return pData_->bRenderCircle_;
        case AXIS:              return pData_->bRenderAxis_;
        case TRIANGLE:          return pData_->bRenderTriangle_;
        case AABB:              return pData_->bRenderAABB_;
        case TERRAIN_AABB:      return pData_->bRenderTerrainAABB_;
        case OBB:               return pData_->bRenderOBB_;
        case STRING:            return pData_->bRenderString_;
        case FRUSTUM:           return pData_->bRenderFrustum_;
        case MODEL_WIREFRAME:   return pData_->bRenderWireframe_;

        default:
            LogErr(LOG, "you fucked up, there is no type: %d", (int)type);
    }

    return false;
}

//---------------------------------------------------------
// turn on/off rendering of debug geometry of specific type
//---------------------------------------------------------
void DbgDrawMgr::SwitchRenderingType(const eDbgGeomType type, const bool onOff)
{
    assert(pData_);
    using enum eDbgGeomType;

    switch (type)
    {
        case LINE:              pData_->bRenderLines_       = onOff; break;
        case CROSS:             pData_->bRenderCross_       = onOff; break;
        case SPHERE:            pData_->bRenderSphere_      = onOff; break;
        case CIRCLE:            pData_->bRenderCircle_      = onOff; break;
        case AXIS:              pData_->bRenderAxis_        = onOff; break;
        case TRIANGLE:          pData_->bRenderTriangle_    = onOff; break;
        case AABB:              pData_->bRenderAABB_        = onOff; break;
        case TERRAIN_AABB:      pData_->bRenderTerrainAABB_ = onOff; break;
        case OBB:               pData_->bRenderOBB_         = onOff; break;
        case STRING:            pData_->bRenderString_      = onOff; break;
        case FRUSTUM:           pData_->bRenderFrustum_     = onOff; break;
        case MODEL_WIREFRAME:   pData_->bRenderWireframe_   = onOff; break;

        default:
            LogErr(LOG, "you fucked up, there is no type: %d", (int)type);
    }
}

//---------------------------------------------------------
// Desc:   add a line segment to the debug drawing queue
//---------------------------------------------------------
void DbgDrawMgr::AddLine(
    const Vec3& fromPos,
    const Vec3& toPos,
    const Vec3& colorRGB,
    const int lineWidth,
    const int durationMs,
    const bool bDepth)
{
    static int lineIdx = 0;

    assert(pData_);
    pData_->lines_[lineIdx] = DbgLine{
        fromPos,
        toPos,
        GetU32Color(colorRGB),
        (uint16)durationMs,
        (uint8)lineWidth,
        (uint8)bDepth };

    lineIdx++;
    lineIdx &= (MAX_NUM_DBG_LINES - 1);      // idx % MAX_NUM_DBG_LINES

#if 0
    printf("\nline is added:\n");
    printf("start: %f %f %f\n", line.fromPos.x, line.fromPos.y, line.fromPos.z);
    printf("to:    %f %f %f\n", line.toPos.x, line.toPos.y, line.toPos.z);
    printf("color: %f %f %f\n", line.color.r, line.color.g, line.color.b);
    printf("width: %f\n", line.lineWidth);
    printf("dur:   %f\n", line.duration);
    printf("depth: %d\n", (int)line.depthEnabled);
#endif
}

//---------------------------------------------------------
// Desc:   add a wireframe sphere to the debug drawing queue
//---------------------------------------------------------
void DbgDrawMgr::AddSphere(
    const Vec3& centerPos,
    const float radius,
    const Vec3& colorRGB,
    const int durationMs,
    const bool bDepth)       // enable/disable depth for this sphere
{
    const uint8  lineWidth = 1;
    const uint16 dur       = (uint16)durationMs;
    const uint32 color     = GetU32Color(colorRGB);

    assert(pData_);
    pData_->currNumSpheres_++;

    // radius is just a bit bigger to prevent z-fighting btw lines and actual geometry
    const float r = radius + 0.005f;

    cvector<DbgLineVertex>& verts = pData_->sphereVerts_;
    float* cosines = pData_->cosines_;
    float* sines   = pData_->sines_;

    // compute a circle in x-z plane
    for (int i = 0; i < NUM_CIRCLE_VERTICES; ++i)
    {
        const Vec3 pos(
            centerPos.x + (r * cosines[i]),
            centerPos.y,
            centerPos.z + (r * sines[i]));

        verts.push_back(DbgLineVertex{ pos, color, dur, lineWidth, bDepth });
    }

    // compute a circle in x-y plane
    for (int i = 0; i < NUM_CIRCLE_VERTICES; ++i)
    {
        const Vec3 pos(
            centerPos.x + (r * cosines[i]),
            centerPos.y + (r * sines[i]),
            centerPos.z);

        verts.push_back(DbgLineVertex{ pos, color, dur, lineWidth, bDepth });
    }

    // compute a circle in y-z plane
    for (int i = 0; i < NUM_CIRCLE_VERTICES; ++i)
    {
        const Vec3 pos(
            centerPos.x,
            centerPos.y + (r * sines[i]),
            centerPos.z + (r * cosines[i]));

        verts.push_back(DbgLineVertex{ pos, color, dur, lineWidth, bDepth });
    }
}

//---------------------------------------------------------
// Desc:   add an axis-aligned bounding box to the debug queue
//---------------------------------------------------------
void DbgDrawMgr::AddAABB(
    const Vec3& minP,
    const Vec3& maxP,
    const Vec3& colorRGB,
    const int lineWidth,
    const int durationMs,
    const bool depthEnabled)
{
    assert(pData_);
    ++pData_->currNumAABBs_;

    // define points
    const Vec3 p0(minP.x, minP.y, minP.z);
    const Vec3 p1(minP.x, minP.y, maxP.z);
    const Vec3 p2(maxP.x, minP.y, maxP.z);
    const Vec3 p3(maxP.x, minP.y, minP.z);

    const Vec3 p4(minP.x, maxP.y, minP.z);
    const Vec3 p5(minP.x, maxP.y, maxP.z);
    const Vec3 p6(maxP.x, maxP.y, maxP.z);
    const Vec3 p7(maxP.x, maxP.y, minP.z);

    const uint32 color  = GetU32Color(colorRGB);
    const uint16 dur    = (uint16)durationMs;
    const uint8  w      = (uint8)lineWidth;
    const uint8  bDepth = (uint8)depthEnabled;

    // setup AABB vertices
    cvector<DbgLineVertex>& verts = pData_->aabbsVerts_;

    verts.push_back(DbgLineVertex{ p0, color, dur, w, bDepth });
    verts.push_back(DbgLineVertex{ p1, color, dur, w, bDepth });
    verts.push_back(DbgLineVertex{ p2, color, dur, w, bDepth });
    verts.push_back(DbgLineVertex{ p3, color, dur, w, bDepth });

    verts.push_back(DbgLineVertex{ p4, color, dur, w, bDepth });
    verts.push_back(DbgLineVertex{ p5, color, dur, w, bDepth });
    verts.push_back(DbgLineVertex{ p6, color, dur, w, bDepth });
    verts.push_back(DbgLineVertex{ p7, color, dur, w, bDepth });
}

//---------------------------------------------------------
// Desc:   add an axis-aligned bounding box terrain's patch (sector, chunk)
//        to the debug drawing queue
//---------------------------------------------------------
void DbgDrawMgr::AddTerrainAABB(
    const Vec3& minP,
    const Vec3& maxP,
    const Vec3& colorRGB,
    const int lineWidth,
    const int durationMs,
    const bool depthEnabled)
{
    assert(pData_);
    ++pData_->currNumTerrainAABBs_;

    // define points
    const Vec3 p0(minP.x, minP.y, minP.z);
    const Vec3 p1(minP.x, minP.y, maxP.z);
    const Vec3 p2(maxP.x, minP.y, maxP.z);
    const Vec3 p3(maxP.x, minP.y, minP.z);

    const Vec3 p4(minP.x, maxP.y, minP.z);
    const Vec3 p5(minP.x, maxP.y, maxP.z);
    const Vec3 p6(maxP.x, maxP.y, maxP.z);
    const Vec3 p7(maxP.x, maxP.y, minP.z);

    const uint32 color  = GetU32Color(colorRGB);
    const uint16 dur    = (uint16)durationMs;
    const uint8  w = (uint8)lineWidth;
    const uint8  bDepth = (uint8)depthEnabled;

    // setup AABB vertices
    cvector<DbgLineVertex>& verts = pData_->terrainAABBsVerts_;

    verts.push_back(DbgLineVertex{ p0, color, dur, w, bDepth });
    verts.push_back(DbgLineVertex{ p1, color, dur, w, bDepth });
    verts.push_back(DbgLineVertex{ p2, color, dur, w, bDepth });
    verts.push_back(DbgLineVertex{ p3, color, dur, w, bDepth });

    verts.push_back(DbgLineVertex{ p4, color, dur, w, bDepth });
    verts.push_back(DbgLineVertex{ p5, color, dur, w, bDepth });
    verts.push_back(DbgLineVertex{ p6, color, dur, w, bDepth });
    verts.push_back(DbgLineVertex{ p7, color, dur, w, bDepth });
}

//---------------------------------------------------------
// Desc:   add a view frustum to the debug drawing queue
//---------------------------------------------------------
void DbgDrawMgr::AddFrustum(
    const Vec3& nearTopLeft,
    const Vec3& nearBottomLeft,
    const Vec3& nearTopRight,
    const Vec3& nearBottomRight,
    const Vec3& farTopLeft,
    const Vec3& farBottomLeft,
    const Vec3& farTopRight,
    const Vec3& farBottomRight,
    const Vec3& colorRGB)
{
    assert(pData_);

    const uint32 color = GetU32Color(colorRGB);
    DbgLine*     lines = pData_->frustumLines_;

    // near plane
    lines[0]  = DbgLine{ nearTopLeft,     nearBottomLeft,  color };
    lines[1]  = DbgLine{ nearBottomLeft,  nearBottomRight, color };
    lines[2]  = DbgLine{ nearBottomRight, nearTopRight,    color };
    lines[3]  = DbgLine{ nearTopRight,    nearTopLeft,     color };

    // far plane
    lines[4]  = DbgLine{ farTopLeft,      farBottomLeft,   color };
    lines[5]  = DbgLine{ farBottomLeft,   farBottomRight,  color };
    lines[6]  = DbgLine{ farBottomRight,  farTopRight,     color };
    lines[7]  = DbgLine{ farTopRight,     farTopLeft,      color };

    // lines from near to far plane
    lines[8]  = DbgLine{ nearTopLeft,     farTopLeft,      color };
    lines[9]  = DbgLine{ nearBottomLeft,  farBottomLeft,   color };
    lines[10] = DbgLine{ nearTopRight,    farTopRight,     color };
    lines[11] = DbgLine{ nearBottomRight, farBottomRight,  color };
}

//---------------------------------------------------------
// precompute indices for lines of axis-aligned bounding boxes (AABB)
//---------------------------------------------------------
void DbgDrawMgr::PrecomputeAABB(void)
{
    assert(pData_);

    const uint16 aabbIndices[NUM_AABB_INDICES] =
    {
        0,1, 1,2, 2,3, 0,3,   // bottom lines
        4,5, 5,6, 6,7, 4,7,   // upper lines
        0,4, 1,5, 2,6, 3,7,   // vertical lines
    };

    pData_->aabbIndices_.resize(NUM_AABB_INDICES);
    memcpy(pData_->aabbIndices_.data(), aabbIndices, sizeof(uint16) * NUM_AABB_INDICES);
}

//---------------------------------------------------------
// precompute sines, cosines, and indices for lines of debug spheres
//---------------------------------------------------------
void DbgDrawMgr::PrecomputeSphere(void)
{
    assert(pData_);

    int i = 0;
    int index = 0;
    int vertexIdx = 0;
    const int angleStep = 360 / NUM_CIRCLE_VERTICES;

    //
    // precompute sines and cosines for a sphere
    //
    for (int i = 0; i < NUM_CIRCLE_VERTICES; ++i)
    {
        const float angle = DEG_TO_RAD(i * angleStep);
        pData_->sines_[i]   = sinf(angle);
        pData_->cosines_[i] = cosf(angle);
    }

    //
    // precompute indices for sphere
    //
    cvector<uint16>& idxs = pData_->sphereIndices_;
    idxs.resize(NUM_SPHERE_INDICES);

    // indices for sphere's circle in x-z plane
    for (; vertexIdx < NUM_CIRCLE_VERTICES; ++vertexIdx, ++index)
    {
        idxs[i++] = index;
        idxs[i++] = index + 1;
    }
    idxs[i - 1] = 0;

    // indices for sphere's circle in x-y plane
    for (; vertexIdx < NUM_CIRCLE_VERTICES * 2; ++vertexIdx, ++index)
    {
        idxs[i++] = index;
        idxs[i++] = index + 1;
    }
    idxs[i - 1] = NUM_CIRCLE_VERTICES;

    // indices for sphere's circle in y-z plane
    for (; vertexIdx < NUM_CIRCLE_VERTICES * 3; ++vertexIdx, ++index)
    {
        idxs[i++] = index;
        idxs[i++] = index + 1;
    }
    idxs[i - 1] = NUM_CIRCLE_VERTICES * 2;
}

} // namespace
