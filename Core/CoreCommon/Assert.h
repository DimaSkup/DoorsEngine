// *********************************************************************************
// Filename:     Assert.h
// Descption:    contains functional of asserting different conditions;
//               if we can't assert something then we throw an EngineException
// 
// Created:      12.08.24
// *********************************************************************************
#pragma once

#include <source_location>
#include "EngineException.h"


namespace Core
{

class Assert
{
public:

	inline static void True(
		const bool boolean,
		const char* msg,
		const std::source_location& location = std::source_location::current())
	{
		// throwable version
		if (boolean != true)
			throw EngineException(msg, location, 0);
	}

	// ----------------------------------------------------

	inline static void NotNullptr(
		const void* ptr,
		const char* msg,
		const std::source_location& location = std::source_location::current())
	{
		if (ptr == nullptr)
			throw EngineException(msg, location, 0);
	}

	// ----------------------------------------------------

	inline static void NotFailed(
		const HRESULT hr,
		const char* msg,
		const std::source_location& location = std::source_location::current())
	{
		if (FAILED(hr))       
			throw EngineException(msg, location, hr);
	}

	// ----------------------------------------------------

	template <class T>
	inline static void NotZero(
		const T value,
		const char* msg,
		const std::source_location& location = std::source_location::current())
	{
		if (value == 0)
			throw EngineException(msg, location, 0);
	}

	// ----------------------------------------------------

	inline static void NotEmpty(
		const bool isEmpty,
		const char* msg,
		const std::source_location& location = std::source_location::current())
	{
		True(isEmpty != true, msg, location);
	}
};

};  // namespace Core
