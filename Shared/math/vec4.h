/***************************************************************\

    ******    ********  ********  ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******    ********  ********  **    **  ********

    Filename: vec3.h
    Desc:     vector of 4 floats 
    Created:  13.09.2025 by DimaSkup
\***************************************************************/
#pragma once


struct Vec4
{
    Vec4() :
        x{ 0 }, y{ 0 }, z{ 0 }, w{ 0 } {}

    Vec4(const float _x, const float _y, const float _z, const float _w) :
        x{ _x }, y{ _y }, z{ _z }, w{ _w } {}



    inline Vec4 operator+(const Vec4& v) const
    {
        return { x + v.x, y + v.y, z + v.z, w + v.w };
    }

    inline Vec4 operator-(const Vec4& v) const
    {
        return { x - v.x, y - v.y, z - v.z, w - v.w };
    }

    inline float operator[](const int n)  const { return xyzw[n]; }
    inline float& operator[](const int n)       { return xyzw[n]; }

    union
    {
        float xyzw[4];

        struct
        {
            float x, y, z, w;
        };
    };


};
