// =================================================================================
// Filename:    Log.h
// Description: there is a log system header
// =================================================================================
#pragma once

#include "LIB_Exception.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>
#include <source_location>
#include <list>


namespace ImgReader
{

enum ConsoleColor
{
	// text color with a black background
	GREEN = 0x000A,
	WHITE = 0x0007,
	RED = 0x0004,
	YELLOW = 0x000E,
};

class Log
{
private:
	Log();
	~Log();

public:
	static void Setup(FILE* pFile, std::list<std::string>* pMsgsList);

	static void Print();
	static void Print(const std::string& msg, ConsoleColor attr);
	static void Print(const std::string& msg, const std::source_location& location = std::source_location::current());

	static void Debug(const std::source_location& location = std::source_location::current());
	static void Debug(const std::string& msg, const std::source_location& location = std::source_location::current());

	static void Error(const std::string& msg, const std::source_location& location = std::source_location::current());
	static void Error(LIB_Exception* pException, bool showMsgBox = false);
	static void Error(LIB_Exception& exception,  bool showMsgBox = false);

private:
	static std::string GenerateLogMsg(const std::string& msg, const std::source_location& location);

	static void PrintExceptionErrHelper(LIB_Exception& e, bool showMsgBox); 
	static void PrintHelper(const char* levtext, const std::string& text);  


private:
	static HANDLE handle_;                       // we need it for changing the text colour in the command prompt

	static FILE* pFile_;                         // a ptr to the global logger file handler
	static std::list<std::string>* pMsgsList_;   // a ptr to the global log messages list

};

} // namespace ImgReader