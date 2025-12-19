/***************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: bit_flags.h
    Desc:     functional for simple bit and flag collections handling

              This code mainly I rewrote from sources to the amazing book
              "Real Time 3D Terrain Engines Using C++ and DirectX"
              by Greg Snook

    Created:  15.09.2025 by DimaSkup
\***************************************************************/
#pragma once

#include "Types.h"

// disable the MSVC warning regarding 
// forcing values to true or false
#pragma warning( disable : 4800 )


//---------------------------------------------------------
// Macros for bit and flag collection testing
//---------------------------------------------------------
#define FLAG(x) (1 << x)
constexpr bool TEST_BIT(const uint32 flag, const uint32 bit)  { return ((flag & FLAG(bit)) != 0); }
#define TEST_ALL (flag, set)  ((flag.value & set) == set)
#define TEST_ANY (flag, set)  ((flag.value & set) != 0)
#define SET_BIT  (flag, bit)  (flag.value |= FLAG(bit))
#define CLEAR_BIT(flag, bit)  (flag.value &= ~FLAG(bit))
#define SET_ALL  (flag, set)  (flag.value |= set)
#define CLEAR_ALL(flag, set)  (flag.value &= ~(set))


//---------------------------------------------------------
// Class: BitFlags
//---------------------------------------------------------
template <class T>
class BitFlags
{
public:

    T value_;

    // creators
    BitFlags();
    BitFlags(const T settings);
    BitFlags(const BitFlags& src);
    ~BitFlags();

    // operators
    BitFlags& operator = (const BitFlags& src);
    BitFlags& operator = (const T src);

    operator T() const { return (value_); }
    bool operator == (const BitFlags& src) const;
    bool operator != (const BitFlags& src) const;

    // mutators/setters
    void Set(const T settings);
    void Clear(void);
    void SetFlags(const T settings);
    void ClearFlags(const T settings);
    void SetBit(const int bit);
    void ClearBit(const int bit);
    void SetBit(const int bit, const bool state);

    // accessors/getters
    bool IsEmpty() const;
    bool TestBit(const int bit) const;
    bool TestFlags(const T test) const;
    bool TestAny(const T test) const;

    int TotalBits() const;
    int TotalSet() const;
};

// common flag typedefs
using u8Flags  = BitFlags<uint8>;
using u16Flags = BitFlags<uint16>;
using u32Flags = BitFlags<uint32>;

