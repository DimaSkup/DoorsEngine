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

enum LogType
{
    LOG_TYPE_MESSAGE,
    LOG_TYPE_DEBUG,
    LOG_TYPE_ERROR,
    LOG_TYPE_FORMATTED
};

struct LogMessage
{
    char*   msg = nullptr;
    LogType type = LOG_TYPE_MESSAGE;
};

struct LogStorage
{
    // here we store log messages (preferably we may use it in the UI log printing)

    LogMessage logs[2048];
    int        numLogs = 0;   // actual number of log messages
};


// macros for printing info about ther caller function
#define LOG_MSG "%s%s() (line: %d): %s%s",      GREEN, __FILE__, __func__, __LINE__, RESET
#define LOG_ERR "%sERROR: %s() (line: %d): %s%s", RED, __FILE__, __func__, __LINE__, RESET
#define LOG_DBG "DEBUG: %s() (line: %d): %s",          __FILE__, __func__, __LINE__

///////////////////////////////////////////////////////////

constexpr int g_StrLim = 512;
extern char   g_String[g_StrLim];

extern void SetupLogger(FILE* pLogFile, void* pLogStorage);

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
