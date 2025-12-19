/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: sphere.h
    Desc:     just sphere

              This code I mainly rewrote from sources to the amazing book
              "Real Time 3D Terrain Engines Using C++ and DirectX"
              by Greg Snook

    Created:  18.09.2025 by DimaSkup
\**********************************************************************************/
#pragma once

#include <math/vec3.h>

class Sphere
{
public:

    //-----------------------------------------------------
    // public data
    //-----------------------------------------------------
    float radius = 0.0f;
    Vec3  center = { 0,0,0 };


    //-----------------------------------------------------
    // creators
    //-----------------------------------------------------
    Sphere() {};
    Sphere(const Vec3& _center, const float _radius);
    Sphere(const float x, const float y, const float z, const float r);
    Sphere(const Sphere& src);
    ~Sphere(void) {};


    //-----------------------------------------------------
    // operators
    //-----------------------------------------------------
    bool operator == (const Sphere& src) const;
    bool operator != (const Sphere& src) const;

    const Sphere& operator = (const Sphere& src);


    //-----------------------------------------------------
    // calculations / operations
    //-----------------------------------------------------
    void Clear();
    bool IsClear() const;
    bool IsValid() const;
    void Normalize();

    void Set(const Vec3& _center, const float _radius);
    void Set(const float x, const float y, const float z, const float r);
    void Set(const Sphere& src);

    //float Volume() const;
    void Offset(const Vec3& offset);
    void Expand(const float n);
};
