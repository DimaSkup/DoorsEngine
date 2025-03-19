// =================================================================================
// Filename:    Log.h
// Description: there is a log system header
// =================================================================================
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>
#include <source_location>
#include "EngineException.h"

#include <list>

namespace Core
{

enum eConsoleColor
{
    // text color with a black background

    DEFAULT = 0x0007,

    RED     = 0x0004,
    WHITE   = 0x0007,
    GREEN   = 0x000A,
    YELLOW  = 0x000E,
};

///////////////////////////////////////////////////////////

class Log
{
public:
    Log();
    ~Log();

    // returns a pointer to the instance of the Log class
    inline static Log* Get() { return pInstance_; };

    inline static FILE* GetFilePtr() { return pFile_; }
    inline static std::list<std::string>& GetLogMsgsList() { return msgsList_; }

    // setup console color
    inline static void ResetConsoleColor()                        { SetConsoleTextAttribute(Log::handle_, eConsoleColor::DEFAULT); }
    inline static void SetConsoleColor(const eConsoleColor color) { SetConsoleTextAttribute(Log::handle_, color); }


    static void Print();
    static void Print(const std::string& msg, eConsoleColor color);
    static void Debug(const std::source_location& location = std::source_location::current());

    // print input message into console/log-file
    static void Print(const std::string& msg, const std::source_location& location = std::source_location::current());
    static void Debug(const std::string& msg, const std::source_location& location = std::source_location::current());
    static void Error(const std::string& msg, const std::source_location& location = std::source_location::current());

    // print input message of some fixed format into console/log-file
    static void Printf(const std::source_location& location, const char* format, ...);
    static void Debugf(const std::source_location& location, const char* format, ...);
    static void Errorf(const std::source_location& location, const char* format, ...);

    static void Printf(const char* format, ...);
    static void Debugf(const char* format, ...);
    static void Errorf(const char* format, ...);

    // exception handlers
    static void Error(EngineException* pException, bool showMsgBox = false);
    static void Error(EngineException& exception, bool showMsgBox = false);

private:
    bool InitHelper(); 
    void CloseHelper();

    static std::string GenerateLogMsg(const std::string& msg, const std::source_location& location);

    static void PrintExceptionErrHelper(EngineException& e, bool showMsgBox);  
    static void PrintHelper(const char* levtext, const std::string& text);     

private:
    static HANDLE handle_;    // we need it for changing the text colour in the command prompt
    static FILE* pFile_;      // a pointer to the Logger file handler
    static Log* pInstance_;

    static std::list<std::string> msgsList_;
};

} // namespace Core
