// =================================================================================
// Filename: Log.cpp
// =================================================================================
#include "Log.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#pragma warning (disable : 4996)


namespace Render
{

char g_String[256]{ '\0' };                 // global buffer for characters
char s_TmpStr[256]{ '\0' };                 // static buffer for internal using

static LogStorage* s_pLogStorage = nullptr; // static pointer to the log storage   (we get to actual pointer from the Core when call SetupLogger() function) 
static FILE* s_pLogFile = nullptr;          // a static descriptor of the log file (we get actual pointer from the Core when call SetupLogger() function)

// helpers prototypes
void        GetPathFromProjRoot(const char* fullPath, char* outPath);
void        PrintHelper(const char* lvlText, const char* text, const LogType type);
const char* PrepareMsg(const char* msg, const std::source_location& loc);
void        PrintExceptionErrHelper(const LIB_Exception& e, const bool showMsgBox);

// =================================================================================

void SetupLogger(FILE* pLogFile, void* pLogStorage)
{
    if (!pLogFile)
    {
        LogErr("can't setup logger: input ptr to the log file == nullptr");
        return;
    }

    if (!pLogStorage)
    {
        LogErr("can't setup logger: input ptr to the log storage == nullptr");
        return;
    }

    s_pLogFile = pLogFile;

    // NOTE: structures LogStorage and LogMessage must be the same as in the Core::Log.h
    s_pLogStorage = (LogStorage*)pLogStorage;
}

///////////////////////////////////////////////////////////

void AddMsgIntoLogStorage(const char* msg, const LogType type)
{
    // add a new message into the log storage 

    if (!msg || msg[0] == '\0' || !s_pLogStorage)
        return;

    int& numLogs    = s_pLogStorage->numLogs;
    LogMessage& log = s_pLogStorage->logs[numLogs];

    char** buf = &log.msg;                          // get a buffer to the text of this log
    log.type = type;                                // store the type of this log

    *buf = new char[strlen(msg) + 1]{ '\0' };
    strcpy(*buf, msg);

    ++numLogs;
}


// =================================================================================
// logger functions which prints info about its caller (file, function, line)
// =================================================================================
void LogMsg(const char* msg, const std::source_location& loc)
{
    printf("%s", GREEN);                                // setup console color
    PrepareMsg(msg, loc);
    PrintHelper("", s_TmpStr, LOG_TYPE_MESSAGE);
    printf("%s", RESET);                                // reset console color
}

///////////////////////////////////////////////////////////

void LogDbg(const char* msg, const std::source_location& loc)
{
    PrepareMsg(msg, loc);
    PrintHelper("DEBUG: ", s_TmpStr, LOG_TYPE_DEBUG);
}

///////////////////////////////////////////////////////////

void LogErr(const char* msg, const std::source_location& loc)
{
    printf("%s", RED);                                  // setup console color
    PrepareMsg(msg, loc);
    PrintHelper("ERROR: ", s_TmpStr, LOG_TYPE_ERROR);
    printf("%s", RESET);                                // reset console color
}

///////////////////////////////////////////////////////////

#if 0
void LogMsg(const std::string& msg, const std::source_location& location = std::source_location::current());
void LogDbg(const std::string& msg, const std::source_location& location = std::source_location::current());
void LogErr(const std::string& msg, const std::source_location& location = std::source_location::current());
#endif



// =================================================================================
// logger function with variadic arguments
// =================================================================================
void LogMsgf(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    vsprintf(s_TmpStr, format, args);
    PrintHelper("", s_TmpStr, LOG_TYPE_FORMATTED);
    printf("%s", RESET);                  // reset console color

    va_end(args);
}


// =================================================================================
// exception handlers
// =================================================================================
void LogErr(const LIB_Exception* pException, bool showMsgBox)
{
    // exception ERROR PRINTING (takes a pointer to the EngineException)
    PrintExceptionErrHelper(*pException, showMsgBox);
}

///////////////////////////////////////////////////////////

void LogErr(const LIB_Exception& e, const bool showMsgBox)
{
    // exception ERROR PRINTING (takes a reference to the EngineException)
    PrintExceptionErrHelper(e, showMsgBox);
}


// =================================================================================
// Helpers
// =================================================================================
void GetPathFromProjRoot(const char* fullPath, char* outPath)
{
    // return relative path from the project root

    if ((!fullPath) || (fullPath[0] == '\0'))
    {
        LogErr("input path is empty!");
        return;
    }

    if (!outPath)
    {
        LogErr("in-out path == nullptr");
        return;
    }

    const char* found = strstr(fullPath, "DoorsEngine\\");

    // if we found the substring we copy all the text after "DoorsEngine\"
    if (found != nullptr)
        strcpy(outPath, found + strlen("DoorsEngine\\"));
    else
        outPath[0] = '\0';
}

///////////////////////////////////////////////////////////

void PrintHelper(const char* lvlText, const char* text, const LogType type)
{
    // a helper for printing messages into the command prompt
    // and into the Logger text file

    sprintf(g_String, "[%05d] %s: %s\n", clock(), lvlText, text);
    printf(g_String);
    AddMsgIntoLogStorage(g_String, type);

    if (s_pLogFile)
        fprintf(s_pLogFile, g_String);
}

///////////////////////////////////////////////////////////

const char* PrepareMsg(const char* msg, const std::source_location& loc)
{
    // prepare a message for logger and put it into the buffer (s_TmpStr)

    char pathFromProjRoot[128]{ '\0' };
    GetPathFromProjRoot(loc.file_name(), pathFromProjRoot);

    sprintf(s_TmpStr, "%s: %s() (line: %d): %s",
        pathFromProjRoot,                          // relative path to the caller file
        loc.function_name(),                       // a function name where we call this log-function
        loc.line(),                                // at what line
        msg);

    return s_TmpStr;
}

///////////////////////////////////////////////////////////

void PrintExceptionErrHelper(const LIB_Exception& e, const bool showMsgBox)
{
    // show a message box if we need
    if (showMsgBox)
        MessageBoxW(NULL, e.GetStrWide(), L"Error", MB_ICONERROR);

    // print an error msg into the console and log file
    printf("%s", RED);                                   // setup console color
    PrintHelper("ERROR: ", e.GetConstStr(), LOG_TYPE_ERROR);
    printf("%s", RESET);                                 // reset console color
}

} // namespace
