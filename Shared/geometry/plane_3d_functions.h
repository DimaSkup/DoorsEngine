/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: plane_3d.cpp
    Desc:     normalized 3d plane implementation

              This code I mainly rewrote from sources to the amazing book
              "Real Time 3D Terrain Engines Using C++ and DirectX"
              by Greg Snook

    Created:  17.09.2025 by DimaSkup
\**********************************************************************************/
#pragma once
#include "plane_3d.h"
#include "../math/vec_functions.h"


//==================================================================================
// CREATORS
//==================================================================================

//---------------------------------------------------------
// Desc:   default constructor
//---------------------------------------------------------
inline Plane3d::Plane3d() : 
    normal(0, 0, 0), 
    distance(0) 
{
}

//---------------------------------------------------------
// Desc:   init a plane with input components for normal-vector and distance to plane
//---------------------------------------------------------
inline Plane3d::Plane3d(const float nx, const float ny, const float nz, const float dist) :
    normal(nx, ny, nz),
    distance(dist)
{
}

//---------------------------------------------------------
// Desc:   copy constructor
//---------------------------------------------------------
inline Plane3d::Plane3d(const Plane3d& pl) :
    normal(pl.normal),
    distance(pl.distance)
{
}

//---------------------------------------------------------
// Desc:    input vec <nx, ny, nz, distance>
//---------------------------------------------------------
inline Plane3d::Plane3d(const Vec4& v) :
    normal(v.x, v.y, v.z),
    distance(v.w)
{
}

//---------------------------------------------------------
// Desc:   init a plane by input 3d points
//---------------------------------------------------------
inline Plane3d::Plane3d(const Vec3& p0, const Vec3& p1, const Vec3& p2)
{
    Set(p0, p1, p2);
}

//---------------------------------------------------------
// Desc:   init a plane with input normal-vector and distance
//---------------------------------------------------------
inline Plane3d::Plane3d(const Vec3& _normal, const float _distance) :
    normal(_normal),
    distance(_distance)
{
}

//---------------------------------------------------------
// Desc:   compute a plane by input 3d point and normal-vector
//---------------------------------------------------------
inline Plane3d::Plane3d(const Vec3& point, const Vec3& normal)
{
    Set(point, normal);
}

//---------------------------------------------------------
// Desc:   just destructor
//---------------------------------------------------------
inline Plane3d::~Plane3d()
{
}

//==================================================================================
// OPERATORS
//==================================================================================

inline Plane3d& Plane3d::operator = (const Plane3d& src)
{
    normal   = src.normal;
    distance = src.distance;

    return (*this);
}

//---------------------------------------------------------

inline bool Plane3d::operator == (const Plane3d& plane) const
{
    return (normal == plane.normal && FloatEqual(distance, plane.distance));
}

//---------------------------------------------------------

inline bool Plane3d::operator != (const Plane3d& plane) const
{
    return !(*this == plane);
}

//==================================================================================
// MUTATORS / SETTERS / OPERATIONS
//==================================================================================

inline void Plane3d::Normalize()
{
    const float invMag = 1.0f / Vec3Length(normal);

    normal.x *= invMag;
    normal.y *= invMag;
    normal.z *= invMag;
    distance *= invMag;
}

//---------------------------------------------------------
// Desc:   given Y and Z, solve for X on the plane
//---------------------------------------------------------
inline float Plane3d::SolveForX(const float Y, const float Z) const
{
    // Ax + By + Cz + D = 0
    // Ax = -(By + Cz + D)
    // x = -(By + Cz + D) / A

    if (normal.x)
    {
        return -(normal.y*Y + normal.z*Z + distance) / normal.x;
    }

    return 0.0f;
}

//---------------------------------------------------------
// Desc:   given X and Z, solve for Y on the plane
//---------------------------------------------------------
inline float Plane3d::SolveForY(const float X, const float Z) const
{
    // Ax + By + Cz + D = 0
    // By = -(Ax + Cz + D)
    // y = -(Ax + Cz + D) / B

    if (normal.y)
    {
        return -(normal.x*X + normal.z*Z + distance) / normal.y;
    }

    return 0.0f;
}

//---------------------------------------------------------
// Desc:   given X and Y, solve for Z on the plane
//---------------------------------------------------------
inline float Plane3d::SolveForZ(const float X, const float Y) const
{
    // Ax + By + Cz + D = 0
    // Cz = -(Ax + By + D)
    // z = -(Ax + By + D) / C

    if (normal.z)
    {
        return -(normal.x*X + normal.y*Y + distance) / normal.z;
    }

    return 0.0f;
}

//---------------------------------------------------------
// Desc:   given a point in 3D space, project the point onto this plane
//         along the plane normal
//---------------------------------------------------------
inline Vec3 Plane3d::ProjectPointToPlane(const Vec3& point) const
{
    return point + (normal * -SignedDistance(point));
}

//---------------------------------------------------------
// Desc:   setup a plane with given a clockwise ordering of 3D points
//---------------------------------------------------------
inline void Plane3d::Set(const Vec3& p0, const Vec3& p1, const Vec3& p2)
{
    const Vec3 vecA(p1 - p0);
    const Vec3 vecB(p2 - p0);

    normal = Vec3Cross(vecA, vecB);
    Vec3Normalize(normal);

    distance = -Vec3Dot(normal, p0);
}

//---------------------------------------------------------
// Desc:   setup a plane with input normal-vector and distance
//---------------------------------------------------------
inline void Plane3d::Set(const Vec3& norm, const float dist)
{
    normal   = norm;
    distance = dist;
}

//---------------------------------------------------------
// Desc:   setup a plane with input normal-vector and 3d point on plane
// 
// NOTE:   a resulting distance is correct only if normal vector is normalized
//---------------------------------------------------------
inline void Plane3d::Set(const Vec3& p, const Vec3& n)
{
    normal   = n;
    distance = -Vec3Dot(normal, p);
}

//---------------------------------------------------------
// Desc:   transform a plane using tranformation 4x4 matrix
// Args:   - m:   an INVERSE matrix of the original transformation matrix
//                (1. for computations speed up: in cases when we want to trasform
//                    multiple plane with the same transformation matrix;
//                 2. not inverse transpose but only inverse because of row-major order)
//---------------------------------------------------------
inline void Plane3d::Transform(const Matrix& inverse)
{
    // D` = D - dot(normal, T*invM)
    Vec4 tmp;
    MatrixMulVec4(plane, inverse, tmp);
    plane = tmp;
}

//---------------------------------------------------------
// Desc:   returns the signed distance btw the plane and
//         the provided 3D point. Negative distances are "behind"
//         the plane, i.e. in the opposite direction of the plane normal
//---------------------------------------------------------
inline float Plane3d::SignedDistance(const Vec3& point) const
{
    return Vec3Dot(normal, point) + distance;
}
