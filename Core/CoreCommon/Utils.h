// *********************************************************************************
// Filename:     Utils.h
// Description:  contains some common utils for the engine
// *********************************************************************************
#pragma once
#include "Types.h"
#include "StringHelper.h"

#include <vector>
#include <algorithm>

#include <cctype>
#include <random>

namespace CoreUtils
{
	

// ****************************************************************************
// different help purposes API

template<typename T>
inline std::string JoinArrIntoStr(
	const std::vector<T>& arrOfNum,
	const std::string& glue = ", ")
{
	std::vector<std::string> arrNumAsStr = Core::StringHelper::ConvertNumbersIntoStrings<T>(arrOfNum);
	return Core::StringHelper::Join(std::move(arrNumAsStr), glue);
}

// ------------------------------------------

template<class T>
inline void AppendToArray(std::vector<T>& head, const std::vector<T>& tail)
{
	head.insert(head.end(), tail.begin(), tail.end());
}

// ------------------------------------------

template<class T>
inline void GetRangeOfArr(
	const std::vector<T>& arr,
	const u32 firstIdx,
	const u32 lastIdx,
	std::vector<T>& outValues)
{
	// get a range of values from the input arr
		
	for (u32 idx = firstIdx; idx < lastIdx; ++idx)
		outValues.push_back(arr[idx]);
}

// ------------------------------------------


template<class T>
inline std::vector<T> GetRangeOfArr(
	const std::vector<T>& arr,
	const u32 firstIdx,
	const u32 lastIdx)
{
	// RETURN a range of values from the input arr

	std::vector<T> outRange;
	//outRange.reserve(lastIdx - firstIds);

	for (u32 idx = firstIdx; idx < lastIdx; ++idx)
		outRange.push_back(arr[idx]);

	return outRange;
}


// ****************************************************************************
// sorted insertion API

template<class T>
inline static ptrdiff_t GetPosForVal(
	const std::vector<T>& arr,
	const T& value)
{
	// get position (index) into array for sorted insertion of value
	// 
	// input:  1. a SORTED array
	//         2. a value for which we want to find an insertion pos
	// return:    an insertion idx for value

	return std::distance(arr.begin(), std::upper_bound(arr.begin(), arr.end(), value));
}

template<class T>
inline static void InsertAtPos(
	std::vector<T>& arr,
	const ptrdiff_t pos,
	const T& val)
{
	// std::insert() wrapper
	// insert val into the arr at pos (index) 
	arr.insert(arr.begin() + pos, val);
}

template<class T>
inline static void InsertAtPos(
	std::vector<T>& arr,
	const ptrdiff_t pos,
	T&& val)
{
	// std::insert() wrapper
	// insert val into the arr at pos (index) 
	arr.emplace(arr.begin() + pos, std::move(val));
}

	

// ****************************************************************************
// check existing API

template<typename T>
inline static bool BinarySearch(
	const std::vector<T>& arr,
	const T& value)
{
	// std::binary_search() wrapper
	// input:  1. a SORTED arr
	//         2. a searched value
	// return: true - if there is such a value; or false - in another case

	return std::binary_search(arr.begin(), arr.end(), value);
}

// ------------------------------------------

template<class T>
inline bool CheckValuesExistInArr(
	const std::vector<T>& inOrigArr,
	const std::vector<T>& inValuesArr)
{
	// check if each value (from inValueArr) exists in the origin arr
	auto itBeg = inOrigArr.begin();
	auto itEnd = inOrigArr.end();
	bool isExist = true;

	for (u32 idx = 0; const T & val : inValuesArr)
		isExist &= std::binary_search(itBeg, itEnd, val);

	return isExist;
}

// ------------------------------------------

template<typename T>
inline static bool ArrHasVal(
	const std::vector<T>& arr,
	const T& val)
{
	// input:  1. an array of RANDOMLY placed values
	//         2. a searched value
	// return: true - if there is such a value; or false - in another case

	return std::find(arr.begin(), arr.end(), val) != arr.end();
}


// ****************************************************************************
// search for index API

template<class T>
inline ptrdiff_t GetIdxInSortedArr(const std::vector<T>& arr, const T& val)
{
	// get current position (index) into the SORTED array for input value
	// NOTE:   we subtract 1 from the final result because of upper_bound which 
	//         returns the idx right after the searched value
	// return: data idx of searched value

	return std::distance(arr.begin(), std::upper_bound(arr.begin(), arr.end(), val)) - 1;
}

// ------------------------------------------

template<class T>
inline void GetIdxsInSortedArr(
	const std::vector<T>& inOrigArr,
	const std::vector<T>& inValuesArr,
	std::vector<ptrdiff_t>& outIdxsArr)
{
	// get arr of idxs to values from the SORTED input array

	auto itBeg = inOrigArr.begin();
	auto itEnd = inOrigArr.end();

	outIdxsArr.resize(inValuesArr.size());

	for (u32 idx = 0; const T& val : inValuesArr)
		outIdxsArr[idx++] = std::distance(itBeg, std::upper_bound(itBeg, itEnd, val)) - 1;
}

// ------------------------------------------

template<typename T>
inline static ptrdiff_t FindIdxOfVal(
	const std::vector<T>& arr,
	const T& value)
{
	// input:  1. an array of RANDOMLY placed values
	//         2. a searched value
	// 
	// return: success: idx into the array of searched value
	//         fail:    arr size

	return std::distance(arr.begin(), std::find(arr.begin(), arr.end(), value));
}

// ------------------------------------------

template<class T>
inline void GetIdxsInArr(
	const std::vector<T>& inOrigArr,
	const std::vector<T>& inValuesArr,
	std::vector<ptrdiff_t>& outIdxsArr)
{
	// in:  inOrigArr  - arr of RANDOMLY placed values
	//      inValueArr - arr of values which idxs we will seach for
	// 
	// out: arr of idxs

	auto itBeg = inOrigArr.begin();
	auto itEnd = inOrigArr.end();

	outIdxsArr.resize(inValuesArr.size());

	for (u32 idx = 0; const T & val : inValuesArr)
		outIdxsArr[idx++] = std::distance(itBeg, std::find(itBeg, itEnd, val)) ;
}


// ****************************************************************************
// generators of rand values

static u32 GenID()
{
	// generate random uint32_t value which can be used as ID

	std::random_device os_seed;
	std::mt19937 generator(os_seed());
	std::uniform_int_distribution<u32> distribute(0, UINT_MAX);

	// return generated ID
	return distribute(generator);
}


}  // namespace CoreUtils

