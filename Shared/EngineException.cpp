///////////////////////////////////////////////////////////////////////////////////////////
// Filename:     EngineException.cpp
// Descption:    a wrapper class for manual exceptions of the DoorsEngine core
///////////////////////////////////////////////////////////////////////////////////////////
#include "EngineException.h"
#include "FileSystem.h"
#include <time.h>
#pragma warning (disable : 4996)

//---------------------------------------------------------
// Desc:  generate an error string with data about some EngineException so later we can
//        use this string for the logger printing functions
// Args:  - msg:      text content of an exception
//        - location: info about the place where this exception was thrown
//        - hr:       error code
//---------------------------------------------------------
EngineException::EngineException(
    const char* msg,
    const std::source_location& location,
    const HRESULT hr)
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

    const time_t time = clock();

    const char* fmt =
        "[%05ld] ERROR:\n"
        "FILE:  %s\n"
        "FUNC:  %s()\n"
        "LINE:  %d\n"
        "MSG:   %s\n\n";

    char path[256]{'\0'};
    FileSys::GetPathFromProjRoot(location.file_name(), path);

    // string
    snprintf(
        buf_,                       // dst buffer
        512,                        // buffer size limit
        fmt,
        time,
        path,
        location.function_name(),
        location.line(),
        msg);
}
