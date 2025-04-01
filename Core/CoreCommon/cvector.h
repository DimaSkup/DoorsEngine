// =================================================================================
// Filename:     cvector.h
// Description:  header-only custom reimplementation of std::vector;
//               and added some specific functional as well
// 
// Created:      12.12.2024  by DimaSkup
// =================================================================================
#pragma once

#include <algorithm>
#include <utility>
#include <stdarg.h>

namespace Core
{

// this macro is used for the vassert() method
#define CALLER_INFO "  FILE: \t%s\n  FUNC: \t%s()\n  LINE: \t%d\n  MSG: \t\t%s\n", __FILE__, __func__, __LINE__


constexpr bool ENABLE_CHECK = true;


// some typedefas
using index = ptrdiff_t;
using vsize = ptrdiff_t;
static float growFactor_ = 1.5f;


// =================================================================================
// CVECTOR
// =================================================================================
template<typename T>
class cvector
{
private:
    T* data_ = nullptr;
    vsize size_ = 0;
    vsize capacity_ = 0;

public:
    cvector();
    cvector(const vsize count, const T& value = T());

    cvector(const cvector<T>& other);
    cvector(cvector<T>&& other) noexcept;

    cvector(std::initializer_list<T> il);

    template<typename Iter>
    inline cvector(const Iter* first, const Iter* last) { assign(first, last); }

    ~cvector();


    // operators
    inline       T& operator[](index i) { return data_[i]; }    // v[i] = x
    inline const T& operator[](index i) const { return data_[i]; }    // x = v[i]

    bool        operator==(const cvector<T>& rhs) const;
    cvector<T>& operator=(const cvector<T>& rhs);
    cvector<T>& operator=(cvector<T>&& rhs) noexcept;
    cvector<T>& operator=(std::initializer_list<T> list);


    // iterators
    inline T*       begin()                       { return data_; }
    inline const T* begin()                 const { return data_; }
    inline T*       end()                         { return data_ + size_; }
    inline const T* end()                   const { return data_ + size_; }
    inline       T& back()                  const { return *(data_ + size_ - 1); }


    // getters
    inline T*       data()                  const { return data_; }
    inline bool     empty()                 const { return size_ == 0; }
    inline vsize    size()                  const { return size_; }
    inline vsize    capacity()              const { return capacity_; }
    inline bool     is_valid_index(index i) const { return (i >= 0) && (i < size_); };

    void get_data_by_idxs(const cvector<index>& idxs, cvector<T>& outData) const;
    void get_data_by_idxs(const cvector<index>& idxs, T* outData) const;

    // setters
    void         push_back(const T& value);
    void         push_back(T&& rvalue);
    inline void  pop_back() { if (size_ > 0) size_--; }
    inline void  clear() { size_ = 0; };

    void         shrink_to_fit();
    void         purge();
    void         erase(const vsize index);
    inline void  assign(std::initializer_list<T> il) { assign(il.begin(), il.end()); };

    // get index(or indices) for sorted insertion / insertion
    index get_insert_idx(const ptrdiff_t& value) const;
    void  get_insert_idxs(const cvector<T>& values, cvector<index>& idxs) const;
    void  get_insert_idxs(const T* values, const vsize numValues, cvector<index>& idxs) const;
    void  insert_before(const vsize idx, const T& val);
    void  insert_before(const vsize idx, T&& value);

    template <typename U>
    void append_vector(U&& src);

    template<typename Iter>
    void assign(Iter first, Iter last);


    // shift elements
    void shift_right(const index idx, const int num);
    void shift_left(const index idx, const int num);


    // search
    index find(const T& value) const;
    index get_idx(const T& value) const;
    void get_idxs(const T* values, const vsize numElems, cvector<index>& outIdxs) const;
    void get_idxs(const cvector<T>& values, cvector<index>& outIdxs) const;

    bool has_value(const T& val) const;
    bool binary_search(const T& value) const;
    bool binary_search(const cvector<T>& vSrc) const;
    bool binary_search(const T* values, const vsize numElems) const;
    void binary_search(const T* values, vsize numElems, cvector<bool>& flags) const;


    // allocators
    void reserve(const vsize newCapacity);
    void resize(const vsize newSize);
    void resize(const vsize newSize, const T& value);

private:
    void vassert(
        const bool condition,
        const char* msg,
        const char* format,
        const char* fileName,
        const char* className,
        const char* funcName,
        const int line) const;

    void error_msg(
        const char* msg,
        const char* format,
        const char* fileName,
        const char* funcName,
        const int line) const;


    void realloc_buffer_discard(const vsize newCapacity);
    void realloc_buffer(const vsize newCapacity);

    inline void safe_delete() { if (data_) { delete[] data_; data_ = nullptr; } }

    inline vsize GetGrownCapacity(const vsize capacity)
    {
        // compute grown capacity: newCapacity = growFactor * capacity;
        return (vsize)(ceil(growFactor_ * (float)capacity));
    }
};

// =================================================================================

template <typename T>
void cvector<T>::error_msg(
    const char* msg,
    const char* format,
    const char* fileName,
    const char* funcName,
    const int line) const
{
    const char* consoleRed = "\x1B[31m";
    const char* consoleNorm = "\x1B[0m";

    printf("%s\nERROR:\n", consoleRed);
    printf(format, fileName, funcName, line, msg);
    printf("%s", consoleNorm);
}


// =================================================================================
//                          constructor, destructor
// =================================================================================
template <typename T>
inline cvector<T>::cvector() :
    size_(0),
    capacity_(0),
    data_(nullptr)
{
}

// ----------------------------------------------------

template <typename T>
inline cvector<T>::cvector(const vsize count, const T& value) :
    size_(count),
    capacity_(count)
{
    data_ = new T[capacity_];                               // alloc memory

    for (vsize i = 0; i < size_; ++i)
        data_[i] = value;                                   // init each element
}

// ----------------------------------------------------

template <typename T>
inline cvector<T>::cvector(const cvector<T>& other) :
    size_(other.size_),
    capacity_(other.capacity_)
{
    data_ = new T[capacity_];

    for (vsize i = 0; i < size_; ++i)
        data_[i] = other.data_[i];
}

// ----------------------------------------------------

template <typename T>
inline cvector<T>::cvector(cvector<T>&& other) noexcept :
    data_(std::exchange(other.data_, nullptr)),
    size_(std::exchange(other.size_, 0)),
    capacity_(std::exchange(other.capacity_, 0))
{
}

// ----------------------------------------------------

template <typename T>
inline cvector<T>::cvector(std::initializer_list<T> il) : cvector()
{
    assign(il.begin(), il.end());
}

// ----------------------------------------------------

template <typename T>
cvector<T>::~cvector()
{
    safe_delete();
    size_ = 0;
    capacity_ = 0;
}

// ----------------------------------------------------

template <typename T>
bool cvector<T>::operator==(const cvector<T>& rhs) const
{
    // check if sizes are equal
    if (this->size() != rhs.size())
        return false;

    bool isEqual = true;

    // check if elements are equal
    for (vsize i = 0; i < size(); ++i)
        isEqual |= (data_[i] == rhs[i]);

    return isEqual;
}

// =================================================================================
//                  assignment: copy, move, initializer_list
// =================================================================================
template <typename T>
inline cvector<T>& cvector<T>::operator=(const cvector<T>& rhs)
{
    // copy assignment operator

    if (this == &rhs) return *this;

    // realloc memory if need
    if (capacity_ < rhs.size_)
    {
        realloc_buffer_discard(rhs.size_);
    }

    // copy the data
    for (vsize i = 0; i < rhs.size_; ++i)
        data_[i] = rhs.data_[i];

    size_ = rhs.size_;

    return *this;
}

// ----------------------------------------------------

template <typename T>
inline cvector<T>& cvector<T>::operator=(cvector<T>&& rhs) noexcept
{
    if (this == &rhs) return *this;

    safe_delete();

    data_ = std::exchange(rhs.data_, nullptr);
    size_ = std::exchange(rhs.size_, 0);
    capacity_ = std::exchange(rhs.capacity_, 0);

    return *this;
}

// ----------------------------------------------------

template <typename T>
cvector<T>& cvector<T>::operator=(std::initializer_list<T> list)
{
    const vsize listSize = list.size();

    // if we need a bigger data buffer
    if (capacity_ < listSize)
    {
        realloc_buffer_discard(listSize);
    }

    size_ = listSize;

    // copy elems from the list
    for (vsize i = 0; const T & elem : list)
        data_[i++] = elem;

    return *this;
}


// =================================================================================
//                           get data by indices
// =================================================================================
template <typename T>
inline void cvector<T>::get_data_by_idxs(
    const cvector<index>& idxs,
    cvector<T>& outData) const
{
    // out:  array of data elements by input indices

    outData.resize(idxs.size());

    for (int i = 0; index idx : idxs)
        outData[i++] = data_[idx];
}

// ----------------------------------------------------

template <typename T>
void cvector<T>::get_data_by_idxs(const cvector<index>& idxs, T* outData) const
{
    // out:  array of data elements by input indices
    // NOTE: it is supposed that idxs.size() == outData.size()

    if constexpr (ENABLE_CHECK)
    {
        if (outData == nullptr)
        {
            error_msg("invalid input args", CALLER_INFO);
            return;
        }
    }

    for (int i = 0; index idx : idxs)
        outData[i++] = data_[idx];
}


// =================================================================================
//                               shift elements
// =================================================================================
template <typename T>
void cvector<T>::shift_right(const index idx, const int num)
{
    // shift right all the elements of range [idx, end) by the num positions;
    // idx - start index of the original range
    // num - the number of positions to shift

    if constexpr (ENABLE_CHECK)
    {
        if (!is_valid_index(idx) | (num <= 0))
        {
            error_msg("can't exec shift left", CALLER_INFO);
            return;
        }
    }

    // if our type is POD, or plain structure we use memmove
    if constexpr (std::is_standard_layout_v<T>)
    {
        index numToMove = size_ - idx - num;

        if ((numToMove > 0))
            memmove(&data_[idx + num], &data_[idx], numToMove * sizeof(T));
    }
    // we have some complex data type
    else
    {
        std::shift_right(begin() + idx, end(), num);
    }
}

// ----------------------------------------------------

template <typename T>
inline void cvector<T>::shift_left(const index idx, const int num)
{
    // shift left all the elements of range [idx, end) by the num positions;
    // idx - start index of the original range
    // num - the number of positions to shift;

    if constexpr (ENABLE_CHECK)
    {
        if (!is_valid_index(idx) | (num <= 0))
        {
            error_msg("can't exec shift left", CALLER_INFO);
            return;
        }
    }
  
    std::shift_left(begin() + idx, end(), num);
}


// =================================================================================                                                                                                              
//                               public setters
// =================================================================================
template <typename T>
inline void cvector<T>::push_back(const T& value)
{
    if (size_ == capacity_)
    {
        // create a new array with growFactor times the original capacity
        const vsize newCapacity = GetGrownCapacity(capacity_ ? capacity_ : 8);
        reserve(newCapacity);
    }

    data_[size_] = value;
    size_++;
}

// ----------------------------------------------------

template <typename T>
inline void cvector<T>::push_back(T&& rvalue)
{
    if (size_ == capacity_)
    {
        // create a new array with growFactor times the original capacity
        const vsize newCapacity = GetGrownCapacity(capacity_ ? capacity_ : 8);
        reserve(newCapacity);
    }

    data_[size_]
        =
        std::move(rvalue);
    size_++;
}

// ----------------------------------------------------

template <typename T>
inline void cvector<T>::erase(const vsize index)
{
    if constexpr (ENABLE_CHECK)
    {
        if ((index < 0) | (index >= size_))
        {
            error_msg("invalid input args", CALLER_INFO);
            return;
        }
    }

    for (vsize i = index; i < size_ - 1; ++i)
        data_[i] = std::move(data_[i + 1]);

    size_--;
}

// ----------------------------------------------------

template <typename T>
index cvector<T>::get_insert_idx(const ptrdiff_t& value) const
{
    // get position (index) into array for sorted INSERTION;
    // is used together with insert_before() method

    return std::distance(begin(), std::upper_bound(begin(), end(), value));
}

// ----------------------------------------------------

template <typename T>
void cvector<T>::get_insert_idxs(const cvector<T>& values, cvector<index>& idxs) const
{
    // get positions (indices) into array for sorted INSERTION;
    // is used together with insert_before() method

    const vsize sz = values.size();
    idxs.resize(sz);

    const T* b = begin();
    const T* e = end();

    for (index i = 0; i < sz; ++i)
        idxs[i] = std::distance(b, std::upper_bound(b, e, values[i]));
}

// ----------------------------------------------------

template <typename T>
void cvector<T>::get_insert_idxs(
    const T* values,
    const vsize numValues,
    cvector<index>& idxs) const
{
    // get positions (indices) into array for sorted INSERTION;
    // is used together with insert_before() method

    if constexpr (ENABLE_CHECK)
    {
        if ((values == nullptr) | (numValues < 0))
        {
            error_msg("invalid input args", CALLER_INFO);
            return;
        }
    }

    const T* b = begin();
    const T* e = end();
    idxs.resize(numValues);

    for (index i = 0; i < numValues; ++i)
        idxs[i] = std::distance(b, std::upper_bound(b, e, values[i]));
}

// ----------------------------------------------------

template <typename T>
void cvector<T>::insert_before(const vsize idx, const T& value)
{
    // insert input value before arr value by idx;
    // so input value will be right at this idx and all the rest will shift right;
    // (for instance insert 1 at idx 1: [0 2 3] becomes [0 1 2 3]

    if constexpr (ENABLE_CHECK)
    {
        if ((idx < 0) | (idx > size_))
        {
            error_msg("invalid input args", CALLER_INFO);
            return;
        }
    }

    if (capacity_ <= size_)
    {
        // create a new array with growFactor times the original capacity
        const vsize newCapacity = GetGrownCapacity(capacity_ ? capacity_ : 8);
        reserve(newCapacity);
    }

    // now we have one more element
    size_++;

    // prepare a place for the input element and set it by index
    shift_right(idx, 1);
    new (&data_[idx]) T(value);
}

// ----------------------------------------------------

template <typename T>
void cvector<T>::insert_before(const vsize idx, T&& value)
{
    // insert input value before arr value by idx;
    // so input value will be right at this idx and all the rest will shift right;
    // (for instance insert 1 at idx 1: [0 2 3] becomes [0 1 2 3]

    if constexpr (ENABLE_CHECK)
    {
        if ((idx < 0) | (idx > size_))
        {
            error_msg("invalid input args", CALLER_INFO);
            return;
        }
    }

    if (capacity_ <= size_)
    {
        // create a new array with growFactor times the original capacity
        const vsize newCapacity = GetGrownCapacity(capacity_ ? capacity_ : 8);
        reserve(newCapacity);
    }

    // now we have one more element
    size_++;

    // prepare a place for the input element and set it by index
    shift_right(idx, 1);
    new (&data_[idx]) T(std::move(value));
}

// ----------------------------------------------------

template <typename T>
template <typename U>
void cvector<T>::append_vector(U&& src)
{
    // move or copy the input cvector at the end of 
    // the current one (append one to another)

    const vsize base = size();
    const vsize srcSize = src.size();
    vsize newSize = base + srcSize;

    if (capacity_ < newSize)
    {
        // create a new array with growFactor times the original capacity
        const vsize newCapacity = GetGrownCapacity(newSize);
        reserve(newCapacity);
    }

    // move or copy the elements
    for (vsize i = 0; i < srcSize; ++i)
    {
        if constexpr (std::is_rvalue_reference<U&&>::value)
        {
            data_[base + i] = std::move(src[i]);
        }
        else
        {
            data_[base + i] = src[i];
        }
    }

    // if this is an rvalue, destroy the source cvector
    if constexpr (std::is_rvalue_reference<U&&>::value)
    {
        src.purge();
    }

    size_ = newSize;
}

// ----------------------------------------------------

template <typename T>
template <typename Iter>
inline void cvector<T>::assign(Iter first, Iter last)
{
    vsize const sz = vsize(last - first);
    if (size_ < sz) resize(sz);

    for (Iter it = first; it != last; ++it)
        new (&data_[vsize(it - first)]) T(*it);
}


// =================================================================================
//                                  search
// =================================================================================
template <typename T>
inline index cvector<T>::find(const T& val) const
{
    // NOTE:  is used for a cvector of RANDOMLY placed values;
    // DESC:  find first matching val and return its index;
    //        if there is no such elements return -1;

    auto it = std::find(begin(), end(), val);

    return (it != end()) ? std::distance(begin(), it) : -1;
}

// ----------------------------------------------------

template <typename T>
inline index cvector<T>::get_idx(const T& val) const
{
    // NOTE:  your (*this) cvector must be SORTED!
    // DESC:  get current position (index) into (*this) array for the input value

    return std::distance(begin(), std::upper_bound(begin(), end(), val)) - 1;
}

// ----------------------------------------------------

template <typename T>
inline void cvector<T>::get_idxs(
    const T* values,
    const vsize numElems,
    cvector<index>& outIdxs) const
{
    // NOTE:  your (*this) cvector must be SORTED!
    // out:   an arr of idxs to the input values

    if constexpr (ENABLE_CHECK)
    {
        if ((values == nullptr) | (numElems < 0))
        {
            error_msg("invalid input args", CALLER_INFO);
            return;
        }
    }
   
    outIdxs.resize(numElems);

    for (int i = 0; i < numElems; ++i)
        outIdxs[i] = std::distance(begin(), std::lower_bound(begin(), end(), values[i]));
}

// ----------------------------------------------------

template <typename T>
inline void cvector<T>::get_idxs(
    const cvector<T>& values,
    cvector<index>& outIdxs) const
{
    // NOTE:  your (*this) cvector must be SORTED!
    // out:   an arr of idxs to the input values

    outIdxs.resize(values.size());

    for (int i = 0; const T & val : values)
        outIdxs[i++] = std::distance(begin(), std::lower_bound(begin(), end(), val));
}

// ----------------------------------------------------

template <typename T>
inline bool cvector<T>::has_value(const T& val) const
{
    // NOTE:  for a cvector of RANDOMLY placed values:
    // DESC:  check if (*this) cvector has such a value

    return std::find(begin(), end(), val) != end();
}

// ----------------------------------------------------

template <typename T>
inline bool cvector<T>::binary_search(const T& val) const
{
    // NOTE: your (*this) cvector must be SORTED!
    return std::binary_search(begin(), end(), val);
}

// ----------------------------------------------------

template <typename T>
bool cvector<T>::binary_search(const cvector<T>& values) const
{
    // NOTE: your (*this) cvector must be SORTED!
    // check if each value from the input cvector exists in the current (*this) cvector

    bool isExist = true;
    const T* b = begin();
    const T* e = end();

    for (index i = 0; i < values.size(); ++i)
        isExist &= std::binary_search(b, e, values[i]);

    return isExist;
}

// ----------------------------------------------------

template <typename T>
bool cvector<T>::binary_search(const T* values, const vsize numElems) const
{
    // NOTE: your (*this) cvector must be SORTED!
    // check if each value from the input raw array exists in the current cvector

    if constexpr (ENABLE_CHECK)
    {
        if ((values == nullptr) | (numElems < 0))
        {
            error_msg("invalid input args", CALLER_INFO);
            return false;
        }
    }

    bool isExist = true;
    const T* b = begin();
    const T* e = end();

    for (index i = 0; i < numElems; ++i)
        isExist &= std::binary_search(b, e, values[i]);

    return isExist;
}

// ----------------------------------------------------

template <typename T>
void cvector<T>::binary_search(const T* values, vsize numElems, cvector<bool>& flags) const
{
    // NOTE: your (*this) cvector must be SORTED!
    // check if each value from the input raw array exists and put responsible boolean-flag into output array
    //
    // out: flags -- array of existing flags

    if constexpr (ENABLE_CHECK)
    {
        if ((values == nullptr) | (numElems < 0))
        {
            error_msg("invalid input args", CALLER_INFO);
            return;
        }
    }

    const T* b = begin();
    const T* e = end();

    flags.resize(numElems);

    for (index i = 0; i < numElems; ++i)
        flags[i] = std::binary_search(b, e, values[i]);
}


// =================================================================================
//                          change size / capacity
// =================================================================================
template <typename T>
inline void cvector<T>::reserve(const vsize newCapacity)
{
   // printf("reserve for :%s of size %d\n", typeid(T).name(), newCapacity);
    if (capacity_ < newCapacity)
        realloc_buffer(newCapacity);
}

// ----------------------------------------------------

template <typename T>
inline void cvector<T>::resize(const vsize newSize)
{
    if (capacity_ < newSize)
        realloc_buffer(newSize);

    size_ = newSize * (newSize >= 0);
}

// ----------------------------------------------------

template <typename T>
void cvector<T>::resize(const vsize newSize, const T& value)
{
    if (capacity_ < newSize)
    {
        realloc_buffer(newSize);

        for (vsize i = size_; i < capacity_; ++i)
            new (&data_[i]) T(value);
    }
    size_ = newSize * (newSize >= 0);
}


// =================================================================================
//                            memory deallocation
// =================================================================================
template <typename T>
void cvector<T>::shrink_to_fit()
{
    // requests the removal of unused capacity. 
    // so the capacity() may be reduced to size().

    if (size_ < capacity_)
        realloc_buffer(size_);
}

// ----------------------------------------------------

template <typename T>
void cvector<T>::purge()
{
    safe_delete();
    size_ = 0;
    capacity_ = 0;
}


// =================================================================================
//                              private methods
// =================================================================================
template <typename T>
void cvector<T>::vassert(
    const bool condition,
    const char* msg,
    const char* format,
    const char* fileName,
    const char* className,
    const char* funcName,
    const int line) const
{
    if (!condition)
    {
        const char* consoleRed = "\x1B[31m";
        const char* consoleNorm = "\x1B[0m";

        printf("%s\nERROR:\n", consoleRed);
        printf(format, fileName, className, funcName, line, msg);
        printf("%s", consoleNorm);
    }
}

// ----------------------------------------------------

template <typename T>
inline void cvector<T>::realloc_buffer_discard(const vsize newCapacity)
{
    // reallocate memory for a new buffer of capacity == newCapacity
    // without saving an old data;
    //
    // If reallocation occurs, all iterators(including the end() iterator) 
    // and all references to the elements are invalidated.

    try
    {
        // if we had any data before
        if (data_)
            safe_delete();

        data_ = new T[newCapacity];
        capacity_ = newCapacity;
    }
    catch (const std::bad_alloc& e)
    {
        error_msg(e.what(), CALLER_INFO);
        error_msg("can't allocate memory for buffer", CALLER_INFO);
    }
}

// ----------------------------------------------------

template <typename T>
inline void cvector<T>::realloc_buffer(const vsize newCapacity)
{
    // If reallocation occurs, all iterators(including the end() iterator) 
    // and all references to the elements are invalidated.

    try
    {
        // if we had any data before
        if (data_)
        {
            T* newData = new T[newCapacity]{};

            // if we need to store less elements than before
            if (newCapacity < size_)
                size_ = newCapacity;
            

            // TODO: test using memmove()
            // 
            // move necessary elements into the new buffer
            for (vsize i = 0; i < size_; ++i)
                newData[i] = std::move(data_[i]);

            // release memory from the old buffer
            delete[] data_;

            data_ = newData;
            capacity_ = newCapacity;
        }
        else
        {
            data_ = new T[newCapacity]{};
            capacity_ = newCapacity;
        }
    }
    catch (const std::bad_alloc& e)
    {
        error_msg(e.what(), CALLER_INFO);
        error_msg("can't allocate memory for buffer", CALLER_INFO);
    }
}

} // namespace Core
