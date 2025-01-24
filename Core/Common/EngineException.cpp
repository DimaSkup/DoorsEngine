///////////////////////////////////////////////////////////////////////////////////////////
// Filename:     EngineException.cpp
// Descption:    implementation of functional for the EngineException class 
///////////////////////////////////////////////////////////////////////////////////////////
#include "EngineException.h"
#include "StringHelper.h"

#include <DxErr.h>


EngineException::EngineException(
	HRESULT hr, 
	const std::string& msg,
	const std::string& file,
	const std::string& function,
	const int line)
{
	// generate an error string with data about some exception so later we can
	// use this string for the Log::Error() function

	MakeExceptionMsg(hr, msg, file, function, line);
}

///////////////////////////////////////////////////////////

EngineException::EngineException(
	const std::string& msg,
	const std::source_location& loc,
	const HRESULT hr)
{
	MakeExceptionMsg(hr, msg, loc.file_name(), loc.function_name(), loc.line());
}

///////////////////////////////////////////////////////////

const std::wstring EngineException::GetWideStr() const
{
	return StringHelper::StringToWide(errMsg_).c_str();
}

///////////////////////////////////////////////////////////

const std::string& EngineException::GetStr() const
{
	return errMsg_;
}


// *********************************************************************************

void EngineException::MakeExceptionMsg(
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

