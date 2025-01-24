///////////////////////////////////////////////////////////////////////////////////////////
// Filename:     EngineException.h
// Descption:    a wrapper class for manual exceptions of the DoorsEngine
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once



#include <comdef.h>         // for using the _com_error class which defines an error object
#include <source_location>
#include <string>


class EngineException
{
public:
	EngineException(
		HRESULT hr, 
		const std::string& msg, 
		const std::string& file, 
		const std::string& function,
		const int line);

	EngineException(
		const std::string& msg,
		const std::source_location& location = std::source_location::current(),
		const HRESULT hr = S_OK);

	const std::string& GetStr() const;
	const std::wstring GetWideStr() const;

private:
	void MakeExceptionMsg(
		HRESULT hr,
		const std::string& msg,
		const std::string& file,
		const std::string& function,
		const int line);

private:
	std::string errMsg_;
};