/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: debug_draw_manager.h
    Desc:     the debug drawing API

    Created:  06.10.2025  by DimaSkup
\**********************************************************************************/
#pragma once

#include <math/vec3.h>
#include <math/matrix.h>
#include <cvector.h>
#include <types.h>


namespace Core
{
//---------------------------------------------------------
// constants
//---------------------------------------------------------
constexpr int DEBUG_LINE_BUF_LIMIT = 512;
constexpr int DEBUG_LINE_VERTEX_BUF_LIMIT = 512;

constexpr int NUM_AABB_VERTICES = 8;
constexpr int NUM_AABB_INDICES = 32;
constexpr int NUM_CIRCLE_VERTICES = 36;
constexpr int NUM_SPHERE_VERTICES = NUM_CIRCLE_VERTICES * 3;
constexpr int NUM_SPHERE_INDICES  = NUM_CIRCLE_VERTICES * 6;

//---------------------------------------------------------
// debug geometry shapes definitions
//---------------------------------------------------------
struct DebugLine
{
    Vec3   fromPos       = {0,0,0};
    Vec3   toPos         = {0,0,1};
    uint32 color = 0;
    uint8  lineWidth = 1;               // width in pixels
    uint16 durationMs = 0;              // 0 is infinite lifetime
    uint8  depthEnabled = true;
};

struct DebugLineVertex
{
    Vec3   pos           = {0,0,1};
    uint32 color         = 0;
    uint8  lineWidth     = 1;           // width in pixels
    uint16 durationMs    = 0;           // 0 is inifinite lifetime
    uint8  depthEnabled = true;
};

//---------------------------------------------------------
// class: DebugDrawMgr
//---------------------------------------------------------
class DebugDrawMgr
{
public:
    DebugDrawMgr();

    // add a line segment to the debug drawing queue
    void AddLine(const Vec3& fromPos,
                 const Vec3& toPos,
                 const Vec3& color,
                 const int lineWidth = 1,
                 const int duration = 0,
                 const bool depthEnabled = true);

    // add an axis-aligned cross (3 lines converging at a point)
    // to the debug drawing queue
    void AddCross(const Vec3& pos,
                  const Vec3& color,
                  const float size,
                  const float duration = 0.0f,
                  const bool depthEnabled = true);

    // add a wireframe sphere to the debug drawing queue
    void AddSphere(const Vec3& centerPos,
                   const float radius,
                   const Vec3& color,
                   const int durationMs = 0,
                   const bool depthEnabled = true);

    // add a circle to the debug drawing queue
    void AddCircle(const Vec3& centerPos,
                   const Vec3& planeNormal,
                   const float radius,
                   const Vec3& color,
                   const float duration = 0.0f,
                   const bool depthEnalbed = true);

    // add a set of coordinate axes depicting the position and orientation
    // of the given transformation to the debug draing queue
    void AddAxes(const Matrix& m,
                 const Vec3& color,
                 const float size,
                 const float duration = 0.0f,
                 const bool depthEnabled = true);

    // add a wireframe triangle to the debug drawing queue
    void AddTriangle(const Vec3& vertex0,
                     const Vec3& vertex1,
                     const Vec3& vertex2,
                     const Vec3& color,
                     const float lineWidth = 1.0f,
                     const float duration = 0.0f,
                     const bool depthEnabled = true);

    // add an axis-aligned bounding box to the debug drawing queue
    void AddAABB(const Vec3& minCoords,
                 const Vec3& maxCoords,
                 const Vec3& color,
                 const int lineWidth = 1,
                 const int durationMs = 0,
                 const bool depthEnabled = true);

    // add an axis-aligned bounding box terrain's patch (sector, chunk)
    // to the debug drawing queue
    void AddTerrainAABB(const Vec3& minCoords,
                        const Vec3& maxCoords,
                        const Vec3& color,
                        const int lineWidth = 1,
                        const int durationMs = 0,
                        const bool depthEnabled = true);

    // add an oriented bounding box to the debug queue
    void AddOBB(const Matrix& centerTransform,
                const Vec3& scaleXYZ,
                const Vec3& color,
                const float lineWidth = 1.0f,
                const float duration = 0.0f,
                const bool depthEnabled = true);

    // add a text string to the debug drawing queue
    void AddString(const Vec3& topLeftPos,
                   const char* text,
                   const Vec3& color,
                   const float duration = 0.0f,
                   const bool depthEnabled = true);

    // add a view frustum to the debug drawing queue
    void AddFrustum(const Vec3& nearTopLeft,
                    const Vec3& nearBottomLeft,
                    const Vec3& nearTopRight,
                    const Vec3& nearBottomRight,
                    const Vec3& farTopLeft,
                    const Vec3& farBottomLeft,
                    const Vec3& farTopRight,
                    const Vec3& farBottomRight,
                    const Vec3& color);

    // after we add some debug shape into mgr we need to
    // update relative vertex and index buffer
    //bool NeedUpdateBuffers()   const;

    constexpr inline int GetNumVerticesInAABB()   const { return NUM_AABB_VERTICES; }
    constexpr inline int GetNumIndicesInAABB()    const { return NUM_AABB_INDICES; }

    constexpr inline int GetNumVerticesInSphere() const { return NUM_SPHERE_VERTICES; }
    constexpr inline int GetNumIndicesInSphere()  const { return NUM_SPHERE_INDICES; }

public:
    // vertices arrays by debug shape types
    cvector<DebugLine> lines_;

    DebugLine       frustumLines_[12];
    cvector<DebugLineVertex> aabbsVertices_;
    cvector<DebugLineVertex> sphereVertices_;
    cvector<DebugLineVertex> terrainAABBsVertices_;

    cvector<uint16> aabbIndices_;
    cvector<uint16> sphereIndices_;

    static int      currNumLines_;
    static int      currNumAABBs_;
    static int      currNumTerrainAABBs_;
    static int      currNumSpheres_;

    // defines if we want to render all the debug shapes or only
    // some kind of it (only lines, or only AABBs)
    uint16 doRendering_          : 1 = false;
    uint16 renderDbgLines_       : 1 = true;
    uint16 renderDbgCross_       : 1 = true;
    uint16 renderDbgSphere_      : 1 = true;
    uint16 renderDbgCircle_      : 1 = true;
    uint16 renderDbgAxes_        : 1 = true;
    uint16 renderDbgTriangle_    : 1 = true;
    uint16 renderDbgAABB_        : 1 = true;
    uint16 renderDbgOBB_         : 1 = true;
    uint16 renderDbgString_      : 1 = true;
    uint16 renderDbgFrustum_     : 1 = true;
    uint16 renderDbgTerrainAABB_ : 1 = true;
};

//---------------------------------------------------------
// this global debug drawing manager is cofigured for drawing 
// in full 3D with a perspective projection
//---------------------------------------------------------
extern DebugDrawMgr g_DebugDrawMgr;

//---------------------------------------------------------
// this global debug drawing manager draws its primitives in
// 2d screen space. The (x,y) coordinates of the point specify
// a 2D location on-screen, and the z coordinate contains a special
// code that indicates whether the (x,y) coordinates are measured
// in absolute pixels or in normalized coordinates that range from 0.0 to 1.0.
// (The latter mode allows drawing to be independent
//  of the actual resolution of the screen)
//---------------------------------------------------------
extern DebugDrawMgr g_DebugDrawMgr2D;

}
