/***************************************************************\

    ******    ********  ********  ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******    ********  ********  **    **  ********

    Filename: rect_3d.h
    Desc:     3d rectangle implementation
    Created:  13.09.2025 by DimaSkup
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
        struct Rect2d rect2d;
        float z0, z1;
    };

    //-----------------------------------------------------
    // creators
    //-----------------------------------------------------
    Rect3d() {};
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

};
