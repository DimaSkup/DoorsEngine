// =================================================================================
// Filename: Log.cpp
// There is a Log system source file
// =================================================================================
#include "Log.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <sstream>

#include "MemHelpers.h"
#include "StringHelper.h"

#pragma warning (disable : 4996)

namespace ECS
{
	
Log* Log::pInstance_ = nullptr;
HANDLE Log::handle_ = GetStdHandle(STD_OUTPUT_HANDLE);
FILE* Log::pFile_ = nullptr;
std::list<std::string>* Log::pMsgsList_ = nullptr;

///////////////////////////////////////////////////////////

Log::Log()
{
	if (!pInstance_) // we can have only one instance of Logger
	{
		pInstance_ = this;
		printf("Log::Log(): the Log system is created successfully\n");
	}
	else
	{
		printf("Log::Log(): there is already one instance of the ECS::Log\n");
	}
}

///////////////////////////////////////////////////////////

Log::~Log()
{
	pFile_ = nullptr;
	pMsgsList_ = nullptr;

	printf("Log::~Log(): the Log system is destroyed\n");
}

///////////////////////////////////////////////////////////

void Log::Setup(FILE* pFile, std::list<std::string>* pMsgsList)
{
	// setup logger to make possible writing into the log file;

	if (!pFile)     { Log::Error("ptr to file == nullptr");	     return; }
	if (!pMsgsList) { Log::Error("ptr to msgs list == nullptr"); return; }

	pFile_ = pFile;
	pMsgsList_ = pMsgsList;
}


// =================================================================================
// 
//                             LOG PRINT METHODS
// 
// =================================================================================

void Log::Print(const std::string& msg, ConsoleColor attr)
{
	// prints a usual message and setup it wit passed particular console text attribute

	SetConsoleTextAttribute(Log::handle_, attr);
	PrintHelper(" ", msg);
	SetConsoleTextAttribute(Log::handle_, ConsoleColor::WHITE);  // reset
}

///////////////////////////////////////////////////////////

void Log::Print()
{
	// print empty string
	PrintHelper("", "");
}

///////////////////////////////////////////////////////////

void Log::Print(const std::string& msg, const std::source_location& location)
{
	// prints a usual message and the source location params as well
	SetConsoleTextAttribute(Log::handle_, ConsoleColor::GREEN);
	PrintHelper("", GenerateLogMsg(msg, location));
	SetConsoleTextAttribute(Log::handle_, ConsoleColor::WHITE);
}



// =================================================================================
//  
//                             LOG DEBUG METHODS
// 
// =================================================================================

void Log::Debug(const std::source_location& location)
{
	PrintHelper("DEBUG_ECS: ", GenerateLogMsg(" ", location));
}

///////////////////////////////////////////////////////////

void Log::Debug(const std::string& msg, const std::source_location& location)
{
	// prints a debug message
	Log::PrintHelper("DEBUG_ECS: ", GenerateLogMsg(msg, location));
}


// =================================================================================
// 
//                             LOG ERROR METHODS
// 
// =================================================================================


void Log::Error(LIB_Exception* pException, bool showMsgBox)
{
	// exception ERROR PRINTING (takes a pointer to the LIB_Exception)
	PrintExceptionErrHelper(*pException, showMsgBox);
}

///////////////////////////////////////////////////////////

void Log::Error(LIB_Exception& e, bool showMsgBox)
{
	// exception ERROR PRINTING (takes a reference to the LIB_Exception)
	PrintExceptionErrHelper(e, showMsgBox);
}

///////////////////////////////////////////////////////////

void Log::Error(const std::string& msg, const std::source_location& location)
{
	SetConsoleTextAttribute(Log::handle_, ConsoleColor::RED);
	PrintHelper("ERROR_ECS: ", GenerateLogMsg(msg, location));
	SetConsoleTextAttribute(Log::handle_, ConsoleColor::WHITE);
}


// =================================================================================
// 
//                         PRIVATE METHODS (HELPERS)
// 
// =================================================================================


void Log::PrintExceptionErrHelper(LIB_Exception& e, bool showMsgBox)
{
	// show a message box if we need
	if (showMsgBox)
		MessageBoxW(NULL, e.GetStrWide().c_str(), L"ECS Error", MB_ICONERROR);

	// print an error msg into the console and log file
	SetConsoleTextAttribute(Log::handle_, ConsoleColor::RED);
	PrintHelper("ERROR_ECS: ", e.GetStr());
	SetConsoleTextAttribute(Log::handle_, ConsoleColor::WHITE);
}

///////////////////////////////////////////////////////////

void Log::PrintHelper(const char* levtext, const std::string& text)
{
	// a helper for printing messages into the command prompt
	// and into the Logger text file (if we have a ptr to it)

	clock_t cl = clock();
	char* buffer = nullptr;
	int size = snprintf(nullptr, 0, "[%05d]\t%s%s\n", cl, levtext, text.c_str());

	buffer = new char[size + 1]{ '\0' };   // +1 because of '\0'

	sprintf(buffer, "[%05d]\t%s%s\n", cl, levtext, text.c_str());
	printf(buffer);

	if (pFile_)
		fwrite(buffer, sizeof(char), size, pFile_);

	if (pMsgsList_)
		pMsgsList_->push_back(std::string(buffer));

	SafeDeleteArr(buffer);
}

///////////////////////////////////////////////////////////

std::string Log::GenerateLogMsg(
	const std::string& msg,
	const std::source_location& location)
{
	std::stringstream ss;

	ss << StringHelper::GetPathFromProjRoot(location.file_name()) << ": "
		<< location.function_name() << "() (line:"
		<< location.line() << "): "
		<< msg;

	return ss.str();
}

};