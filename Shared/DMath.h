//==================================================================================
// Filename:   DMath.h
// Desc:       different math stuff
//
// Created:    06.07.2025  by DimaSkup
//==================================================================================
#pragma once

#include <math.h>

// --------------------------------------------------------
// Desc:   check if input value is power of 2
// Args:   - value: a value to check
// Ret:    - true if is power of 2
// --------------------------------------------------------
inline bool IsPow2(const int value)
{
    return (value && !(value & (value - 1)));
}

//---------------------------------------
// swapping values with XOR (NOTE: works only for int values)
//---------------------------------------
#define SWAP(a, b) ((a ^= b), (b ^= a), (a ^= b))

//---------------------------------------
// Desc:   helpers to get a square of input value
//---------------------------------------
inline int   SQR(const int number) { return number * number; }
inline float SQR(const float number) { return number * number; }

//---------------------------------------------------------
// Desc:   primitive Vec3
//---------------------------------------------------------
struct Vec3
{
    inline float Length() const
    {
        return sqrtf(x * x + y * y + z * z);
    }

    Vec3& operator=(const Vec3& v)
    {
        x = v.x;
        y = v.y;
        z = v.z;

        return *this;
    }

    float x = 0;
    float y = 0;
    float z = 0;
};

//---------------------------------------------------------
// Desc:   compute the dot product for two input 3D vectors
// Args:   - v1, v2: vectors in 3D space
// Ret:    - float value: the dot product
//---------------------------------------------------------
inline float Dot(const Vec3& v1, const Vec3& v2)
{
    return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}
