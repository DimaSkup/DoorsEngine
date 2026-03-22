//==================================================================================
// Filename:   math_helpers.h
// Desc:       different math helpers
//
// Created:    06.07.2025  by DimaSkup
//==================================================================================
#pragma once

#include "math_constants.h"
#include <math.h>

//---------------------------------------------------------
// Angles convertion
//---------------------------------------------------------
#define DEG_TO_RAD( angle )	  ( ( angle )  *PIOVER180 )
#define RAD_TO_DEG( radians ) ( ( radians )*PIUNDER180 )

// --------------------------------------------------------
// Desc:   check if input value is power of 2
// Args:   - value: a value to check
// Ret:    - true if is power of 2
// --------------------------------------------------------
#define IS_POW2(x) (((x) != 0) && !((x) & ((x) - 1)))

//---------------------------------------
// swapping values with XOR (NOTE: works only for INTEGER values)
//---------------------------------------
#define SWAP(a, b) ((a ^= b), (b ^= a), (a ^= b))

//---------------------------------------
// Desc:   helpers to get a square/cube of input value
//---------------------------------------
#define SQR(n)  ((n)*(n))
#define CUBE(n) ((n)*(n)*(n))

//-----------------------------------------------------

float FastDistance3D(const float x, const float y, const float z);

//-----------------------------------------------------

// only for integers
inline int FastClamp(int x, int lo, int hi)
{
    x = x + ((lo - x) & ((lo - x) >> 31));
    x = x - ((x - hi) & ((x - hi) >> 31));
    return x;
}

// ----------------------------------------------------

inline static float clampf(const float x, const float low, const float high)
{
    return x < low ? low : (x > high ? high : x);
}

// ----------------------------------------------------
template<typename T>
inline static T Clamp(const T& x, const T& low, const T& high)
{
    return x < low ? low : (x > high ? high : x);
}

//---------------------------------------------------------
// clamp pitch / yaw
//---------------------------------------------------------
inline float ClampPitch(const float pitch)
{
    if (pitch > PIDIV2 - 0.1f)   return PIDIV2 - 0.1f;
    if (pitch < -PIDIV2 + 0.1f)  return -PIDIV2 + 0.1f;

    return pitch;
}

//---------------------------------------------------------

inline float ClampYaw(const float yaw)
{
    if (yaw > M_2PI)   return -M_2PI;
    if (yaw < -M_2PI)  return M_2PI;

    return yaw;
}

// ----------------------------------------------------

inline int powi(const int value, const int power)
{
    return (int)exp(log(value) * power);
}

// ----------------------------------------------------

inline float lerp(const float a, const float b, const float factor)
{
    return (a * factor + (1.0f - factor) * b);
}

// ----------------------------------------------------

inline int FastMin(const int x, const int y)
{
    return y ^ ((x ^ y) & -(x < y)); // min(x, y)
}

// ----------------------------------------------------

inline int FastMax(const int x, const int y)
{
    return x ^ ((x ^ y) & -(x < y)); // max(x, y)
}

// ----------------------------------------------------

template <class T>
inline T Min(const T x, const T y)
{
    return (x < y) ? x : y;
}

// ----------------------------------------------------

template <class T>
inline T Max(const T x, const T y)
{
    return (x > y) ? x : y;
}

//---------------------------------------------------------
// a little helper to check if 2 input floats are "equal"
//---------------------------------------------------------
inline bool FloatEqual(const float a, const float b)
{
    return fabsf(a - b) < EPSILON_E5;
}
