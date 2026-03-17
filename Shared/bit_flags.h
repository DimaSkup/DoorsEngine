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

#include "types.h"

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
    T    Get() const;
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



//==================================================================================
// constructors / destructors
//==================================================================================
template <class T>
inline BitFlags<T>::BitFlags() : value_(0)
{
}

//---------------------------------------------------------

template <class T>
inline BitFlags<T>::BitFlags(const T settings) : value_(settings)
{
}

//---------------------------------------------------------

template <class T>
inline BitFlags<T>::BitFlags(const BitFlags& src) : value_(src.value_)
{
}

//---------------------------------------------------------

template <class T>
inline BitFlags<T>::~BitFlags()
{
}


//==================================================================================
// operators
//==================================================================================
template <class T>
inline BitFlags<T>& BitFlags<T>::operator = (const BitFlags<T>& src)
{
    value_ = src.value_;
    return (*this);
}

//---------------------------------------------------------

template <class T>
inline BitFlags<T>& BitFlags<T>::operator = (const T src)
{
    value_ = src;
    return (*this);
}

//---------------------------------------------------------

template <class T>
bool BitFlags<T>::operator == (const BitFlags& src) const
{
    return (value_ == src.value_);
}

//---------------------------------------------------------

template <class T>
bool BitFlags<T>::operator != (const BitFlags& src) const
{
    return (value_ != src.value_);
}

//==================================================================================
// mutators / setters
//==================================================================================
template <class T>
inline void BitFlags<T>::Set(const T settings)
{
    value_ = settings;
}

//---------------------------------------------------------

template <class T>
inline void BitFlags<T>::Clear()
{
    value_ = 0;
}

//---------------------------------------------------------

template <class T>
inline void BitFlags<T>::SetFlags(const T settings)
{
    value_ |= settings;
}

//---------------------------------------------------------

template <class T>
inline void BitFlags<T>::ClearFlags(const T settings)
{
    value_ &= ~settings;
}

//---------------------------------------------------------

template <class T>
inline void BitFlags<T>::SetBit(const int bit)
{
    value_ |= (1 << bit);
}

//---------------------------------------------------------

template <class T>
inline void BitFlags<T>::SetBit(const int bit, const bool state)
{
    if (state)
    {
        value_ |= (1 << bit);
    }
    else
    {
        ClearBit(bit);
    }
}

//---------------------------------------------------------

template <class T>
inline void BitFlags<T>::ClearBit(const int bit)
{
    value_ &= (~(1<<bit));
}

//==================================================================================
// accessors / getters
//==================================================================================
template <class T>
inline T BitFlags<T>::Get() const
{
    return value_;
}

template <class T>
inline bool BitFlags<T>::IsEmpty() const
{
    return value_ == 0;
}

//---------------------------------------------------------

template <class T>
inline bool BitFlags<T>::TestBit(const int bit) const
{
    return (value_ & (1<<bit));
}

//---------------------------------------------------------

template <class T>
inline bool BitFlags<T>::TestFlags(const T test) const
{
    return ((value_ & test) == test);
}

//---------------------------------------------------------

template <class T>
inline bool BitFlags<T>::TestAny(const T test) const
{
    return (value_ & test);
}

//---------------------------------------------------------

template <class T>
inline int BitFlags<T>::TotalBits() const
{
    return (sizeof(T) << 3);
}

//---------------------------------------------------------
// counting bits set, Brian Kernighan's way
//---------------------------------------------------------
template <class T>
inline int BitFlags<T>::TotalSet() const
{
    int c = 0;           // c accumulates the total bits set in v
    T   v = value_;

    for (c = 0; v; c++)
    {
        v &= (v - 1);    // clear the least significant bit set
    }

    return c;
}

