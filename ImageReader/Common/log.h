// =================================================================================
// Filename:    Log.h
// Description: just logger
// =================================================================================
#pragma once

#include <source_location>
#include "LIB_Exception.h"
#include "ConsoleColorMacro.h"

#pragma warning (disable : 4996)

namespace ImgReader
{

// macros for printing info about the caller
#define LOG_INFO "%s:%s() (line: %d): %s" __FILE__, __func__, __LINE__

///////////////////////////////////////////////////////////

extern char g_String[256];

extern bool InitLogger();        // call it at the very beginning of the application
extern void CloseLogger();       // call it at the very end of the application

// for C++20
extern void LogMsg(const char* msg, const std::source_location& location = std::source_location::current());   // using: LogMsg("your msg");
extern void LogDbg(const char* msg, const std::source_location& location = std::source_location::current());
extern void LogErr(const char* msg, const std::source_location& location = std::source_location::current());

// for C and C++ under C++20
extern void LogMsg(const char* fileName, const char* funcName, const int codeLine, const char* msg);           // using: LogMsg(LOG_INFO, "your msg");
extern void LogDbg(const char* fileName, const char* funcName, const int codeLine, const char* msg);
extern void LogErr(const char* fileName, const char* funcName, const int codeLine, const char* msg);


// variadic arguments
extern void LogMsgf(const char* format, ...);

// exception handlers
extern void LogErr(const LIB_Exception* pException, const bool showMsgBox = false);
extern void LogErr(const LIB_Exception& e,          const bool showMsgBox = false);

} // namespace ImgReader
