/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: frustum.h
    Desc:     a Frustum is a set of six Plane3d objects representing camera space.
              These planes are extracted from a camera matrix directly or from
              camera's field of view (fov), aspect ration, near and far plane distance.

              NOTE: the planes of a Frustum object are not normalized!!!
              This means they are only sutable for half-space testing.
              No distance values calculated using these planes will be accurate
              other than to show whether positions lie in the positive or negative
              half-space of the plane.

              This code I mainly rewrote from sources to the amazing book
              "Real Time 3D Terrain Engines Using C++ and DirectX"
              by Greg Snook

    Created:  17.09.2025 by DimaSkup
\**********************************************************************************/
#pragma once

#include <math/matrix.h>
#include <geometry/plane_3d.h>
#include <geometry/rect_3d.h>
#include <geometry/sphere.h>
#include <geometry/intersection_tests.h>

class Frustum
{
public:
    enum ePlaneClassifications
    {
        PLANE_FRONT = 0,
        PLANE_BACK,
        PLANE_INTERSECT
    };

    //-----------------------------------------------------
    // public data
    //-----------------------------------------------------
    Plane3d leftPlane_;
    Plane3d rightPlane_;
    Plane3d topPlane_;
    Plane3d bottomPlane_;
    Plane3d nearPlane_;
    Plane3d farPlane_;

    // sizes in view space (for frustum corner points computation)
    float nearPlaneWidth_  = 0;
    float nearPlaneHeight_ = 0;
    float farPlaneWidth_   = 0;
    float farPlaneHeight_  = 0;

    //-----------------------------------------------------
    // creators
    //-----------------------------------------------------
    Frustum();
    Frustum(const float fov, const float aspectRatio, const float zn, const float zf);
    Frustum(const Plane3d& l, const Plane3d& r, const Plane3d& t, const Plane3d& b, const Plane3d& n, const Plane3d& f);
    Frustum(const Matrix& proj);
    ~Frustum();

    //-----------------------------------------------------
    // mutators / setters
    //-----------------------------------------------------
    void Init(
        const float fov,
        const float aspectRatio,
        const float nearZ,
        const float farZ);

    void CreateFromProjMatrix(const Matrix& proj, const bool normalizePlanes = false);

    void Transform(Frustum& outFrustum, const Matrix& mat) const;

    void GetPoints(
        Vec3& nearTopLeft,
        Vec3& nearBottomLeft,
        Vec3& nearTopRight,
        Vec3& nearBottomRight,
        Vec3& farTopLeft,
        Vec3& farBottomLeft,
        Vec3& farTopRight,
        Vec3& farBottomRight);


    //-----------------------------------------------------
    // test operations
    //-----------------------------------------------------
    bool TestPoint(const Vec3& point) const;
    bool TestRect(const Rect3d& rect) const;
    bool TestSphere(const Sphere& sphere) const;
};
