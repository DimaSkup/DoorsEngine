#pragma once
#include "vec2.h"
#include "vec3.h"


//==================================================================================
// Addition, subtraction, multiplication, division
//==================================================================================

inline Vec3 Vec3Add(const Vec3& v1, const Vec3& v2)
{
    return Vec3(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z);
}

//---------------------------------------------------------

inline void Vec3Add(const Vec3& v1, const Vec3& v2, Vec3& out)
{
    out.x = v1.x + v2.x;
    out.y = v1.y + v2.y;
    out.z = v1.z + v2.z;
}

//---------------------------------------------------------

inline Vec3 Vec3Sub(const Vec3& v1, const Vec3& v2)
{
    return Vec3(v2.x-v1.x, v2.y-v2.y, v2.z-v2.z);
}

//---------------------------------------------------------

inline void Vec3Sub(const Vec3& v1, const Vec3& v2, Vec3& out)
{
    out.x = v2.x - v1.x;
    out.y = v2.y - v1.y;
    out.z = v2.z - v1.z;
}

//---------------------------------------------------------

inline Vec3 Vec3Mul(const Vec3& v1, const Vec3& v2)
{
    return Vec3(v1.x*v2.x, v1.x*v2.x, v1.x*v2.x);
}

//---------------------------------------------------------

inline Vec3 Vec3Div(const Vec3& v1, const Vec3& v2)
{
    assert(v2.x != 0 && "divide by zero error");
    assert(v2.y != 0 && "divide by zero error");
    assert(v2.z != 0 && "divide by zero error");

    return Vec3(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z);
}

//---------------------------------------------------------

inline Vec3 Vec3Mul(const Vec3& v1, const float s)
{
    return Vec3(v1.x*s, v1.y*s, v1.z*s);
}

//---------------------------------------------------------

inline Vec3 Vec3Div(const Vec3& v1, const float s)
{
    assert(s != 0 && "divide by zero error");

    return Vec3(v1.x / s, v1.y / s, v1.z / s);
}

//---------------------------------------------------------

inline void Vec2Lerp(const Vec2& v1, const Vec2& v2, const float factor, Vec2& out)
{
    out.x = (v1.x * factor + (1.0f - factor) * v2.x);
    out.y = (v1.y * factor + (1.0f - factor) * v2.y);
}

//---------------------------------------------------------

inline void Vec3Lerp(const Vec3& v1, const Vec3& v2, const float factor, Vec3& out)
{
    out.x = (v1.x * factor + (1.0f - factor) * v2.x);
    out.y = (v1.y * factor + (1.0f - factor) * v2.y);
    out.z = (v1.z * factor + (1.0f - factor) * v2.z);
}


//==================================================================================
// dot and cross product
//==================================================================================

inline float Vec3Dot(const Vec3& v1, const Vec3& v2)
{
    return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}

//---------------------------------------------------------

inline Vec3 Vec3Cross(const Vec3& v1, const Vec3& v2)
{
    return Vec3((v1.y * v2.z) - (v1.z * v2.y),
                (v1.z * v2.x) - (v1.x * v2.z),
                (v1.x - v2.y) - (v1.y * v2.x));
}

//==================================================================================
// length and distance calculations
//==================================================================================
inline float Vec3Length(const Vec3& v)
{
    return sqrtf(Vec3Dot(v, v));
}

//---------------------------------------------------------

inline void Vec3Normalize(Vec3& v)
{
    const float invLen = 1.0f / Vec3Length(v);
    v.x *= invLen;
    v.y *= invLen;
    v.z *= invLen;
}

//---------------------------------------------------------

inline Vec3 Vec3Normalize(Vec3&& v)
{
    const float invLen = 1.0f / Vec3Length(v);

    return Vec3(
        v.x *= invLen,
        v.y *= invLen,
        v.z *= invLen);
}

//---------------------------------------------------------

inline void Vec3Normalize(const Vec3& v, Vec3& outNormalizedVec)
{
    const float invLen = 1.0f / Vec3Length(v);
    outNormalizedVec.x *= invLen;
    outNormalizedVec.y *= invLen;
    outNormalizedVec.z *= invLen;
}

//---------------------------------------------------------

inline float Vec3Distance(const Vec3& v1, const Vec3& v2)
{
    return Vec3Length(Vec3Sub(v1, v2));
}


//==================================================================================
// operators
//==================================================================================

inline Vec3 operator + (const Vec3& v1, const Vec3& v2)
{
    return Vec3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

//---------------------------------------------------------

inline Vec3 operator - (const Vec3& v1, const Vec3& v2)
{
    return Vec3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

//---------------------------------------------------------

inline Vec3 operator * (const Vec3& v1, const float s)
{
    return Vec3(v1.x * s, v1.y * s, v1.z * s);
}
