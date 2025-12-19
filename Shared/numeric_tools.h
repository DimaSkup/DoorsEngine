/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: numeric_tools.h
    Desc:     a collection of helper functions dealing with numeric data.

              This code I mainly rewrote from sources to the amazing book
              "Real Time 3D Terrain Engines Using C++ and DirectX"
              by Greg Snook

    Created:  18.09.2025 by DimaSkup
\**********************************************************************************/
#pragma once

#include <assert.h>
#include <intrin.h>

//---------------------------------------------------------
// Desc:   returns the index of the highest bit st in the input value
//---------------------------------------------------------
template <class T>
inline int HighestBitSet(const T input)
{
#if 0
    int result;
    assert(input);           // zero is invalid input!
    assert(sizeof(T) == 4);  // 32bit data only!
    _asm bsr eax, input
    _asm mov result, eax
    return result;
#endif

    DWORD index;
    DWORD mask = (DWORD)input;
    _BitScanReverse(&index, input);
    return (int)index;
}
