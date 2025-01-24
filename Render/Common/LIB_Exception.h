///////////////////////////////////////////////////////////////////////////////////////////
// Filename:     LIB_Exception.h
// Descption:    a wrapper class for manual LIB_Exceptions of the DoorsEngine
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <comdef.h>         // for using the _com_error class which defines an error object
#include <source_location>
#include <string>

namespace Render
{

class LIB_Exception
{
public:
	LIB_Exception(
		HRESULT hr,
		const std::string& msg,
		const std::string& file, 
		const std::string& function, 
		const int line);

	LIB_Exception(
		const std::string& msg,
		const std::source_location& location = std::source_location::current(),
		const HRESULT hr = S_OK);

	const std::string& GetStr() const;
	const std::wstring GetStrWide() const;

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

} // namespace Render