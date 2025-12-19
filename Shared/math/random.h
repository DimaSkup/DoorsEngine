/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: random.h
    Desc:     helpers for generation of random values

    Created:  16.09.2025 by DimaSkup
\**********************************************************************************/
#pragma once

#include <stdlib.h>

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
