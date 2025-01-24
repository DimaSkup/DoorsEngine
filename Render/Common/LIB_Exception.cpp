///////////////////////////////////////////////////////////////////////////////////////////
// Filename:     LIB_Exception.cpp
// Descption:    implementation of functional for the LIB_Exception class 
///////////////////////////////////////////////////////////////////////////////////////////
#include "LIB_Exception.h"
#include "StringHelper.h"  

namespace Render
{


LIB_Exception::LIB_Exception(
	HRESULT hr,
	const std::string& msg, 
	const std::string& file, 
	const std::string& function, 
	const int line)
{
	// generate an error string with data about some LIB_Exception so later we can
	// use this string for the ECS::Log::Error() function

	MakeExceptionMsg(hr, msg, file, function, line);
}

///////////////////////////////////////////////////////////

LIB_Exception::LIB_Exception(
	const std::string& msg,
	const std::source_location& location,
	const HRESULT hr)
{
	// generate an error string with data about some LIB_Exception so later we can
	// use this string for the logger printing functions

	MakeExceptionMsg(hr, msg, location.file_name(), location.function_name(), location.line());
}

///////////////////////////////////////////////////////////

const std::string& LIB_Exception::GetStr() const
{
	return errMsg_;
}

///////////////////////////////////////////////////////////
const std::wstring LIB_Exception::GetStrWide() const
{
	return StringHelper::StringToWide(errMsg_);
}


// *********************************************************************************

void LIB_Exception::MakeExceptionMsg(
	HRESULT hr,
	const std::string& msg,
	const std::string& file,
	const std::string& function,
	const int line)
{
	errMsg_ = "\nErrorMsg: " + msg;

	if (hr != NULL)
	{
		_com_error error(hr);

		errMsg_ += "\n";
		errMsg_ += StringHelper::ToString(error.ErrorMessage());
	}

	errMsg_ += "\nFile:     " + file;
	errMsg_ += "\nFunction: " + function + "()";
	errMsg_ += "\nLine:     " + std::to_string(line);
	errMsg_ += "\n\n";
}

}