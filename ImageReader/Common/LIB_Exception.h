///////////////////////////////////////////////////////////////////////////////////////////
// Filename:     LIB_Exception.h
// Descption:    a wrapper class for manual LIB_Exceptions of the ImageReader module
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <comdef.h>         // for using the _com_error class which defines an error object
#include <source_location>


namespace ImgReader
{

class LIB_Exception
{
public:
    LIB_Exception(
        const char* msg,
        const std::source_location& location = std::source_location::current(),
        const HRESULT hr = S_OK);

    inline const char*    GetConstStr() const { return strBuf_; }
    inline const wchar_t* GetStrWide()  const { return wstrBuf_; }

private:
    void MakeExceptionMsg(
        HRESULT hr,
        const char* msg,
        const std::source_location& location);

private:
    char    strBuf_[256]{'\0'};
    wchar_t wstrBuf_[256]{L'\0'};
};

} // namespace ImgReader
