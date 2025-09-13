/***************************************************************\

    ******    ********  ********  ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******    ********  ********  **    **  ********

    Filename: vec3.h
    Desc:     vector of 3 floats
    Created:  13.09.2025 by DimaSkup
\***************************************************************/
#pragma once


struct Vec3
{
public:
    Vec3() :
        x(0), y(0), z(0) {}

    Vec3(const float x_, const float y_, const float z_) :
        x(x_), y(y_), z(z_) {}


    Vec3& operator=(const Vec3& v);


    union
    {
        float xyz[3]{ 0 };

        struct
        {
            float x, y, z;
        };
    };
};
