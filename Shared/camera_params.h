/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: camera_params.h
    Desc:     just container for different camera's params 

    Created:  20.09.2025 by DimaSkup
\**********************************************************************************/
#pragma once

struct CameraParams
{
    // position in world
    float posX;
    float posY;
    float posZ;

    // camera params
    float fov;
    float aspect;
    float zn;       // dist to near clipping plane (by Z-axis)
    float zf;       // dist to far clipping plane (by Z-axis)

    // view, inverse view, and projection matrix
    float view[16];
    float invView[16];
    float proj[16];
};
