// *********************************************************************************
// Filename:     Assert.h
// Descption:    contains functional of asserting different conditions;
//               if we can't assert something then we throw a LIB_Exception exception
// 
// Created:      12.08.24
// *********************************************************************************
#pragma once

#include <source_location>
#include "LIB_Exception.h"

namespace Render
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
			throw LIB_Exception(msg, location, 0);
	}

	// ----------------------------------------------------

	inline static void NotNullptr(
		const void* ptr,
		const char* msg,
		const std::source_location& location = std::source_location::current())
	{
		if (ptr == nullptr)
			throw LIB_Exception(msg, location, 0);
	}

	// ----------------------------------------------------

	inline static void NotFailed(
		const HRESULT hr,
		const char* msg,
		const std::source_location& location = std::source_location::current())
	{
		if (FAILED(hr))       
			throw LIB_Exception(msg, location, hr);
	}
};

};  // namespace
