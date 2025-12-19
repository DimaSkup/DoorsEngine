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
    float posX          = 0.0f;;
    float posY          = 0.0f;;
    float posZ          = 0.0f;;

    float fov           = 0.0f;
    float aspectRatio   = 0.0f;
    float nearZ         = 0.0f;
    float farZ          = 0.0f;

    // view and projection matrix
    float view[16]{ 0 };
    float proj[16]{ 0 };
};
