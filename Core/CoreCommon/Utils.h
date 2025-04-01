// *********************************************************************************
// Filename:     Utils.h
// Description:  contains some common utils for the engine
// *********************************************************************************
#pragma once
#include "Types.h"
#include "StringHelper.h"

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


// ****************************************************************************
// generators of rand values

static uint32_t GenID()
{
	// generate random uint32_t value which can be used as ID

	std::random_device os_seed;
	std::mt19937 generator(os_seed());
	std::uniform_int_distribution<uint32_t> distribute(0, UINT_MAX);

	// return generated ID
	return distribute(generator);
}


}  // namespace CoreUtils

