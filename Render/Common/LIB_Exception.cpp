///////////////////////////////////////////////////////////////////////////////////////////
// Filename:     LIB_Exception.cpp
// Descption:    implementation of functional for the LIB_Exception class 
///////////////////////////////////////////////////////////////////////////////////////////
#include "LIB_Exception.h"
#pragma warning (disable : 4996)


namespace Render
{

LIB_Exception::LIB_Exception(
    const char* msg,
    const std::source_location& location,
    const HRESULT hr)
{
    // generate an error string with data about some LIB_Exception so later we can
    // use this string for the logger printing functions

    MakeExceptionMsg(hr, msg, location);
}

///////////////////////////////////////////////////////////

void LIB_Exception::MakeExceptionMsg(
    HRESULT hr,
    const char* msg,
    const std::source_location& location)
{
    
#if 0
    // TODO: FIXME (getting error msg from the HR)
    if (hr != NULL)
    {
        _com_error error(hr);

        errMsg_ += "\n";
        errMsg_ += StringHelper::ToString(error.ErrorMessage());
    }
#endif

    // string
    sprintf(strBuf_,                                         // dst buffer
        "\nErrMsg: %s \nFile: %s \nFunction: %s() \nLine: %d \n\n",
        msg,
        location.file_name(),
        location.function_name(),
        location.line());

    // wstring
    wsprintf(wstrBuf_,                                             // dst buffer
        L"\nErrMsg: %s \nFile: %s \nFunction: %s() \nLine: %d \n\n",
        msg,
        location.file_name(),
        location.function_name(),
        location.line());
}

} // namespace
