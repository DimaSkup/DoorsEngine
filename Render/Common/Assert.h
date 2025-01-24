// *********************************************************************************
// Filename:     Assert.h
// Descption:    contains functional of asserting different conditions;
//               if we can't assert something then we throw a LIB_Exception exception
// 
// Created:      12.08.24
// *********************************************************************************
#pragma once

#include <string>
#include <source_location>
#include "LIB_Exception.h"

namespace Render
{

class Assert final
{
public:

	inline static void True(
		const bool boolean,
		const std::string& msg,
		const std::source_location& location = std::source_location::current())
	{
		if (boolean != true)
			throw LIB_Exception(0, msg, location.file_name(), location.function_name(), location.line());
	}

	// ----------------------------------------------------

	inline static void NotNullptr(
		const void* ptr,
		const std::string& msg,
		const std::source_location& location = std::source_location::current())
	{
		if (ptr == nullptr)
			throw LIB_Exception(0, msg, location.file_name(), location.function_name(), location.line());
	}

	// ----------------------------------------------------

	inline static void NotFailed(
		const HRESULT hr,
		const std::string& msg,
		const std::source_location& location = std::source_location::current())
	{
		if (FAILED(hr))       
			throw LIB_Exception(hr, msg, location.file_name(), location.function_name(), location.line());
	}

	// ----------------------------------------------------

	template <class T>
	inline static void NotZero(
		const T value,
		const std::string& msg,
		const std::source_location& location = std::source_location::current())
	{
		if (value == 0)
			throw LIB_Exception(0, msg, location.file_name(), location.function_name(), location.line());
	}

	// ----------------------------------------------------

	inline static void NotEmpty(
		const bool isEmpty,
		const std::string& msg,
		const std::source_location& location = std::source_location::current())
	{
		True(isEmpty != true, msg, location);
	}

	// ----------------------------------------------------
};

};  // namespace Render