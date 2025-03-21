// *********************************************************************************
// Filename:     Utils.h
// Description:  contains some common utils for the Entity-Component-System
// *********************************************************************************
#pragma once

#include "Types.h"
#include <string>
#include <vector>

namespace ECS
{


class Utils
{
public:
	// ****************************************************************************
// different help purposes API

	static std::string GetEnttsIDsAsString(
		const EntityID* ids,
		const int count,
		const std::string& glue = ", ");

	// ----------------------------------------------------

	template<class T>
	static void AppendToArray(std::vector<T>& head, const std::vector<T>& tail)
	{
		head.insert(head.end(), tail.begin(), tail.end());
	}

	// ----------------------------------------------------

	template<class T1, class T2>
	static bool CheckArrSizesEqual(
		const std::vector<T1>& arr1,
		const std::vector<T2>& arr2)
	{
		return std::ssize(arr1) == std::ssize(arr2);
	}


	// ****************************************************************************
	// sorted insertion API

	static index GetPosForID(
		const std::vector<EntityID>& enttsIDs,
		const EntityID& enttID);

	// ----------------------------------------------------


	template<class T>
	static void InsertAtPos(std::vector<T>& arr, const index pos, const T& val)
	{
		// std::insert() wrapper
		// insert val into the arr at pos (index) 
		arr.insert(arr.begin() + pos, val);
	}

	template<class T>
	static void InsertAtPos(std::vector<T>& arr, const index pos, const T&& val)
	{
		// std::insert() wrapper
		// insert val into the arr at pos (index) 
		arr.insert(arr.begin() + pos, val);
	}


	// ****************************************************************************
	// check existing API

	template<typename T>
	static void GetExistingFlags(
		const std::vector<T>& inArr,
		const std::vector<T>& values,
		std::vector<bool>& flags)
	{
		// out: arr of boolean flags which define if input values exist in inArr or not

		const auto beg = inArr.begin();
		const auto end = inArr.end();

		flags.resize(std::ssize(values));

		for (int i = 0; const T & val : values)
			flags[i++] = std::binary_search(beg, end, val);
	}

	// ------------------------------------------

	template<typename T>
	static bool BinarySearch(
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
	static bool CheckValuesExistInSortedArr(
		const std::vector<T>& inOrigArr,
		const std::vector<T>& inValuesArr)
	{
		// check if each value (from inValueArr) exists in the SORTED origin arr
		auto itBeg = inOrigArr.begin();
		auto itEnd = inOrigArr.end();
		bool isExist = true;

		for (const T& val : inValuesArr)
			isExist &= std::binary_search(itBeg, itEnd, val);

		return isExist;
	}

	// ------------------------------------------

	template<class T>
	static bool CheckValuesExistInSortedArr(
		const std::vector<T>& inOrigArr,
		const T* inValuesArr,
		const size inNumValues)
	{
		// check if each value (from inValueArr) exists in the SORTED origin arr
		auto itBeg = inOrigArr.begin();
		auto itEnd = inOrigArr.end();
		bool isExist = true;

		for (index i = 0; i < inNumValues; ++i)
			isExist &= std::binary_search(itBeg, itEnd, inValuesArr[i]);

		return isExist;
	}

	// ------------------------------------------

	template<class T>
	static bool CheckValuesExistInArr(
		const std::vector<T>& inOrigArr,
		const std::vector<T>& inValuesArr)
	{
		// check if each value (from inValueArr) exists in
		// the origin arr (with randomly placed values)
		auto itBeg = inOrigArr.begin();
		auto itEnd = inOrigArr.end();
		bool isExist = true;

		for (const T& val : inValuesArr)
			isExist &= (std::find(itBeg, itEnd, val) != itEnd);

		return isExist;
	}

	// ----------------------------------------------------

	template<typename T>
	static bool ArrHasVal(
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
	static index GetIdxInSortedArr(
		const std::vector<T>& arr,
		const T& val)
	{
		// NOTE:   we subtract 1 from the final result because of upper_bound which 
		//         returns the idx right after the searched value
		// return: pos (data idx) of searched value

		return std::distance(arr.begin(), std::upper_bound(arr.begin(), arr.end(), val)) - 1;
	}

	// ------------------------------------------

	template<class T>
	static void GetIdxsInSortedArr(
		const std::vector<T>& inOrigArr,
		const std::vector<T>& inValuesArr,
		std::vector<index>& outIdxsArr)
	{
		// get arr of idxs to values

		auto itBeg = inOrigArr.begin();
		auto itEnd = inOrigArr.end();

		const size numSearchValues = std::ssize(inValuesArr);
		outIdxsArr.resize(numSearchValues);

		for (int i = 0; const T& val : inValuesArr)
			outIdxsArr[i++] = std::distance(itBeg, std::lower_bound(itBeg, itEnd, val));
	}


	template<class T>
	static void GetIdxsInSortedArr(
		const std::vector<T>& inOrigArr,
		const T* inValuesArr,
		const size inNumValues,
		std::vector<index>& outIdxs)
	{
		// get arr of idxs to values

		auto itBeg = inOrigArr.begin();
		auto itEnd = inOrigArr.end();

		outIdxs.resize(inNumValues);

		for (index i = 0; i < inNumValues; ++i)
			outIdxs[i] = std::distance(itBeg, std::lower_bound(itBeg, itEnd, inValuesArr[i]));
	}

	// ----------------------------------------------------

	template<typename T>
	static index FindIdxOfVal(
		const std::vector<T>& arr,
		const T& value)
	{
		// input:  1. an array of RANDOMLY placed values
		//         2. a searched value
		// 
		// return: idx into the array of searched value

		return std::distance(arr.begin(), std::find(arr.begin(), arr.end(), value));
	}
};


} // namespace ECS
