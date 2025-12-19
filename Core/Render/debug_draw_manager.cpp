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

namespace Core
{
//---------------------------------------------------------
// global instances of the manager
//---------------------------------------------------------
DebugDrawMgr g_DebugDrawMgr;
DebugDrawMgr g_DebugDrawMgr2D;

//---------------------------------------------------------
// static data fields
//---------------------------------------------------------
int DebugDrawMgr::currNumLines_         = 0;
int DebugDrawMgr::currNumAABBs_         = 0;
int DebugDrawMgr::currNumTerrainAABBs_  = 0;
int DebugDrawMgr::currNumSpheres_       = 0;

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
DebugDrawMgr::DebugDrawMgr()
{
    //printf("dbg line sizeof:        %d\n", (int)sizeof(DebugLine));
    //printf("dbg line vertex sizeof: %d\n", (int)sizeof(DebugLineVertex));
    //exit(0);

    // reserve ahead some memory for vertices
    aabbsVertices_.reserve(256);
    sphereVertices_.reserve(1024);
    terrainAABBsVertices_.reserve(1024 * 8);


    // precompute indices for AABB
    const uint16 aabbIndices[NUM_AABB_INDICES] =
    {
        0,1, 1,2, 2,3, 0,3,   // bottom lines
        4,5, 5,6, 6,7, 4,7,   // upper lines
        0,4, 1,5, 2,6, 3,7,   // vertical lines
    };

    aabbIndices_.resize(NUM_AABB_INDICES);
    memcpy(aabbIndices_.data(), aabbIndices, sizeof(uint16) * 32);


    // precompute indices for sphere
    int i = 0;
    int index = 0;
    int vertexIdx = 0;
    sphereIndices_.resize(NUM_SPHERE_INDICES);

    // indices for sphere's circle in x-z plane
    for (; vertexIdx < NUM_CIRCLE_VERTICES; ++vertexIdx, ++index)
    {
        sphereIndices_[i++] = index;
        sphereIndices_[i++] = index + 1;
    }
    sphereIndices_[i - 1] = 0;

    // indices for sphere's circle in x-y plane
    for (; vertexIdx < NUM_CIRCLE_VERTICES*2; ++vertexIdx, ++index)
    {
        sphereIndices_[i++] = index;
        sphereIndices_[i++] = index + 1;
    }
    sphereIndices_[i - 1] = NUM_CIRCLE_VERTICES;

    // indices for sphere's circle in y-z plane
    for (; vertexIdx < NUM_CIRCLE_VERTICES*3; ++vertexIdx, ++index)
    {
        sphereIndices_[i++] = index;
        sphereIndices_[i++] = index + 1;
    }
    sphereIndices_[i - 1] = NUM_CIRCLE_VERTICES*2;
}

//---------------------------------------------------------
// Desc:   add a line segment to the debug drawing queue
//---------------------------------------------------------
void DebugDrawMgr::AddLine(
    const Vec3& fromPos,
    const Vec3& toPos,
    const Vec3& colorRGB,
    const int lineWidth,
    const int durationMs,
    const bool depthEnabled)
{
    ++currNumLines_;
    lines_.push_back(DebugLine{
        fromPos,
        toPos,
        GetU32Color(colorRGB),
        (uint8)lineWidth,
        (uint16)durationMs,
        (uint8)depthEnabled });

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
void DebugDrawMgr::AddSphere(
    const Vec3& centerPos,
    const float radius,
    const Vec3& colorRGB,
    const int durationMs,
    const bool depthEnabled)
{
    const uint8  lineWidth   = 1;
    const uint8  depthEnable = (uint8)depthEnabled;
    const uint16 duration    = (uint16)durationMs;
    const int    angleStep   = 360 / NUM_CIRCLE_VERTICES;
    const uint32 color       = GetU32Color(colorRGB);

    float sines  [NUM_CIRCLE_VERTICES];
    float cosines[NUM_CIRCLE_VERTICES];


    // precompute sines and cosines
    const float r = radius + 0.005f;   // just a bit bigger to prevent z-fighting btw lines and actual geometry

    for (int i = 0; i < NUM_CIRCLE_VERTICES; ++i)
    {
        const float angle = DEG_TO_RAD(i * angleStep);
        sines[i]   = r * sinf(angle);
        cosines[i] = r * cosf(angle);
    }

    // compute a circle in x-z plane
    for (int i = 0; i < NUM_CIRCLE_VERTICES; ++i)
    {
        const Vec3 pos(
            centerPos.x + cosines[i],
            centerPos.y,
            centerPos.z + sines[i]);

        sphereVertices_.push_back(DebugLineVertex{
            pos,
            color,
            lineWidth,
            duration,
            depthEnable });
    }

    // compute a circle in x-y plane
    for (int i = 0; i < NUM_CIRCLE_VERTICES; ++i)
    {
        const Vec3 pos(
            centerPos.x + cosines[i],
            centerPos.y + sines[i],
            centerPos.z);

        sphereVertices_.push_back(DebugLineVertex{
            pos,
            color,
            lineWidth,
            duration,
            depthEnabled });
    }

    // compute a circle in y-z plane
    for (int i = 0; i < NUM_CIRCLE_VERTICES; ++i)
    {
        const Vec3 pos(
            centerPos.x,
            centerPos.y + sines[i],
            centerPos.z + cosines[i]);

        sphereVertices_.push_back(DebugLineVertex{
            pos,
            color,
            lineWidth,
            duration,
            depthEnabled });
    }

    currNumSpheres_++;
}

//---------------------------------------------------------
// Desc:   add an axis-aligned bounding box to the debug queue
//---------------------------------------------------------
void DebugDrawMgr::AddAABB(
    const Vec3& minCoords,
    const Vec3& maxCoords,
    const Vec3& colorRGB,
    const int lineWidth,
    const int durationMs,
    const bool depthEnabled)
{
    ++currNumAABBs_;

    // define points
    const Vec3 p0(minCoords.x, minCoords.y, minCoords.z);
    const Vec3 p1(minCoords.x, minCoords.y, maxCoords.z);
    const Vec3 p2(maxCoords.x, minCoords.y, maxCoords.z);
    const Vec3 p3(maxCoords.x, minCoords.y, minCoords.z);

    const Vec3 p4(minCoords.x, maxCoords.y, minCoords.z);
    const Vec3 p5(minCoords.x, maxCoords.y, maxCoords.z);
    const Vec3 p6(maxCoords.x, maxCoords.y, maxCoords.z);
    const Vec3 p7(maxCoords.x, maxCoords.y, minCoords.z);

    const uint8 width     = (uint8)lineWidth;
    const uint16 duration = (uint16)durationMs;
    const uint8 hasDepth  = (uint8)depthEnabled;
    const uint32 color    = GetU32Color(colorRGB);

    // setup AABB vertices
    aabbsVertices_.push_back(DebugLineVertex{ p0, color, width, duration, hasDepth });
    aabbsVertices_.push_back(DebugLineVertex{ p1, color, width, duration, hasDepth });
    aabbsVertices_.push_back(DebugLineVertex{ p2, color, width, duration, hasDepth });
    aabbsVertices_.push_back(DebugLineVertex{ p3, color, width, duration, hasDepth });

    aabbsVertices_.push_back(DebugLineVertex{ p4, color, width, duration, hasDepth });
    aabbsVertices_.push_back(DebugLineVertex{ p5, color, width, duration, hasDepth });
    aabbsVertices_.push_back(DebugLineVertex{ p6, color, width, duration, hasDepth });
    aabbsVertices_.push_back(DebugLineVertex{ p7, color, width, duration, hasDepth });
}

//---------------------------------------------------------
// Desc:   add an axis-aligned bounding box terrain's patch (sector, chunk)
//        to the debug drawing queue
//---------------------------------------------------------
void DebugDrawMgr::AddTerrainAABB(const Vec3& minCoords,
    const Vec3& maxCoords,
    const Vec3& colorRGB,
    const int lineWidth,
    const int durationMs,
    const bool depthEnabled)
{
    ++currNumTerrainAABBs_;

    // define points
    const Vec3 p0(minCoords.x, minCoords.y, minCoords.z);
    const Vec3 p1(minCoords.x, minCoords.y, maxCoords.z);
    const Vec3 p2(maxCoords.x, minCoords.y, maxCoords.z);
    const Vec3 p3(maxCoords.x, minCoords.y, minCoords.z);

    const Vec3 p4(minCoords.x, maxCoords.y, minCoords.z);
    const Vec3 p5(minCoords.x, maxCoords.y, maxCoords.z);
    const Vec3 p6(maxCoords.x, maxCoords.y, maxCoords.z);
    const Vec3 p7(maxCoords.x, maxCoords.y, minCoords.z);

    const uint8 width     = (uint8)lineWidth;
    const uint16 duration = (uint16)durationMs;
    const uint8 hasDepth  = (uint8)depthEnabled;
    const uint32 color    = GetU32Color(colorRGB);

    // setup AABB vertices
    terrainAABBsVertices_.push_back(DebugLineVertex{ p0, color, width, duration, hasDepth});
    terrainAABBsVertices_.push_back(DebugLineVertex{ p1, color, width, duration, hasDepth });
    terrainAABBsVertices_.push_back(DebugLineVertex{ p2, color, width, duration, hasDepth });
    terrainAABBsVertices_.push_back(DebugLineVertex{ p3, color, width, duration, hasDepth });

    terrainAABBsVertices_.push_back(DebugLineVertex{ p4, color, width, duration, hasDepth });
    terrainAABBsVertices_.push_back(DebugLineVertex{ p5, color, width, duration, hasDepth });
    terrainAABBsVertices_.push_back(DebugLineVertex{ p6, color, width, duration, hasDepth });
    terrainAABBsVertices_.push_back(DebugLineVertex{ p7, color, width, duration, hasDepth });
}

//---------------------------------------------------------
// Desc:   add a view frustum to the debug drawing queue
//---------------------------------------------------------
void DebugDrawMgr::AddFrustum(
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
    int idx = 0;
    const uint32 color = GetU32Color(colorRGB);

    // near plane
    frustumLines_[idx++] = DebugLine{ nearTopLeft,     nearBottomLeft,  color };
    frustumLines_[idx++] = DebugLine{ nearBottomLeft,  nearBottomRight, color };
    frustumLines_[idx++] = DebugLine{ nearBottomRight, nearTopRight,    color };
    frustumLines_[idx++] = DebugLine{ nearTopRight,    nearTopLeft,     color };

    // far plane
    frustumLines_[idx++] = DebugLine{ farTopLeft,      farBottomLeft,   color };
    frustumLines_[idx++] = DebugLine{ farBottomLeft,   farBottomRight,  color };
    frustumLines_[idx++] = DebugLine{ farBottomRight,  farTopRight,     color };
    frustumLines_[idx++] = DebugLine{ farTopRight,     farTopLeft,      color };

    // lines from near to far plane
    frustumLines_[idx++] = DebugLine{ nearTopLeft,     farTopLeft,      color };
    frustumLines_[idx++] = DebugLine{ nearBottomLeft,  farBottomLeft,   color };
    frustumLines_[idx++] = DebugLine{ nearTopRight,    farTopRight,     color };
    frustumLines_[idx++] = DebugLine{ nearBottomRight, farBottomRight,  color };
}


} // namespace
