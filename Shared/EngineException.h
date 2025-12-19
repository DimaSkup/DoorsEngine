//==================================================================================
// Filename:     EngineException.h
// Descption:    a wrapper class for manual exceptions of the DoorsEngine core
//==================================================================================
#pragma once

#include <comdef.h>         // for using the _com_error class which defines an error object
#include <source_location>


class EngineException
{
public:
    EngineException(
        const char* msg,
        const std::source_location& location = std::source_location::current(),
        const HRESULT hr = S_OK);

    //-----------------------------------------------------
    // Desc:  get a string buffer which contains already
    //        prepared message about exception
    // Ret:   a ptr to the buffer
    //-----------------------------------------------------
    inline const char* GetConstStr() const { return buf_; }

private:
    char buf_[512]{'\0'};
};
