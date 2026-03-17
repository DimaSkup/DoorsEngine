/***************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: rect_3d.h
    Desc:     3d rectangle implementation

              This code I mainly rewrote from sources to the amazing book
              "Real Time 3D Terrain Engines Using C++ and DirectX"
              by Greg Snook

    Created:  14.09.2025 by DimaSkup
\***************************************************************/
#pragma once
#include "rect3d.h"
#include "../math/matrix.h"
#include <assert.h>
#include <math.h>
#include <float.h>
#include <stdio.h>


//==================================================================================
// constructors
//==================================================================================

inline Rect3d::Rect3d(const float _x0, const float _x1,
                      const float _y0, const float _y1,
                      const float _z0, const float _z1) :
    x0(_x0), x1(_x1),
    y0(_y0), y1(_y1),
    z0(_z0), z1(_z1)
{
}

//---------------------------------------------------------
// build rectangle from a cloud of points
//---------------------------------------------------------
inline Rect3d::Rect3d(const Vec3* points, const int numPoints)
{
    BuildAABB(points, numPoints);
}

//---------------------------------------------------------

inline Rect3d::Rect3d(const Rect3d& src) :
    x0(src.x0), x1(src.x1),
    y0(src.y0), y1(src.y1),
    z0(src.z0), z1(src.z1)
{
}

//---------------------------------------------------------
// c - box center
// e - box extents
//---------------------------------------------------------
inline Rect3d::Rect3d(Vec3&& c, Vec3&& e)
{
    x0 = c.x - e.x;
    x1 = c.x + e.x;

    y0 = c.y - e.y;
    y1 = c.y + e.y;

    z0 = c.z - e.z;
    z1 = c.z + e.z;
}

//---------------------------------------------------------

inline Rect3d::Rect3d(const float xSize, const float ySize, const float zSize) :
    x0(0), x1(0),
    y0(0), y1(0),
    z0(0), z1(0)
{
    Resize(Vec3(xSize, ySize, zSize));
}

//---------------------------------------------------------

inline Rect3d::Rect3d(const Vec3& size) :
    x0(0), x1(0),
    y0(0), y1(0),
    z0(0), z1(0)
{
    Resize(size);
}

//---------------------------------------------------------
// print out this rectangle into console
//---------------------------------------------------------
inline void Rect3d::Print(const char* msg) const
{
    if (msg && msg[0] != '\0')
        printf("%s\n", msg);

    printf("   x0, x1 = %.2f  %.2f\n" 
           "   y0, y1 = %.2f  %.2f\n" 
           "   z0, z1 = %.2f  %.2f\n",
           x0, x1, y0, y1, z0, z1);
}

//==================================================================================
// operators: equality, inequality, assignment
//==================================================================================
inline bool Rect3d::operator == (const Rect3d& src) const
{
    return (x0 == src.x0  &&  x1 == src.x1  &&
            y0 == src.y0  &&  y1 == src.y1  &&
            z0 == src.z0  &&  z1 == src.z1);
}

inline bool Rect3d::operator != (const Rect3d& src) const
{
    return (x0 != src.x0  ||  x1 != src.x1  ||
            y0 != src.y0  ||  y1 != src.y1  ||
            z0 != src.z0  ||  z1 != src.z1);
}

inline const Rect3d& Rect3d::operator = (const Rect3d& src)
{
    x0 = src.x0;
    x1 = src.x1;
    y0 = src.y0;
    y1 = src.y1;
    z0 = src.z0;
    z1 = src.z1;
    return (*this);
}

//==================================================================================
// operators: addition
//==================================================================================

inline const Rect3d& Rect3d::operator += (const Vec3& vec)
{
    x0 += vec.x;
    x1 += vec.x;
    y0 += vec.y;
    y1 += vec.y;
    z0 += vec.z;
    z1 += vec.z;
    return (*this);
}

//---------------------------------------------------------

inline const Rect3d& Rect3d::operator += (const float s)
{
    x0 += s;
    x1 += s;
    y0 += s;
    y1 += s;
    z0 += s;
    z1 += s;
    return (*this);
}

//---------------------------------------------------------

inline Rect3d operator + (const Rect3d& lha, const Vec3& rha)
{
    return Rect3d(lha.x0 + rha.x, lha.x1 + rha.x,
                  lha.y0 + rha.y, lha.y1 + rha.y,
                  lha.z0 + rha.z, lha.z1 + rha.z);
}

//---------------------------------------------------------

inline Rect3d operator + (const Rect3d& lha, const float rha)
{
    return Rect3d(lha.x0 + rha, lha.x1 + rha,
                  lha.y0 + rha, lha.y1 + rha,
                  lha.z0 + rha, lha.z1 + rha);
}

//---------------------------------------------------------

inline Rect3d operator + (const float lha, const Rect3d& rha)
{
    return rha + lha;
}

//==================================================================================
// operators: subtraction, negation
//==================================================================================

inline const Rect3d& Rect3d::operator -= (const Vec3& vec)
{
    x0 -= vec.x;
    x1 -= vec.x;
    y0 -= vec.y;
    y1 -= vec.y;
    z0 -= vec.z;
    z1 -= vec.z;
    return (*this);
}

//---------------------------------------------------------

inline const Rect3d& Rect3d::operator -= (const float s)
{
    x0 -= s;
    x1 -= s;
    y0 -= s;
    y1 -= s;
    z0 -= s;
    z1 -= s;
    return (*this);
}

//---------------------------------------------------------

inline Rect3d operator - (const Rect3d& lha, const Vec3& rha)
{
    return Rect3d(lha.x0 - rha.x, lha.x1 - rha.x,
                  lha.y0 - rha.y, lha.y1 - rha.y,
                  lha.z0 - rha.z, lha.z1 - rha.z);
}

//---------------------------------------------------------

inline Rect3d operator - (const Rect3d& lha, const float s)
{
    return Rect3d(lha.x0 - s, lha.x1 - s,
                  lha.y0 - s, lha.y1 - s,
                  lha.z0 - s, lha.z1 - s);
}

//---------------------------------------------------------

inline Rect3d Rect3d::operator - () const
{
    return Rect3d(-x1, -x0, -y1, -y0, -z1, -z0);
}

//==================================================================================
// operators: multiplication
//==================================================================================

inline const Rect3d& Rect3d::operator *= (const float s)
{
    x0 *= s;
    x1 *= s;
    y0 *= s;
    y1 *= s;
    z0 *= s;
    z1 *= s;
    return (*this);
}

//---------------------------------------------------------

inline const Rect3d& Rect3d::operator *= (const Vec3& vec)
{
    x0 *= vec.x;
    x1 *= vec.x;
    y0 *= vec.y;
    y1 *= vec.y;
    z0 *= vec.z;
    z1 *= vec.z;
    return (*this);
}

//---------------------------------------------------------

inline Rect3d operator * (const Rect3d& lha, const Vec3& rha)
{
    return Rect3d(lha.x0 * rha.x, lha.x1 * rha.x,
                  lha.y0 * rha.y, lha.y1 * rha.y,
                  lha.z0 * rha.z, lha.z1 * rha.z);
}

//---------------------------------------------------------

inline Rect3d operator * (const Vec3& lha, const Rect3d& rha)
{
    return rha * lha;
}

//---------------------------------------------------------

inline Rect3d operator * (const Rect3d& lha, const float rha)
{
    return Rect3d(lha.x0 * rha, lha.x1 * rha,
                  lha.y0 * rha, lha.y1 * rha,
                  lha.z0 * rha, lha.z1 * rha);
}

//---------------------------------------------------------

inline Rect3d operator * (const float lha, const Rect3d& rha)
{
    return rha * lha;
}

//==================================================================================
// operators: division
//==================================================================================

inline const Rect3d& Rect3d::operator /= (const float s)
{
    assert(s != 0.0f && "divide by zero error");

    const float invScalar = 1.0f / s;

    x0 *= invScalar;
    x1 *= invScalar;
    y0 *= invScalar;
    y1 *= invScalar;
    z0 *= invScalar;
    z1 *= invScalar;

    return (*this);
}

//---------------------------------------------------------

inline const Rect3d& Rect3d::operator /= (const Vec3& vec)
{
    assert(vec.x != 0.0f && "divide by zero error");
    assert(vec.y != 0.0f && "divide by zero error");
    assert(vec.z != 0.0f && "divide by zero error");

    x0 /= vec.x;
    x1 /= vec.x;
    y0 /= vec.y;
    y1 /= vec.y;
    z0 /= vec.z;
    z1 /= vec.z;

    return (*this);
}

//---------------------------------------------------------

inline Rect3d operator / (const Rect3d& lha, const Vec3& rha)
{
    assert(rha.x != 0.0f && "divide by zero error");
    assert(rha.y != 0.0f && "divide by zero error");
    assert(rha.z != 0.0f && "divide by zero error");

    return Rect3d(lha.x0 / rha.x, lha.x1 / rha.x,
                  lha.y0 / rha.y, lha.y1 / rha.y,
                  lha.z0 / rha.z, lha.z1 / rha.z);
}

//---------------------------------------------------------


inline Rect3d operator / (const Rect3d& lha, const float rha)
{
    assert(rha != 0.0f && "divide by zero error");

    const float invRha = 1.0f / rha;

    return Rect3d(lha.x0 * invRha, lha.x1 * invRha,
                  lha.y0 * invRha, lha.y1 * invRha,
                  lha.z0 * invRha, lha.z1 * invRha);
}

//==================================================================================
// calculations / operations
//==================================================================================

//---------------------------------------------------------
// add a single point to 3d rectangle
//---------------------------------------------------------
inline void Rect3d::UnionPoint(const Vec3& p)
{
    x0 = Min(x0, p.x);
    y0 = Min(y0, p.y);
    z0 = Min(z0, p.z);

    x1 = Max(x1, p.x);
    y1 = Max(y1, p.y);
    z1 = Max(z1, p.z);
}

//---------------------------------------------------------
// build an axis-aligned bounding box by input points cloud
//---------------------------------------------------------
inline void Rect3d::BuildAABB(const Vec3* points, const int numPoints)
{
    assert(points && numPoints > 0);

    x0 = y0 = z0 = FLT_MAX;
    x1 = y1 = z1 = FLT_MIN;

    for (int i = 0; i < numPoints; ++i)
    {
        x0 = Min(points[i].x, x0);
        y0 = Min(points[i].y, y0);
        z0 = Min(points[i].z, z0);

        x1 = Max(points[i].x, x1);
        y1 = Max(points[i].y, y1);
        z1 = Max(points[i].z, z1);
    }
}

//---------------------------------------------------------

inline void Rect3d::Clear(void)
{
    x0 = x1 = y0 = y1 = z0 = z1 = 0;
}

//---------------------------------------------------------

inline bool Rect3d::IsClear(void) const
{
    return ((x0 == 0.0f)  &&  (x1 == 0.0f)  &&
            (y0 == 0.0f)  &&  (y1 == 0.0f)  &&
            (z0 == 0.0f)  &&  (z1 == 0.0f));
}

//---------------------------------------------------------

inline void Rect3d::SetFloor(const Rect3d& input)
{
    x0 = floorf(input.x0);
    x1 = floorf(input.x1);
    y0 = floorf(input.y0);
    y1 = floorf(input.y1);
    z0 = floorf(input.z0);
    z1 = floorf(input.z1);
}

//---------------------------------------------------------

inline void Rect3d::SetCeiling(const Rect3d& input)
{
    x0 = ceilf(input.x0);
    x1 = ceilf(input.x1);
    y0 = ceilf(input.y0);
    y1 = ceilf(input.y1);
    z0 = ceilf(input.z0);
    z1 = ceilf(input.z1);
}

//---------------------------------------------------------

inline bool Rect3d::IsValid() const
{
    return (x0 <= x1  &&  y0 <= y1  &&  z0 <= z1);
}

//---------------------------------------------------------

inline void Rect3d::AssertValid() const
{
    assert((x0 <= x1) && "rectangle inverted on X axis");
    assert((y0 <= y1) && "rectangle inverted on Y axis");
    assert((z0 <= z1) && "rectangle inverted on Z axis");
}

//---------------------------------------------------------

inline void Rect3d::ResizeX(const float size)
{
    x1 = MidX() + (size*0.5f);
    x0 = x1 - size;
}

//---------------------------------------------------------

inline void Rect3d::ResizeY(const float size)
{
    y1 = MidY() + (size*0.5f);
    y0 = y1 - size;
}

//---------------------------------------------------------

inline void Rect3d::ResizeZ(const float size)
{
    z1 = MidZ() + (size*0.5f);
    z0 = z1 - size;
}

//---------------------------------------------------------

inline void Rect3d::Resize(const Vec3& size)
{
    ResizeX(size.x);
    ResizeY(size.y);
    ResizeZ(size.z);
}

//---------------------------------------------------------

inline void Rect3d::ResizeMaxX(const float span)
{
    x1 = x0 + span;
}

//---------------------------------------------------------

inline void Rect3d::ResizeMaxY(const float span)
{
    y1 = y0 + span;
}
 
//---------------------------------------------------------

inline void Rect3d::ResizeMaxZ(const float span)
{
    z1 = z0 + span;
}

//---------------------------------------------------------

inline void Rect3d::ResizeMax(const Vec3& size)
{
    ResizeMaxX(size.x);
    ResizeMaxY(size.y);
    ResizeMaxZ(size.z);
}

//---------------------------------------------------------

inline void Rect3d::ResizeMinX(const float span)
{
    x0 = x1 - span;
}

//---------------------------------------------------------

inline void Rect3d::ResizeMinY(const float span)
{
    y0 = y1 - span;
}

//---------------------------------------------------------

inline void Rect3d::ResizeMinZ(const float span)
{
    z0 = z1 - span;
}

//---------------------------------------------------------

inline void Rect3d::ResizeMin(const Vec3& size)
{
    ResizeMinX(size.x);
    ResizeMinY(size.y);
    ResizeMinZ(size.z);
}

//---------------------------------------------------------

inline float Rect3d::MidX() const
{
    return (x0+x1) * 0.5f;
}

//---------------------------------------------------------

inline float Rect3d::MidY() const
{
    return (y0+y1) * 0.5f;
}

//---------------------------------------------------------

inline float Rect3d::MidZ() const
{
    return (z0+z1) * 0.5f;
}

//---------------------------------------------------------

inline Vec3 Rect3d::MidPoint() const
{
    return Vec3(MidX(), MidY(), MidZ());
}

//---------------------------------------------------------

inline Vec3 Rect3d::Extents() const
{
    Vec3 c = MidPoint();
    Vec3 m = MaxPoint();
    return { m.x-c.x, m.y-c.y, m.z-c.z };
}

//---------------------------------------------------------

inline float Rect3d::SizeX() const
{
    return x1 - x0;
}

//---------------------------------------------------------

inline float Rect3d::SizeY() const
{
    return y1 - y0;
}

//---------------------------------------------------------

inline float Rect3d::SizeZ() const
{
    return z1 - z0;
}

//---------------------------------------------------------

inline Vec3 Rect3d::Size() const
{
    return Vec3(SizeX(), SizeY(), SizeZ());
}

//---------------------------------------------------------

inline Vec3 Rect3d::MinPoint() const
{
    return Vec3(x0, y0, z0);
}

//---------------------------------------------------------

inline Vec3 Rect3d::MaxPoint() const
{
    return Vec3(x1, y1, z1);
}

//---------------------------------------------------------

inline float Rect3d::Volume() const
{
    return SizeX() * SizeY() * SizeZ();
}

//---------------------------------------------------------

inline void Rect3d::Expand(const float n)
{
    ExpandX(n);
    ExpandY(n);
    ExpandZ(n);
}

//---------------------------------------------------------

inline void Rect3d::ExpandX(const float n)
{
    x0 -= n;
    x1 += n;
}

//---------------------------------------------------------

inline void Rect3d::ExpandY(const float n)
{
    y0 -= n;
    y1 += n;
}

//---------------------------------------------------------

inline void Rect3d::ExpandZ(const float n)
{
    z0 -= n;
    z1 += n;
}

//---------------------------------------------------------

inline void Rect3d::Expand(const Vec3& size)
{
    ExpandX(size.x);
    ExpandY(size.y);
    ExpandZ(size.z);
}

//---------------------------------------------------------

inline void Rect3d::Normalize()
{
    float tmp = 0.0f;

    if (x0 > x1)
    {
        tmp = x0;
        x0 = x1;
        x1 = tmp;
    }

    if (y0 > y1)
    {
        tmp = y0;
        y0 = y1;
        y1 = tmp;
    }

    if (z0 > z1)
    {
        tmp = z0;
        z0 = z1;
        z1 = tmp;
    }
}

//---------------------------------------------------------

inline bool Rect3d::IsPointInRect(const float x, const float y, const float z) const
{
    return x >= x0  &&  x <= x1  &&
           y >= y0  &&  y <= y1  &&
           z >= z0  &&  z <= z1;
}

//---------------------------------------------------------

inline bool Rect3d::IsPointInRect(const Vec3& p) const
{
    return p.x >= x0  &&  p.x <= x1  &&
           p.y >= y0  &&  p.y <= y1  &&
           p.z >= z0  &&  p.z <= z1;
}

//---------------------------------------------------------
// Desc:  transform this rectangle (AABB) using input matrix
//        so as a result we will get a new AABB
//---------------------------------------------------------
inline void Rect3d::Transform(const Matrix* m)
{
    const Vec3 sz  = Size();
    const Vec3 min = MinPoint();
    const Vec3 max = MaxPoint();

    Vec3 pin[8], pout[8];

    // build all 8 points of aabb
    pin[0] = { min.x,      min.y,      min.z };
    pin[1] = { min.x+sz.x, min.y,      min.z };
    pin[2] = { min.x,      min.y+sz.y, min.z };
    pin[3] = { min.x,      min.y,      min.z+sz.z };

    pin[4] = { max.x,      max.y,      max.z };
    pin[5] = { max.x-sz.x, max.y,      max.z };
    pin[6] = { max.x,      max.y-sz.y, max.z };
    pin[7] = { max.x,      max.y,      max.z-sz.z };

    // transform points
    MatrixMulVec3(pin[0], *m, pout[0]);
    MatrixMulVec3(pin[1], *m, pout[1]);
    MatrixMulVec3(pin[2], *m, pout[2]);
    MatrixMulVec3(pin[3], *m, pout[3]);

    MatrixMulVec3(pin[4], *m, pout[4]);
    MatrixMulVec3(pin[5], *m, pout[5]);
    MatrixMulVec3(pin[6], *m, pout[6]);
    MatrixMulVec3(pin[7], *m, pout[7]);

    // by transformed points we calc a new AABB (already in world space)
    BuildAABB(pout, 8);
}
