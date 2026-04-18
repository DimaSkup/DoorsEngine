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

#include <geometry/rect3d.h>
#include <geometry/sphere.h>
#include <geometry/sphere_functions.h>
#include <geometry/plane3d.h>
#include <math/math_helpers.h>
#include <math/vec_functions.h>


enum ePlaneClassifications
{
    PLANE_FRONT = 0,
    PLANE_BACK,
    PLANE_INTERSECT
};


//---------------------------------------------------------
// Desc:   testing for intersection between two 3D rectangles,
// Ret:    the resulting rectangle of intersection
//---------------------------------------------------------
inline bool IntersectRect3d(const Rect3d& a, const Rect3d& b, Rect3d& result)
{
    result.x0 = Max(a.x0, b.x0);
    result.y0 = Max(a.y0, b.y0);
    result.z0 = Max(a.z0, b.z0);

    result.x1 = Min(a.x1, b.x1);
    result.y1 = Min(a.y1, b.y1);
    result.z1 = Min(a.z1, b.z1);

    return ((result.x0 <= result.x1) &&
            (result.y0 <= result.y1) &&
            (result.z0 <= result.z1));
}

//---------------------------------------------------------
// Desc:   testing for intersection between two 3D rectangles,
// Ret:    true if have any intersection
//---------------------------------------------------------
inline bool IntersectRect3d(const Rect3d& a, const Rect3d& b)
{
    return (Max(a.x0, b.x0) <= Min(a.x1, b.x1)) &&
           (Max(a.y0, b.y0) <= Min(a.y1, b.y1)) &&
           (Max(a.z0, b.z0) <= Min(a.z1, b.z1));
}

//---------------------------------------------------------
// Desc:  testing for intersectin between a 3D sphere and a 3D plane
//---------------------------------------------------------
inline int PlaneClassify(const Sphere& sphere, const Plane3d& plane)
{
    float d = plane.SignedDistance(sphere.center);

    if (fabs(d) < sphere.radius)
        return PLANE_INTERSECT;

    else if (d > 0)
        return PLANE_FRONT;

    return PLANE_BACK;
}

//---------------------------------------------------------
// Desc:   define intersection type between input 3d rectangle and plane
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
    else if (dMin > 0.0f)
    {
        return PLANE_FRONT;
    }

    return PLANE_BACK;
}

//---------------------------------------------------------
// Desc:  test if input spheres intersect each other
//---------------------------------------------------------
inline bool IntersectSphereSphere(const Sphere& a, const Sphere& b)
{
    Vec3 distVec;
    Vec3Sub(a.center, b.center, distVec);

    const float distSqr      = Vec3Dot(distVec, distVec);
    const float radiusSqrSum = SQR(a.radius) + SQR(b.radius);

    return (distSqr < radiusSqrSum);
}

//---------------------------------------------------------
// Desc:  test if input sphere and AABB intersect each other
//---------------------------------------------------------
inline bool IntersectRectSphere(const Rect3d& rect, const Sphere& sphere)
{
    // left/right
    bool bInByX = (rect.x0 < (sphere.center.x + sphere.radius)) &&
                  (rect.x1 > (sphere.center.x - sphere.radius));

    bool bInByY = (rect.y0 < (sphere.center.y + sphere.radius)) &&
                  (rect.y1 > (sphere.center.y - sphere.radius));

    bool bInByZ = (rect.z0 < (sphere.center.z + sphere.radius)) &&
                  (rect.z1 > (sphere.center.z - sphere.radius));

    return bInByX && bInByY && bInByZ;
}

//---------------------------------------------------------
// Desc:  just test if we have any intersection between the ray and sphere
// Ret:   true if have an intersection
//---------------------------------------------------------
inline bool IntersectRaySphere(
    const Sphere& sphere,
    const Vec3& rayOrig,
    const Vec3& rayDir,
    float& t)
{
    // substitute ray into sphere -> quadration equation in t:
    //                  a*t*t + bt + c = 0

    Vec3 oc = rayOrig - sphere.center;

    float a = Vec3Dot(rayDir, rayDir);
    float b = 2.0f * Vec3Dot(oc, rayDir);
    float c = Vec3Dot(oc, oc) - SQR(sphere.radius);

    float discriminant = b*b - 4*a*c;

    if (discriminant < 0)
        return false;

    float sqrtD = sqrtf(discriminant);

    float t1 = (-b - sqrtD) / (2.0f * a);
    float t2 = (-b + sqrtD) / (2.0f * a);

    // return the closest positive intersection
    if (t1 > 0) return t1;
    if (t2 > 0) return t2;

    // both intersections are behind the ray
    return false;


#if 0
    Vec3 oc = rayOrig - sphere.center;
    float c = Vec3Dot(oc, oc) - SQR(sphere.radius);

    // we're inside the sphere
    if (c < 0.0f)
    {
        t = 0;
        return true;
    }

    float halfB = Vec3Dot(oc, rayDir);

    if (halfB > 0.0f)
        return false;

    // rays are assumed to be normalized so no need to calc a 
    const float a = 1.0f;  // dot(rayDir, rayDir)

    float discriminant = SQR(halfB) - (4.0f*a*c);

    if (discriminant < 0.0f)
        return false;

    t = (-halfB - sqrtf(discriminant));

    return (t >= 0.0f);
#endif
}
