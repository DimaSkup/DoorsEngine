/***************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: rect_3d.h
    Desc:     3d rectangle class declaration (a lightweight version,
              for using its functional you need to include "rect_3d_functions" header)

              This code I mainly rewrote from sources to the amazing book
              "Real Time 3D Terrain Engines Using C++ and DirectX"
              by Greg Snook

    Created:  14.09.2025 by DimaSkup
\***************************************************************/
#pragma once

#include "../math/vec3.h"

class Rect3d
{
public:

    //-----------------------------------------------------
    // public data
    //-----------------------------------------------------
    union
    {
        struct
        {
            float x0, x1;
            float y0, y1;
            float z0, z1;
        };
    };

    //-----------------------------------------------------
    // creators
    //-----------------------------------------------------
    Rect3d();
    Rect3d(const float _x0, const float _x1,
           const float _y0, const float _y1,
           const float _z0, const float _z1);

    Rect3d(const Rect3d& src);
    Rect3d(const float xSize, const float ySize, const float zSize);
    Rect3d(const Vec3& size);
    ~Rect3d() {};

    //-----------------------------------------------------
    // operators
    //-----------------------------------------------------

    // negate
    Rect3d operator - () const;

    // equality
    bool operator == (const Rect3d& src) const;

    // inequality
    bool operator != (const Rect3d& src) const;

    // copy
    const Rect3d& operator = (const Rect3d& src);
    const Rect3d& operator = (const Vec3& size);

    const Rect3d& operator += (const Vec3& vec);
    const Rect3d& operator += (const float s);

    const Rect3d& operator -= (const Vec3& vec);
    const Rect3d& operator -= (const float s);

    const Rect3d& operator *= (const Vec3& vec);
    const Rect3d& operator *= (const float s);

    const Rect3d& operator /= (const Vec3& vec);
    const Rect3d& operator /= (const float s);

    //-----------------------------------------------------
    // calculations / operatorsions
    //-----------------------------------------------------
    void Clear();
    bool IsClear() const;
    bool IsValid() const;
    void AssertValid() const;
    void Normalize();

    void SetFloor  (const Rect3d& input);
    void SetCeiling(const Rect3d& input);

    void ResizeX(const float span);
    void ResizeY(const float span);
    void ResizeZ(const float span);
    void Resize (const Vec3& size);

    void ResizeMinX(const float span);
    void ResizeMinY(const float span);
    void ResizeMinZ(const float span);
    void ResizeMin (const Vec3& size);

    void ResizeMaxX(const float span);
    void ResizeMaxY(const float span);
    void ResizeMaxZ(const float span);
    void ResizeMax (const Vec3& size);

    float SizeX() const;
    float SizeY() const;
    float SizeZ() const;
    Vec3  Size()  const;

    float MidX() const;
    float MidY() const;
    float MidZ() const;
    Vec3 MidPoint() const;
    Vec3 MinPoint() const;
    Vec3 MaxPoint() const;

    float Volume() const;

    void Expand(const float n);
    void Expand(const Vec3& size);

    void ExpandX(const float n);
    void ExpandY(const float n);
    void ExpandZ(const float n);

    bool PointInRect(const float x, const float y, const float z) const;
    bool PointInRect(const Vec3& point) const;

    //Sphere CreateBoundingSphere() const;
};
