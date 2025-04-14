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

// global buffer for characters
char g_String[256]{ '\0' };
char s_TmpStr[256]{ '\0' };

// a static descriptor of the log file 
static FILE* s_pLogFile = nullptr;

// helpers prototypes
void GetPathFromProjRoot(const char* fullPath, char* outPath);
void PrintHelper(const char* lvlText, const char* text);
void PrepareMsg(const char* msg, const std::source_location& loc);
void PrintExceptionErrHelper(const Core::EngineException& e, const bool showMsgBox);

// =================================================================================

bool InitLogger()
{
    if ((s_pLogFile = fopen("DoorsEngineLog.txt", "w")) != nullptr)
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
    PrepareMsg(msg, loc);
    PrintHelper("", s_TmpStr);
    printf("%s", RESET);                                // reset console color
}

///////////////////////////////////////////////////////////

void LogDbg(const char* msg, const std::source_location& loc)
{
    PrepareMsg(msg, loc);
    PrintHelper("DEBUG: ", s_TmpStr);
}

///////////////////////////////////////////////////////////

void LogErr(const char* msg, const std::source_location& loc)
{
    printf("%s", RED);                                // setup console color
    PrepareMsg(msg, loc);
    PrintHelper("ERROR: ", s_TmpStr);
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
    PrintHelper("", s_TmpStr);
    printf("%s", RESET);                  // reset console color

    va_end(args);
}


// =================================================================================
// exception handlers
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
// Helpers
// =================================================================================
void GetPathFromProjRoot(const char* fullPath, char* outPath)
{
    // return relative path from the project root

    if ((fullPath == nullptr) || (fullPath[0] == '\0'))
    {
        LogErr("input path is empty!");
        return;
    }

    if (outPath == nullptr)
    {
        LogErr("in-out path == nullptr");
        return;
    }

    const char* found = strstr(fullPath, "DoorsEngine\\");

    // if we found the substring
    if (found != nullptr)
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

    printf("[%05d] %s: %s\n", clock(), lvlText, text);

    if (s_pLogFile)
        fprintf(s_pLogFile, "[%05d] %s: %s\n", clock(), lvlText, text);
}

///////////////////////////////////////////////////////////

void PrepareMsg(const char* msg, const std::source_location& loc)
{
    // prepare a message for logger and put it into the global buffer (g_String)

    char pathFromProjRoot[128]{ '\0' };
    GetPathFromProjRoot(loc.file_name(), pathFromProjRoot);

    sprintf(s_TmpStr, "%s: %s() (line: %d): %s",
        pathFromProjRoot,                               // relative path to the caller file
        loc.function_name(),                            // a function name where we call this log-function
        loc.line(),                                     // at what line
        msg);
}

///////////////////////////////////////////////////////////

void PrintExceptionErrHelper(const Core::EngineException& e, const bool showMsgBox)
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
