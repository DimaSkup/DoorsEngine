/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: vec2.h
    Desc:     vector of 2 floats

    Created:  28.11.2025 by DimaSkup
\**********************************************************************************/
#pragma once
#include <math/math_helpers.h>


struct Vec2
{
public:

    //-----------------------------------------------------
    // public data
    //-----------------------------------------------------
    union
    {
        float xy[2];

        struct
        {
            float x, y;
        };

        struct
        {
            float u, v;
        };
    };


    //-----------------------------------------------------
    // constructors
    //-----------------------------------------------------
    Vec2() {}

    Vec2(const float x_, const float y_) :
        x(x_), y(y_) {}

    Vec2(const Vec2& v) :
        x(v.x), y(v.y) {}

    Vec2(const float* arr) :
        x(arr[0]), y(arr[1]) {}

    //-----------------------------------------------------
    // operators
    //-----------------------------------------------------

    // assignment
    inline Vec2& operator = (const Vec2& v)
    {
        x = v.x;
        y = v.y;
        return *this;
    }

    inline Vec2& operator = (const Vec2&& v) noexcept
    {
        x = v.x;
        y = v.y;
        return *this;
    }

    inline Vec2& operator += (const Vec2& v)
    {
        x += v.x;
        y += v.y;

        return *this;
    }

    inline Vec2& operator -= (const Vec2& v)
    {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    inline Vec2& operator *= (const float s)
    {
        x *= s;
        y *= s;
        return *this;
    }

    inline Vec2& operator /= (const float s)
    {
        // prevent division by zero
        const float inv = (s == 0.0f) ? 1.0f : 1.0f / s;

        x *= inv;
        y *= inv;
    }

    // comparison
    inline bool operator == (const Vec2& v) const
    {
        return FloatEqual(x, v.x) && FloatEqual(y, v.y);
    }

    inline bool operator != (const Vec2& v) const
    {
        return !(*this == v);
    }

    // negation
    inline Vec2 operator - () const
    {
        return Vec2(-x, -y);
    }
};
