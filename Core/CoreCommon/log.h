// =================================================================================
// Filename:    Log.h
// Description: just logger
// =================================================================================
#pragma once

#include <source_location>
#include "EngineException.h"

#pragma warning (disable : 4996)

namespace Core
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

// macros to setup console color
#define RESET       "\033[0m"
#define BLACK       "\033[30m"              /* Black */
#define RED         "\033[31m"              /* Red */
#define GREEN       "\033[32m"              /* Green */
#define YELLOW      "\033[33m"              /* Yellow */
#define BLUE        "\033[34m"              /* Blue */
#define MAGENTA     "\033[35m"              /* Magenta */
#define CYAN        "\033[36m"              /* Cyan */
#define WHITE       "\033[37m"              /* White */
#define BOLDBLACK   "\033[1m\033[30m"       /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"       /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"       /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"       /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"       /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"       /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"       /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"       /* Bold White */

///////////////////////////////////////////////////////////

// macros for printing info about the caller
#define LOG_INFO "%s:%s() (line: %d): %s" __FILE__, __func__, __LINE__

///////////////////////////////////////////////////////////

extern char g_String[256];

extern bool InitLogger(const char* logFileName);      // call it at the very beginning of the application
extern void CloseLogger();                            // call it at the very end of the application

extern FILE*             GetLogFile();
extern const LogStorage* GetLogStorage();


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
extern void LogErr(const EngineException* pException, const bool showMsgBox = false);
extern void LogErr(const EngineException& e,          const bool showMsgBox = false);

} // namespace Core
