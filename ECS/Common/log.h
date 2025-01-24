// =================================================================================
// Filename:    ECS::Log.h
// Description: there is a ECS::Log system header
// =================================================================================
#pragma once

#include "LIB_Exception.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>
#include <list>

namespace ECS
{

enum ConsoleColor
{
	// text color with a black background
	GREEN = 0x000A,
	WHITE = 0x0007,
	RED   = 0x0004,
};

class Log
{
public:
	Log();
	~Log();

	// returns a pointer to the instance of the Log class
	inline static Log* Get() { return pInstance_; }; 

	static void Setup(FILE* pFile, std::list<std::string>* pMsgsList);

	static void Print();
	static void Print(const std::string& msg, ConsoleColor attr);
	static void Print(const std::string& msg, const std::source_location& location = std::source_location::current());

	static void Debug(const std::source_location& location = std::source_location::current());
	static void Debug(const std::string& msg, const std::source_location& location = std::source_location::current());

	static void Error(const std::string& msg, const std::source_location& location = std::source_location::current());
	static void Error(LIB_Exception* pException, bool showMsgBox = false);
	static void Error(LIB_Exception& exception, bool showMsgBox = false);

private:
	static std::string GenerateLogMsg(const std::string& msg, const std::source_location& location);

	static void PrintExceptionErrHelper(LIB_Exception& LIB_Exception, bool showMsgBox);  // a Common handler for error printing
	static void PrintHelper(const char* levtext, const std::string& text);  // a helper for printing messages into the command prompt and into the ECS::Logger text file
	

private:
	static HANDLE handle_;                       // we need it for changing the text colour in the command prompt
	static Log*   pInstance_;
	static FILE*  pFile_;                        // a ptr to the global logger file handler
	static std::list<std::string>* pMsgsList_;   // a ptr to the global log messages list
};


}; // namespace ECS