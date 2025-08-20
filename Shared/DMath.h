//==================================================================================
// Filename:   DMath.h
// Desc:       different math stuff
//
// Created:    06.07.2025  by DimaSkup
//==================================================================================
#pragma once

#include <math.h>

//--------------------------------------------------------------
// Constants
//--------------------------------------------------------------
#define PI		   3.1415926535897932384626433832795f
#define PIOVER180  0.0174532925199432957692369076848861f
#define PIUNDER180 57.2957795130823208767981548141052f
#define EPSILON	   1.0e-8
#define SMALL	   1.0e-4f
#define BIG		   1.0e+10f


//---------------------------------------------------------
// Angles convertion
//---------------------------------------------------------
#define DEG_TO_RAD( angle )	  ( ( angle )*PIOVER180 )
#define RAD_TO_DEG( radians ) ( ( radians )*PIUNDER180 )


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
// Desc:   helpers to get a square/cube of input value
//---------------------------------------
inline int   SQR(const int num)     { return num*num; }
inline float SQR(const float num)   { return num*num; }
inline int   CUBE(const int num)    { return num*num*num; }
inline float CUBE(const float num)  { return num*num*num; }


//---------------------------------------------------------
// Desc:   primitive vector of 3 floats
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
// Desc:   primitive vector of 4 floats
//---------------------------------------------------------
struct Vec4
{
    Vec4() :
        x{0}, y{0}, z{0}, w{0} {}

    Vec4(const float _x, const float _y, const float _z, const float _w) :
        x{_x}, y{_y}, z{_z}, w{_w} {}

    inline Vec4 operator+(const Vec4& v) const
    {
        return { x+v.x, y+v.y, z+v.z, w+v.w };
    }

    inline Vec4 operator-(const Vec4& v) const
    {
        return { x-v.x, y-v.y, z-v.z, w-v.w };
    }

    inline float operator[](const int n)  const { return v[n]; }
    inline float& operator[](const int n)       { return v[n]; }

    union
    {
        float v[4];

        struct
        {
            float x, y, z, w;
        };
    };
    

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
