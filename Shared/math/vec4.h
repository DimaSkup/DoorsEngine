/***************************************************************\

    ******    ********  ********  ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******    ********  ********  **    **  ********

    Filename: vec4.h
    Desc:     vector of 4 floats 
    Created:  13.09.2025 by DimaSkup
\***************************************************************/
#pragma once

#include <math/math_helpers.h>

struct Vec4
{
    //-----------------------------------------------------
    // public data
    //-----------------------------------------------------
    union
    {
        float xyzw[4];

        float m[4];

        struct
        {
            float x, y, z, w;
        };

        struct
        {
            float r, g, b, a;
        };
    };

    //-----------------------------------------------------
    // constructors
    //-----------------------------------------------------
    Vec4() :
        x{ 0 }, y{ 0 }, z{ 0 }, w{ 0 } {}

    Vec4(const float _x, const float _y, const float _z, const float _w) :
        x{ _x }, y{ _y }, z{ _z }, w{ _w } {}

    Vec4(const float* arr) :
        x(arr[0]), y(arr[1]), z(arr[2]), w(arr[3]) {}

    Vec4(const Vec4& v) :
        x(v.x), y(v.y), z(v.z), w(v.w) {}

    Vec4(Vec4&& v) noexcept :
        x(v.x), y(v.y), z(v.z), w(v.w) {}
    
        

    //-----------------------------------------------------
    // operators
    //-----------------------------------------------------

    inline Vec4& operator = (const Vec4& v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
        w = v.w;

        return *this;
    }

    inline Vec4& operator = (Vec4&& v) noexcept
    {
        x = v.x;
        y = v.y;
        z = v.z;
        w = v.w;

        return *this;
    }

    inline bool operator == (const Vec4& v) const
    {
        return FloatEqual(x, v.x) && FloatEqual(y, v.y) && FloatEqual(z, v.z) && FloatEqual(w, v.w);
    }

    inline Vec4 operator+(const Vec4& v) const
    {
        return { x + v.x, y + v.y, z + v.z, w + v.w };
    }

    inline Vec4 operator-(const Vec4& v) const
    {
        return { x - v.x, y - v.y, z - v.z, w - v.w };
    }

    inline float  operator[](const int n) const { return xyzw[n]; }
    inline float& operator[](const int n)       { return xyzw[n]; }

};
