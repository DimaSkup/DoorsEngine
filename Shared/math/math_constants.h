/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: math_constants.h
    Desc:     contains definitions of different math constants

    Created:  25.09.2025  by DimaSkup
\**********************************************************************************/
#pragma once

// definition of small numbers
#define EPSILON_E4 (float)(1E-4)
#define EPSILON_E5 (float)(1E-5)
#define EPSILON_E6 (float)(1E-6)

//--------------------------------------------------------------
// Constants
//--------------------------------------------------------------
constexpr float PI = 3.1415926535897932384626433832795f;
constexpr float M_2PI = 6.283185307f;
constexpr float M_1DIVPI = 0.318309886f;
constexpr float M_1DIV2PI = 0.159154943f;
constexpr float PIDIV2 = 1.570796327f;
constexpr float PIDIV4 = 0.785398163f;

#define PIOVER180  0.0174532925199432957692369076848861f
#define PIUNDER180 57.2957795130823208767981548141052f
#define EPSILON	   1.0e-8
#define SMALL	   1.0e-4f
#define BIG		   1.0e+10f
