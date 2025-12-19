/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: sphere_functions.h
    Desc:     implementation of sphere class functional

              This code I mainly rewrote from sources to the amazing book
              "Real Time 3D Terrain Engines Using C++ and DirectX"
              by Greg Snook

    Created:  18.09.2025 by DimaSkup
\**********************************************************************************/
#pragma once
#include "sphere.h"

//==================================================================================
// CONSTRUCTORS
//==================================================================================

inline Sphere::Sphere(const Vec3& _center, const float _radius) :
    center(_center),
    radius((_radius >= 0) ? _radius : -_radius)
{
}

//-----------------------------------------------------

inline Sphere::Sphere(const float x, const float y, const float z, const float r) :
    center(x, y, z),
    radius((r >= 0) ? r : -r)
{
}

//-----------------------------------------------------

inline Sphere::Sphere(const Sphere& src) :
    center(src.center),
    radius(src.radius)
{
}

//==================================================================================
// OPERATORS
//==================================================================================
inline bool Sphere::operator == (const Sphere& src) const
{
    return (center == src.center)  &&  (radius == src.radius);
}

//-----------------------------------------------------

inline bool Sphere::operator != (const Sphere& src) const
{
    return !(*this == src);
}

//-----------------------------------------------------

inline const Sphere& Sphere::operator = (const Sphere& src)
{
    center = src.center;
    radius = src.radius;
    return *this;
}


//==================================================================================
// CALCULATIONS / OPERATIONS
//==================================================================================

inline void Sphere::Clear(void)
{
    center.x = 0;
    center.y = 0;
    center.z = 0;
    radius = 0;
}

//-----------------------------------------------------

inline bool Sphere::IsClear(void) const
{
    return (center.x == 0 &&
            center.y == 0 &&
            center.z == 0 &&
            radius == 0);
}

//-----------------------------------------------------

inline void Sphere::Set(const Vec3& _center, const float _radius)
{
    center = _center;
    radius = (_radius >= 0) ? _radius : -_radius;
}

//-----------------------------------------------------

inline void Sphere::Set(const float x, const float y, const float z, const float r)
{
    center.x = x;
    center.y = y;
    center.z = z;
    radius = (r >= 0) ? r : -r;
}

//-----------------------------------------------------

inline void Sphere::Set(const Sphere& src)
{
    center = src.center;
    radius = src.radius;
}

//-----------------------------------------------------

inline bool Sphere::IsValid() const
{
    return radius >= 0;
}

//-----------------------------------------------------

inline void Sphere::Offset(const Vec3& offset)
{
    center.x += offset.x;
    center.y += offset.y;
    center.z += offset.z;
}

//-----------------------------------------------------

inline void Sphere::Expand(const float n)
{
    radius += n;
}

//-----------------------------------------------------

inline void Sphere::Normalize()
{
    radius = (radius >= 0) ? radius : -radius;
}
