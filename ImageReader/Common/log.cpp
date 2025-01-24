// =================================================================================
// Filename: Log.cpp
// There is a Log system source file
// =================================================================================
#include "Log.h"
#include <source_location>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <sstream>
#include <format>

#include "MemHelpers.h"
#include "StringHelper.h"

#pragma warning (disable : 4996)

namespace ImgReader
{

HANDLE Log::handle_ = GetStdHandle(STD_OUTPUT_HANDLE);
FILE* Log::pFile_ = nullptr;
std::list<std::string>* Log::pMsgsList_ = nullptr;

///////////////////////////////////////////////////////////

void Log::Setup(FILE* pFile, std::list<std::string>* pMsgsList)
{
	if (!pFile) { Log::Error("ptr to file == nullptr");	     return; }
	if (!pMsgsList) { Log::Error("ptr to msgs list == nullptr"); return; }

	pFile_ = pFile;
	pMsgsList_ = pMsgsList;
}



// =================================================================================
// 
//                             LOG PRINT METHODS
// 
// =================================================================================

void Log::Print()
{
	// print empty string
	PrintHelper("", "");
}

///////////////////////////////////////////////////////////

void Log::Print(const std::string& msg, ConsoleColor attr)
{
	// prints a usual message and setup it wit passed particular console text attribute

	SetConsoleTextAttribute(Log::handle_, attr);
	PrintHelper(" ", msg);
	SetConsoleTextAttribute(Log::handle_, ConsoleColor::WHITE);  // reset
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
	PrintHelper("DEBUG_IMG_READ: ", GenerateLogMsg(" ", location));
}

///////////////////////////////////////////////////////////

void Log::Debug(const std::string& msg, const std::source_location& location)
{
	// prints a debug message
	Log::PrintHelper("DEBUG_IMG_READ: ", GenerateLogMsg(msg, location));
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
	PrintHelper("ERROR_IMG_READ: ", GenerateLogMsg(msg, location));
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
		MessageBoxW(NULL, e.GetStrWide().c_str(), L"Error in the Image Reader", MB_ICONERROR);

	// print an error msg into the console and log file
	SetConsoleTextAttribute(Log::handle_, ConsoleColor::RED);
	PrintHelper("ERROR_IMG_READ: ", e.GetStr());
	SetConsoleTextAttribute(Log::handle_, ConsoleColor::WHITE);
}

///////////////////////////////////////////////////////////

void Log::PrintHelper(const char* levtext, const std::string& text)
{
	// a helper for printing messages into the command prompt
	// and into the Logger text file

	std::string str = std::format("[{:0>5d}]\t{}{}\n", clock(), levtext, text.c_str());

	printf(str.c_str());

	if (pFile_)
		fwrite(str.c_str(), sizeof(char), str.length(), pFile_);

	// write into log msgs list (if we can)
	if (pMsgsList_)
		pMsgsList_->push_back(str);
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

} // namespace ImgReader