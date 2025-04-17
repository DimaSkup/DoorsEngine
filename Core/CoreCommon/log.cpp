// =================================================================================
// Filename: Log.cpp
// =================================================================================
#include "Log.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <cstdarg>

#pragma warning (disable : 4996)


namespace Core
{

char g_String[256]{ '\0' };             // global buffer for characters
char s_TmpStr[256]{ '\0' };             // static buffer for internal using

static FILE* s_pLogFile = nullptr;      // a static descriptor of the log file 

// helpers prototypes
void        GetPathFromProjRoot(const char* fullPath, char* outPath);
void        PrintHelper(const char* lvlText, const char* text);
void        PrintExceptionErrHelper(const EngineException& e, const bool showMsgBox);

const char* PrepareMsg(const char* msg, const std::source_location& loc);                                 // for C++20
const char* PrepareMsg(const char* msg, const char* fileName, const char* funcName, const int codeLine);  // for C or below C++20

const char* PrepareErrMsg(const char* msg, const std::source_location& loc);
const char* PrepareErrMsg(const char* msg, const char* fileName, const char* funcName, const int codeLine);

// =================================================================================

bool InitLogger()
{
#if _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hConsole, &mode);
    SetConsoleMode(hConsole, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif

    const char* logFileName = "DoorsEngineLog.txt";


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


// =================================================================================
// logger functions which prints info about its caller (file, function, line)
// =================================================================================
void LogMsg(const char* msg, const std::source_location& loc)
{
    printf("%s", GREEN);                                // setup console color
    const char* buf = PrepareMsg(msg, loc);
    PrintHelper("", buf);
    printf("%s", RESET);                                // reset console color
}

///////////////////////////////////////////////////////////

void LogDbg(const char* msg, const std::source_location& loc)
{
    const char* buf = PrepareMsg(msg, loc);
    PrintHelper("DEBUG", buf);
}

///////////////////////////////////////////////////////////

void LogErr(const char* msg, const std::source_location& loc)
{
    printf("%s", RED);                                  // setup console color
    const char* buf = PrepareErrMsg(msg, loc);
    PrintHelper("ERROR", buf);
    printf("%s", RESET);                                // reset console color
}

///////////////////////////////////////////////////////////

void LogMsg(const char* fileName, const char* funcName, const int codeLine, const char* msg)
{
    printf("%s", GREEN);                                // setup console color
    const char* buf = PrepareMsg(msg, fileName, funcName, codeLine);
    PrintHelper("", buf);
    printf("%s", RESET);                                // reset console color
}

///////////////////////////////////////////////////////////

void LogDbg(const char* fileName, const char* funcName, const int codeLine, const char* msg)
{
    const char* buf = PrepareMsg(msg, fileName, funcName, codeLine);
    PrintHelper("DEBUG", buf);
}

///////////////////////////////////////////////////////////

void LogErr(const char* fileName, const char* funcName, const int codeLine, const char* msg)
{
    printf("%s", RED);                                  // setup console color
    const char* buf = PrepareErrMsg(msg, fileName, funcName, codeLine);
    PrintHelper("ERROR", buf);
    printf("%s", RESET);                                // reset console color
}


// =================================================================================
// logger function with variadic arguments
// =================================================================================
void LogMsgf(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    vsprintf(s_TmpStr, format, args);
    PrintHelper("", s_TmpStr);
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
    if (!found)
    {
        strcpy(outPath, found + strlen("DoorsEngine\\"));
    }
    else
    {
        outPath[0] = '\0';
    }
}

///////////////////////////////////////////////////////////

void PrintHelper(const char* lvlText, const char* text)
{
    // a helper for printing messages into the command prompt
    // and into the Logger text file

    const clock_t cl = clock();

    printf("[%05d] %s: %s\n", cl, lvlText, text);

    if (s_pLogFile)
        fprintf(s_pLogFile, "[%05d] %s: %s\n", cl, lvlText, text);
}

///////////////////////////////////////////////////////////

const char* PrepareMsg(const char* msg, const std::source_location& loc)
{
    // prepare a message for logger and put it into the global buffer (g_String)

    char pathFromProjRoot[128]{ '\0' };
    GetPathFromProjRoot(loc.file_name(), pathFromProjRoot);

    sprintf(s_TmpStr, "%s: %s() (line: %d): %s",
        pathFromProjRoot,                               // relative path to the caller file
        loc.function_name(),                            // a function name where we called this log-function
        loc.line(),                                     // at what line
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

    sprintf(s_TmpStr, "%s: %s() (line: %d): %s",
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

    sprintf(s_TmpStr,
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

const char* PrepareErrMsg(const char* msg, const char* fileName, const char* funcName, const int codeLine)
{
    // prepare error message to be printed in specific format

    char pathFromProjRoot[128]{ '\0' };
    GetPathFromProjRoot(fileName, pathFromProjRoot);

    sprintf(s_TmpStr,
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
        MessageBoxW(NULL, e.GetStrWide(), L"Error", MB_ICONERROR);

    // print an error msg into the console and log file
    printf("%s", RED);                                   // setup console color
    PrintHelper("ERROR: ", e.GetConstStr());
    printf("%s", RESET);                                 // reset console color
}
} // namespace Core
