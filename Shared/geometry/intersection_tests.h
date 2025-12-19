/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: intersection_tests.cpp
    Desc:     implementation for a lot of intersection tests

              This code I mainly rewrote from sources to the amazing book
              "Real Time 3D Terrain Engines Using C++ and DirectX"
              by Greg Snook

    Created:  17.09.2025 by DimaSkup
\**********************************************************************************/
#pragma once

#include <geometry/rect_3d.h>
#include <geometry/sphere.h>
#include <geometry/sphere_functions.h>
#include <geometry/plane_3d.h>
#include <math/math_helpers.h>



enum ePlaneClassifications
{
    PLANE_FRONT = 0,
    PLANE_BACK,
    PLANE_INTERSECT
};


//---------------------------------------------------------
// Desc:   test if two input 3d rectangles intersects at all
//         if so we write an intersection volume into the output result
//---------------------------------------------------------
inline bool IntersectRect3d(const Rect3d& a, const Rect3d& b, Rect3d& result)
{
    result.x0 = Max(a.x0, b.x0);
    result.y0 = Max(a.y0, b.y0);
    result.z0 = Max(a.z0, b.z0);

    result.x1 = Min(a.x1, b.x1);
    result.y1 = Min(a.y1, b.y1);
    result.z1 = Min(a.z1, b.z1);

    return (result.x0 <= result.x1 &&
            result.y0 <= result.y1 &&
            result.z0 <= result.z1);
}

//---------------------------------------------------------
// Desc:   define intersection type btw input 3d rectangle and plane
//         (rect can be completely in front, behind or be intersected by the plane)
//---------------------------------------------------------
inline int PlaneClassify(const Rect3d& rect, const Plane3d& plane)
{
    Vec3 minPoint, maxPoint;

    // build two points based on the direction of the plane vector. minPoint and
    // maxPoint are two points on the rectangle furthest away from each other
    // along the plane normal

    if (plane.normal.x > 0.0f)
    {
        minPoint.x = rect.x0;
        maxPoint.x = rect.x1;
    }
    else
    {
        minPoint.x = rect.x1;
        maxPoint.x = rect.x0;
    }

    if (plane.normal.y > 0.0f)
    {
        minPoint.y = rect.y0;
        maxPoint.y = rect.y1;
    }
    else
    {
        minPoint.y = rect.y1;
        maxPoint.y = rect.y0;
    }

    if (plane.normal.z > 0.0f)
    {
        minPoint.z = rect.z0;
        maxPoint.z = rect.z1;
    }
    else
    {
        minPoint.z = rect.z1;
        maxPoint.z = rect.z0;
    }

    // compute signed distance from the plane to both points
    const float dMin = plane.SignedDistance(minPoint);
    const float dMax = plane.SignedDistance(maxPoint);

    // the rect intersects the plane if one dist is positive and the other is negative
    if (dMin * dMax < 0.0f)
    {
        return PLANE_INTERSECT;
    }
    else if (dMin >= 0.0f)
    {
        return PLANE_FRONT;
    }

    return PLANE_BACK;
}

//---------------------------------------------------------
// Desc:   testing for intersection btw a 3D sphere and a 3D plane
//---------------------------------------------------------
inline int PlaneClassify(const Sphere& sphere, const Plane3d& plane)
{
    const float d = plane.SignedDistance(sphere.center);

    if (fabs(d) < sphere.radius)
        return PLANE_INTERSECT;

    else if (d)
        return PLANE_FRONT;

    return PLANE_BACK;
}
