/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: vec3.h
    Desc:     vector of 3 floats

    Created:  13.09.2025 by DimaSkup
\**********************************************************************************/
#pragma once
#include <math/math_helpers.h>


struct Vec3
{
public:

    //-----------------------------------------------------
    // public data
    //-----------------------------------------------------
    union
    {
        float xyz[3];

        struct
        {
            float x, y, z;
        };
        struct
        {
            float r, g, b;
        };
    };


    //-----------------------------------------------------
    // constructors
    //-----------------------------------------------------
    Vec3() {}

    Vec3(const float x_, const float y_, const float z_) :
        x(x_), y(y_), z(z_) {}

    Vec3(const Vec3& v) :
        x(v.x), y(v.y), z(v.z) {}

    Vec3(const float* arr) :
        x(arr[0]), y(arr[1]), z(arr[2]) {}

    //-----------------------------------------------------
    // operators
    //-----------------------------------------------------

    // assignment
    inline Vec3& operator = (const Vec3& v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
        return *this;
    }

    inline Vec3& operator = (const Vec3&& v) noexcept
    {
        x = v.x;
        y = v.y;
        z = v.z;
        return *this;
    }

    inline Vec3& operator += (const Vec3& v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    inline Vec3& operator -= (const Vec3& v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    inline Vec3& operator *= (const float s)
    {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }

    inline Vec3& operator /= (const float s)
    {
        // prevent division by zero
        const float inv = (s == 0.0f) ? 1.0f : 1.0f/s;

        x *= inv;
        y *= inv;
        z *= inv;
    }

    // comparison
    inline bool operator == (const Vec3& v) const
    {
        return FloatEqual(x, v.x) && FloatEqual(y, v.y) && FloatEqual(z, v.z);
    }

    // negation
    inline Vec3 operator - () const
    {
        return Vec3(-x, -y, -z);
    }
};
