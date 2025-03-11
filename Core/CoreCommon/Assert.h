// *********************************************************************************
// Filename:     Assert.h
// Descption:    contains functional of asserting different conditions;
//               if we can't assert something then we throw a EngineException exception
// 
// Created:      12.08.24
// *********************************************************************************
#pragma once

#include "EngineException.h"
#include <string>
#include <source_location>


namespace Core
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
			throw EngineException(0, msg, location.file_name(), location.function_name(), location.line());
	}

	inline static void True(
		const bool boolean,
		const char* msg,
		const std::source_location& location = std::source_location::current())
	{
		if (boolean != true)
			throw EngineException(0, msg, location.file_name(), location.function_name(), location.line());
	}

	// ----------------------------------------------------

	inline static void NotNullptr(
		const void* ptr,
		const std::string& msg,
		const std::source_location& location = std::source_location::current())
	{
		if (ptr == nullptr)
			throw EngineException(0, msg, location.file_name(), location.function_name(), location.line());
	}

	inline static void NotNullptr(
		const void* ptr,
		const char* msg,
		const std::source_location& location = std::source_location::current())
	{
		if (ptr == nullptr)
			throw EngineException(0, msg, location.file_name(), location.function_name(), location.line());
	}

	// ----------------------------------------------------

	inline static void NotFailed(
		const HRESULT hr,
		const std::string& msg,
		const std::source_location& location = std::source_location::current())
	{
		if (FAILED(hr))       
			throw EngineException(hr, msg, location.file_name(), location.function_name(), location.line());
	}

	// ----------------------------------------------------

	template <class T>
	inline static void NotZero(
		const T value,
		const std::string& msg,
		const std::source_location& location = std::source_location::current())
	{
		if (value == 0)
			throw EngineException(0, msg, location.file_name(), location.function_name(), location.line());
	}

	// ----------------------------------------------------

	inline static void NotEmpty(
		const bool condition,
		const std::string& msg,
		const std::source_location& location = std::source_location::current())
	{
		if (condition == true)
			throw EngineException(0, msg, location.file_name(), location.function_name(), location.line());
	}

	// ----------------------------------------------------
};


} // namespace Core