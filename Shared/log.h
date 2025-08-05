// =================================================================================
// Filename:    Log.h
// Description: just logger
// =================================================================================
#pragma once

#include "EngineException.h"

#pragma warning (disable : 4996)
#define LOG_BUF_SIZE 512
#define LOG_STORAGE_SIZE 1024

// is necessary to differ logs when we print it in the editor's GUI
enum eLogType
{
    LOG_TYPE_MESSAGE,
    LOG_TYPE_DEBUG,
    LOG_TYPE_ERROR,
    LOG_TYPE_FORMATTED
};

struct LogMessage
{
    char*   msg = nullptr;
    eLogType type = LOG_TYPE_MESSAGE;
};

// here we store log messages (preferably we may use it in the UI log printing)
struct LogStorage
{
    LogMessage logs[LOG_STORAGE_SIZE];
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


// macros for standard log message (info about caller: file_name, func_name, code_line, message)
#define LOG __FILE__, __func__, __LINE__

// string global container
extern char g_String[LOG_BUF_SIZE];

///////////////////////////////////////////////////////////


extern int  InitLogger(const char* logFileName);      // call it at the very beginning of the application
extern void CloseLogger();                            // call it at the very end of the application
extern void SetConsoleColor(const char* keyColor);

FILE*             GetLogFile();
const LogStorage* GetLogStorage();


void LogMsg(const char* format, ...);

void LogMsg(
    const char* fileName,
    const char* funcName,
    const int codeLine,
    const char* format,
    ...);

void LogDbg(
    const char* fileName,
    const char* funcName,
    const int codeLine,
    const char* format,
    ...);

void LogErr(
    const char* fileName,
    const char* funcName,
    const int codeLine,
    const char* format,
    ...);

// exception handlers
void LogErr(const EngineException* pException, const bool showMsgBox = false);
void LogErr(const EngineException& e,          const bool showMsgBox = false);
