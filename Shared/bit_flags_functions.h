/***************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: bit_flags_functions.h
    Desc:     implementation of functional for simple bit
              and flag collections handling

              This code I mainly rewrote from sources to the amazing book
              "Real Time 3D Terrain Engines Using C++ and DirectX"
              by Greg Snook

    Created:  15.09.2025 by DimaSkup
\***************************************************************/
#pragma once
#include "bit_flags.h"

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
