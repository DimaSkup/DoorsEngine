// =================================================================================
// Filename: Log.cpp
// =================================================================================
#include "Log.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <cstdarg>
#include <string>

#pragma warning (disable : 4996)


namespace Render
{

// global buffer for characters
char g_String[256]{ '\0' };

// a static descriptor of the log file 
static FILE* s_pLogFile = nullptr;

// helpers prototypes
std::string GetPathFromProjRoot(const std::string& fullPath);
void PrintHelper(const char* lvlText, const char* text);
void PrepareMsg(const char* msg, const std::source_location& loc);
void PrintExceptionErrHelper(const LIB_Exception& e, const bool showMsgBox);

// =================================================================================

bool InitLogger()
{
    if ((s_pLogFile = fopen("DoorsEngineLog.txt", "w")) == 0)
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
        printf("%scan't initialize the logger %s\n", RED, RESET);
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
    PrintHelper("", g_String);
    printf("%s", RESET);                                // reset console color
}

///////////////////////////////////////////////////////////

void LogDbg(const char* msg, const std::source_location& loc)
{
    PrepareMsg(msg, loc);
    PrintHelper("DEBUG: ", g_String);
}

///////////////////////////////////////////////////////////

void LogErr(const char* msg, const std::source_location& loc)
{
    printf("%s", GREEN);                                // setup console color
    PrepareMsg(msg, loc);
    PrintHelper("ERROR: ", g_String);
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

    vsprintf(g_String, format, args);
    PrintHelper("", g_String);
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
std::string GetPathFromProjRoot(const std::string& fullPath)
{
    // return relative path from the project root

    std::size_t found = fullPath.find("DoorsEngine\\");

    if (found != std::string::npos)
        return fullPath.substr(found + strlen("DoorsEngine\\"));

    return "invalid_path";
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

    sprintf(g_String, "%s: %s() (line: %d): %s\n",
        GetPathFromProjRoot(loc.file_name()).c_str(),   // relative path to the caller file
        loc.function_name(),                            // a function name where we call this log-function
        loc.line(),                                     // at what line
        msg);
}

///////////////////////////////////////////////////////////

void PrintExceptionErrHelper(const LIB_Exception& e, const bool showMsgBox)
{
    // show a message box if we need
    if (showMsgBox)
        MessageBoxW(NULL, e.GetStrWide(), L"Error", MB_ICONERROR);

    // print an error msg into the console and log file
    printf("%s", RED);                                   // setup console color
    PrintHelper("ERROR: ", e.GetConstStr());
    printf("%s", RESET);                                 // reset console color
}

} // namespace Render
