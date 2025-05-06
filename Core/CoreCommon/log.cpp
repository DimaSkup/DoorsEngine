// =================================================================================
// Filename: Log.cpp
// =================================================================================
#include "Log.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#pragma warning (disable : 4996)


namespace Core
{

char g_String[g_StrLim]{ '\0' };                 // global buffer for characters
char s_TmpStr[g_StrLim]{ '\0' };                 // static buffer for internal using

static LogStorage s_LogStorage;
static FILE*      s_pLogFile = nullptr;     // a static descriptor of the log file 

// helpers prototypes
void        GetPathFromProjRoot(const char* fullPath, char* outPath);
void        PrintHelper(const char* lvlText, const char* text, const LogType type);
void        PrintExceptionErrHelper(const EngineException& e, const bool showMsgBox);

const char* PrepareMsg(const char* msg, const std::source_location& loc);                                 // for C++20
const char* PrepareMsg(const char* msg, const char* fileName, const char* funcName, const int codeLine);  // for C or below C++20

const char* PrepareErrMsg(const char* msg, const std::source_location& loc);
const char* PrepareErrMsg(const char* msg, const char* fileName, const char* funcName, const int codeLine);

// =================================================================================

bool InitLogger(const char* logFileName)
{
    if (!logFileName || logFileName[0] == '\0')
    {
        LogErr("input name for the log file is empty!");
        return false;
    }

#if _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hConsole, &mode);
    SetConsoleMode(hConsole, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif

    if ((s_pLogFile = fopen(logFileName, "w")) != nullptr)
    {
        LogMsg("the log file is created successfully");

        char time[9];
        char date[9];

        _strtime(time);
        _strtime(date);

        if (s_pLogFile)
            fprintf(s_pLogFile, "%s : %s| the Log file is created\n", time, date);
        LogMsgf("-------------------------------------------\n\n");

        return true;
    }
    else
    {
        printf("%sInitLogger(): can't initialize the logger %s\n", RED, RESET);
        return false;
    }
}

///////////////////////////////////////////////////////////

void CloseLogger()
{
    // print message about closing of the log file and close it

    char time[9];
    char date[9];

    _strtime(time);
    _strdate(date);

    fprintf(s_pLogFile, "\n-------------------------------------------\n");
    fprintf(s_pLogFile, "%s : %s| the end of the Log file\n", time, date);

    fflush(s_pLogFile);
    fclose(s_pLogFile);
}

///////////////////////////////////////////////////////////

void AddMsgIntoLogStorage(const char* msg, const LogType type)
{
    // add a new message into the log storage 

    if (!msg || msg[0] == '\0')
        return;

    int& numLogs = s_LogStorage.numLogs;
    LogMessage& log = s_LogStorage.logs[numLogs];
    char** buf = &log.msg;                          // get a buffer to the text of this log
    log.type = type;                                // store the type of this log
    
    *buf = new char[strlen(msg) + 1]{ '\0' };
    strcpy(*buf, msg);
    
    ++numLogs;
}

///////////////////////////////////////////////////////////

FILE*             GetLogFile()    { return s_pLogFile; }
const LogStorage* GetLogStorage() { return &s_LogStorage; }


// =================================================================================
// logger functions which prints info about its caller (file, function, line)
// =================================================================================
void LogMsg(const char* msg, const std::source_location& loc)
{
    printf("%s", GREEN);                                // setup console color
    const char* buf = PrepareMsg(msg, loc);
    PrintHelper("", buf, LOG_TYPE_MESSAGE);
    printf("%s", RESET);                                // reset console color
}

///////////////////////////////////////////////////////////

void LogDbg(const char* msg, const std::source_location& loc)
{
    const char* buf = PrepareMsg(msg, loc);
    PrintHelper("DEBUG", buf, LOG_TYPE_DEBUG);
}

///////////////////////////////////////////////////////////

void LogErr(const char* msg, const std::source_location& loc)
{
    printf("%s", RED);                                  // setup console color
    const char* buf = PrepareErrMsg(msg, loc);
    PrintHelper("ERROR", buf, LOG_TYPE_ERROR);
    printf("%s", RESET);                                // reset console color
}

///////////////////////////////////////////////////////////

void LogMsg(const char* fileName, const char* funcName, const int codeLine, const char* msg)
{
    printf("%s", GREEN);                                // setup console color
    const char* buf = PrepareMsg(msg, fileName, funcName, codeLine);
    PrintHelper("", buf, LOG_TYPE_MESSAGE);
    printf("%s", RESET);                                // reset console color
}

///////////////////////////////////////////////////////////

void LogDbg(const char* fileName, const char* funcName, const int codeLine, const char* msg)
{
    const char* buf = PrepareMsg(msg, fileName, funcName, codeLine);
    PrintHelper("DEBUG", buf, LOG_TYPE_DEBUG);
}

///////////////////////////////////////////////////////////

void LogErr(const char* fileName, const char* funcName, const int codeLine, const char* msg)
{
    printf("%s", RED);                                  // setup console color
    const char* buf = PrepareErrMsg(msg, fileName, funcName, codeLine);
    PrintHelper("ERROR", buf, LOG_TYPE_ERROR);
    printf("%s", RESET);                                // reset console color
}


// =================================================================================
// logger function with variadic arguments
// =================================================================================
void LogMsgf(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    vsnprintf(s_TmpStr, g_StrLim, format, args);
    PrintHelper("", s_TmpStr, LOG_TYPE_FORMATTED);
    printf("%s", RESET);                  // reset console color

    va_end(args);
}


// =================================================================================
// exception handlers (in case if using C++)
// =================================================================================
void LogErr(const EngineException* pException, bool showMsgBox)
{
    // exception ERROR PRINTING (takes a pointer to the EngineException)
    PrintExceptionErrHelper(*pException, showMsgBox);
}

///////////////////////////////////////////////////////////

void LogErr(const EngineException& e, const bool showMsgBox)
{
    // exception ERROR PRINTING (takes a reference to the EngineException)
    PrintExceptionErrHelper(e, showMsgBox);
}


// =================================================================================
// Private Helpers
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

    snprintf(g_String, g_StrLim, "[%05d] %s: %s\n", clock(), lvlText, text);
    printf(g_String);
    AddMsgIntoLogStorage(g_String, type);

    if (s_pLogFile)
        fprintf(s_pLogFile, g_String);
}

///////////////////////////////////////////////////////////

const char* PrepareMsg(const char* msg, const std::source_location& loc)
{
    // prepare a message for logger and put it into the global buffer (g_String)

    char pathFromProjRoot[128]{ '\0' };
    GetPathFromProjRoot(loc.file_name(), pathFromProjRoot);

    snprintf(s_TmpStr, g_StrLim, "%s: %s() (line: %d): %s",
        pathFromProjRoot,                           // relative path to the caller file
        loc.function_name(),                        // a function name where we called this log-function
        loc.line(),                                 // at what line
        msg);

    return s_TmpStr;
}

///////////////////////////////////////////////////////////

const char* PrepareMsg(
    const char* msg,
    const char* fileName,
    const char* funcName,
    const int codeLine)
{
    char pathFromProjRoot[128]{ '\0' };
    GetPathFromProjRoot(fileName, pathFromProjRoot);

    snprintf(s_TmpStr, g_StrLim, "%s: %s() (line: %d): %s",
        pathFromProjRoot,                               // relative path to the caller file
        funcName,                                       // a function name where we called this log-function
        codeLine,                                       // at what line
        msg);

    return s_TmpStr;
}

///////////////////////////////////////////////////////////

const char* PrepareErrMsg(const char* msg, const std::source_location& loc)
{
    // prepare error message to be printed in specific format

    char pathFromProjRoot[128]{ '\0' };
    GetPathFromProjRoot(loc.file_name(), pathFromProjRoot);

    snprintf(
        s_TmpStr,
        g_StrLim,
        "\nFILE:  %s\n"
        "FUNC:  %s()\n"
        "LINE:  %d\n"
        "MSG:   %s\n",
        pathFromProjRoot,                               // relative path to the caller file
        loc.function_name(),                            // a function name where we called this log-function
        loc.line(),                                     // at what line
        msg);

    return s_TmpStr;
}

///////////////////////////////////////////////////////////

const char* PrepareErrMsg(const char* msg, const char* fileName, const char* funcName, const int codeLine)
{
    // prepare error message to be printed in specific format

    char pathFromProjRoot[128]{ '\0' };
    GetPathFromProjRoot(fileName, pathFromProjRoot);

    snprintf(
        s_TmpStr,
        g_StrLim,
        "\nFILE:  %s\n"
        "FUNC:  %s()\n"
        "LINE:  %d\n"
        "MSG:   %s\n",
        pathFromProjRoot,                               // relative path to the caller file
        funcName,                                       // a function name where we called this log-function
        codeLine,                                       // at what line
        msg);

    return s_TmpStr;
}

///////////////////////////////////////////////////////////

void PrintExceptionErrHelper(const EngineException& e, const bool showMsgBox)
{
    // show a message box if we need
    if (showMsgBox)
        MessageBoxA(NULL, e.GetConstStr(), "Error", MB_ICONERROR);

    // print an error msg into the console and log file
    printf("%s", RED);                                   // setup console color
    PrintHelper("ERROR: ", e.GetConstStr(), LOG_TYPE_ERROR);
    printf("%s", RESET);                                 // reset console color
}
} // namespace Core
