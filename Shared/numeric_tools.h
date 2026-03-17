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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


//---------------------------------------------------------
// floating point macros
//---------------------------------------------------------

// reinterpret a float as an int
#define fpBits(f) (*(const int*)(&f))

// return 0 or -1 based on the sign of the float
#define fpSign(f) (fpBits(f)>>31)

// extract the 8 bits of exponent as a signed integer
// by masking out this bits, shifting down by 23
// and subtracting the bias values of 127
#define fpExponent(f) (((fpBits(f)&0x7FFFFFFF)>>23)-127)

// return 0 or -1 based on the sign of the exponent
#define fpExponentSign(f) (fpExponent(f)>>31)

// get the 23 bits of mantissa without the implied bit
#define fpPureMantissa(f) ((fpBits(f)&0x7FFFFF))

// get the 23 bits of mantissa with the implied bit replaced
#define fpMantissa(f) (fpPureMantissa(f) | (1<<23))

// invert the sign of i if flip equals -1,
// if flip equals 0, it does nothing
#define flipSign(i, flip) ((i ^ flip) - flip)


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

//---------------------------------------------------------
// convert a float values to int32, all fractional values are truncated
// RealToInt32_chop(2.35) = 2;  RealToInt32_chop(-2.35) = -2
//---------------------------------------------------------
inline int RealToInt32_chop(float input)
{
    // read the exponent and decide how much we need to shift the mantissa down
    int shift = 23 - fpExponent(input);

    // read the matrissa and shift it down to remove all the fractional values
    int result = fpMantissa(input) >> shift;

    // set the sign of the new result
    result = flipSign(result, fpSign(input));

    // if the exponent was negative, (-1<input<1) we must return zero
    result &= ~fpExponentSign(input);

    // return the result
    return result;
}


//---------------------------------------------------------
// convert a float value to the next-lowest int32 value
// RealToInt32_floor(2.35) = 2;  RealToInt32_floor(-2.35) = -3
//---------------------------------------------------------
inline int RealToInt32_floor(float input)
{
    // read the exponent and decide how much we need to shift the mantissa down
    int shift = 23 - fpExponent(input);

    // read the mantissa and shift it down to remove all fractional values
    int result = fpMantissa(input) >> shift;

    // set the sign of the new result
    result = flipSign(result, fpSign(input));

    // if the exponent was negative, (-1<input<1) we must return zero
    result &= ~fpExponentSign(input);

    // if the original values is negative, and any fractional values are present,
    // decrement the result by one
    result -= fpSign(input) &&
              (fpExponentSign(input) || (fpPureMantissa(input) & ((1 << shift) - 1)));

    // return the result
    return result;
}
