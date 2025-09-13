//==================================================================================
// Filename:   math_helpers.h
// Desc:       different math stuff
//
// Created:    06.07.2025  by DimaSkup
//==================================================================================
#pragma once

//--------------------------------------------------------------
// Constants
//--------------------------------------------------------------
constexpr float PI          = 3.1415926535897932384626433832795f;
constexpr float M_2PI       = 6.283185307f;
constexpr float M_1DIVPI    = 0.318309886f;
constexpr float M_1DIV2PI   = 0.159154943f;
constexpr float PIDIV2      = 1.570796327f;
constexpr float PIDIV4      = 0.785398163f;

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

// return random unsigned int in range [min, max)
inline unsigned int RandUint(const unsigned int min, const unsigned int max)
{
    return min + (rand()) % (max - min);
}

// ----------------------------------------------------
inline static float RandF()
{
    // returns random float in [0, 1)
    return (float)rand() / (float)RAND_MAX;
}

// ----------------------------------------------------
inline static float RandF(const float a, const float b)
{
    // returns random float in [a, b)
    return a + RandF() * (b - a);
}
// ----------------------------------------------------
template<typename T>
static T Clamp(const T& x, const T& low, const T& high)
{
    return x < low ? low : (x > high ? high : x);
}

// ----------------------------------------------------
inline void ClampPitch(float& pitch)
{
    // limit the pitch value in range (-(PI/2)+0.1f < pitch < (PI/2)-0.1f)
    if (pitch > PIDIV2 - 0.1f)
    {
        pitch = PIDIV2 - 0.1f;
    }
    else if (pitch < -PIDIV2 + 0.1f)
    {
        pitch = -PIDIV2 + 0.1f;
    }
}

// ----------------------------------------------------
inline void ClampYaw(float& yaw)
{
    // limit the yaw value in range (-2PI < yaw < 2PI)
    if (yaw > M_2PI)
    {
        yaw = -M_2PI;
    }
    else if (yaw < -M_2PI)
    {
        yaw = M_2PI;
    }
}

// ----------------------------------------------------
inline int powi(const int value, const int power)
{
    return (int)exp(log(value) * power);
}

