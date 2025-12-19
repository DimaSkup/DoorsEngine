/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: plane_3d.cpp
    Desc:     normalized 3d plane:
              is represented in point-normal for (Ax + By + Cz + D = 0).

              The convention for the distance constant D is:

                       D = -(A,B,C) dot (X,Y,Z)


              This code I mainly rewrote from sources to the amazing book
              "Real Time 3D Terrain Engines Using C++ and DirectX"
              by Greg Snook

    Created:  17.09.2025 by DimaSkup
\**********************************************************************************/
#pragma once

#include <math/vec3.h>
#include <math/vec4.h>
#include <math/matrix.h>

class Plane3d
{
public:
    //-----------------------------------------------------
    // public data
    //-----------------------------------------------------
    union
    {
        float m[4]{0};
        struct
        {
            Vec3  normal;
            float distance;
        };
        struct
        {
            Vec4 plane;
        };
    };

    //-----------------------------------------------------
    // creators
    //-----------------------------------------------------
    Plane3d();

    Plane3d(const float nx, const float ny, const float nz, const float dist);
    Plane3d(const Plane3d& src);
    Plane3d(const Vec4& normalAndDist);
    Plane3d(const Vec3& p0, const Vec3& p1, const Vec3& p2);
    Plane3d(const Vec3& normal, const float distance);
    Plane3d(const Vec3& point, const Vec3& normal);
    ~Plane3d();

    //-----------------------------------------------------
    // operators
    //-----------------------------------------------------
    Plane3d& operator = (const Plane3d& src);
    bool operator == (const Plane3d& plane) const;
    bool operator != (const Plane3d& plane) const;

    //-----------------------------------------------------
    // mutators / setters
    //-----------------------------------------------------
    void Set(const Vec3& p0, const Vec3& p1, const Vec3& p2);
    void Set(const Vec3& normal, const float distance);
    void Set(const Vec3& point, const Vec3& normal);

    void Normalize();
    void Transform(const Matrix& inverse);

    //-----------------------------------------------------
    // point operations
    //-----------------------------------------------------
    float SignedDistance(const Vec3& point) const;
    float SolveForX(const float y, const float z) const;
    float SolveForY(const float x, const float z) const;
    float SolveForZ(const float x, const float y) const;

    Vec3 ProjectPointToPlane(const Vec3& point) const;
};
