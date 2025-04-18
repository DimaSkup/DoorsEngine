// =================================================================================
// Filename:    Log.h
// Description: just logger
// =================================================================================
#pragma once

#include <source_location>
#include "LIB_Exception.h"
#include "ConsoleColorMacro.h"

#pragma warning (disable : 4996)

namespace Render
{

// macros for printing info about ther caller function
#define LOG_MSG "%s%s() (line: %d): %s%s",      GREEN, __FILE__, __func__, __LINE__, RESET
#define LOG_ERR "%sERROR: %s() (line: %d): %s%s", RED, __FILE__, __func__, __LINE__, RESET
#define LOG_DBG "DEBUG: %s() (line: %d): %s",          __FILE__, __func__, __LINE__

///////////////////////////////////////////////////////////

extern char g_String[256];

extern bool InitLogger();
extern void CloseLogger();


extern void LogMsg(const char* msg, const std::source_location& location = std::source_location::current());
extern void LogDbg(const char* msg, const std::source_location& location = std::source_location::current());
extern void LogErr(const char* msg, const std::source_location& location = std::source_location::current());

#if 0
extern void LogMsg(const std::string& msg, const std::source_location& location = std::source_location::current());
extern void LogDbg(const std::string& msg, const std::source_location& location = std::source_location::current());
extern void LogErr(const std::string& msg, const std::source_location& location = std::source_location::current());
#endif

// variadic arguments
extern void LogMsgf(const char* format, ...);

// exception handlers
extern void LogErr(const LIB_Exception* pException, const bool showMsgBox = false);
extern void LogErr(const LIB_Exception& e,          const bool showMsgBox = false);

} // namespace
