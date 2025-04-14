///////////////////////////////////////////////////////////////////////////////////////////
// Filename:     EngineException.cpp
// Descption:    a wrapper class for manual exceptions of the DoorsEngine core
///////////////////////////////////////////////////////////////////////////////////////////
#include "EngineException.h"
#pragma warning (disable : 4996)


namespace Core
{

EngineException::EngineException(
    const char* msg,
    const std::source_location& location,
    const HRESULT hr)
{
    // generate an error string with data about some EngineException so later we can
    // use this string for the logger printing functions

    MakeExceptionMsg(hr, msg, location);
}

///////////////////////////////////////////////////////////

void EngineException::MakeExceptionMsg(
    HRESULT hr,
    const char* msg,
    const std::source_location& location)
{
    // TODO: FIXME (getting error msg from the HR)
#if 0
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

} // namespace Core
