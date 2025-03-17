///////////////////////////////////////////////////////////////////////////////
// Filename: Log.cpp
// There is a Log system source file
///////////////////////////////////////////////////////////////////////////////
#include "Log.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <sstream>
#include <format>
#include <cstdarg>

#include "StringHelper.h"

#pragma warning (disable : 4996)


namespace Core
{

Log* Log::pInstance_ = nullptr;
HANDLE Log::handle_ = GetStdHandle(STD_OUTPUT_HANDLE);
FILE* Log::pFile_ = nullptr;

std::list<std::string> Log::msgsList_;

///////////////////////////////////////////////////////////

Log::Log()
{
	if (!pInstance_) // we can have only one instance of Logger
	{
		if (!InitHelper())
		{
			SetConsoleTextAttribute(Log::handle_, ConsoleColor::RED);
			printf("Log::Log(): can't initialize the logger");
			SetConsoleTextAttribute(Log::handle_, ConsoleColor::WHITE);
		}

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
	if (!pFile_) return;

	CloseHelper();
	fflush(pFile_);
	fclose(pFile_);

	printf("Log::~Log(): the Log system is destroyed\n");
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
	SetConsoleTextAttribute(Log::handle_, WHITE);  // reset
}

///////////////////////////////////////////////////////////

void Log::Print()
{
	// print an empty string
	PrintHelper("", "");
}

///////////////////////////////////////////////////////////

void Log::Print(const std::string& msg, const std::source_location& location)
{
	// prints a usual message and the source location params as well
	SetConsoleTextAttribute(Log::handle_, GREEN);
	PrintHelper("", GenerateLogMsg(msg, location));
	SetConsoleTextAttribute(Log::handle_, WHITE);
}



// =================================================================================
//  
//                             LOG DEBUG METHODS
// 
// =================================================================================

void Log::Debug(const std::source_location& location)
{
	PrintHelper("DEBUG: ", GenerateLogMsg(" ", location));
}

///////////////////////////////////////////////////////////

void Log::Debug(const std::string& msg, const std::source_location& location)
{
	Log::PrintHelper("DEBUG: ", GenerateLogMsg(msg, location));
}



// =================================================================================
// 
//                             LOG ERROR METHODS
// 
// =================================================================================

void Log::Error(EngineException* pException, bool showMsgBox)
{
	// exception ERROR PRINTING (takes a pointer to the EngineException)
	PrintExceptionErrHelper(*pException, showMsgBox);
}

///////////////////////////////////////////////////////////

void Log::Error(EngineException& e, bool showMsgBox)
{
	// exception ERROR PRINTING (takes a reference to the EngineException)
	PrintExceptionErrHelper(e, showMsgBox);
}

///////////////////////////////////////////////////////////

void Log::Error(const std::string& msg, const std::source_location& location)
{
	SetConsoleTextAttribute(Log::handle_, RED);
	PrintHelper("ERROR: ", GenerateLogMsg(msg, location));
	SetConsoleTextAttribute(Log::handle_, WHITE);
}

///////////////////////////////////////////////////////////

void Log::Error(const std::source_location& location, const char* format, ...)
{

    va_list args;
    va_start(args, format);

    char buffer[512]{ '\0' };
    int len = _vscprintf(format, args) + 1;

    vsprintf_s(buffer, len, format, args);

    SetConsoleTextAttribute(Log::handle_, RED);
    PrintHelper("ERROR: ", GenerateLogMsg(buffer, location));
    SetConsoleTextAttribute(Log::handle_, WHITE);

    va_end(args);
}


// =================================================================================
// 
//                         PRIVATE METHODS (HELPERS)
// 
// =================================================================================

void Log::PrintExceptionErrHelper(EngineException& e, bool showMsgBox)
{
	// show a message box if we need
	if (showMsgBox)
		MessageBoxW(NULL, e.GetWideStr().c_str(), L"Error", MB_ICONERROR);

	// print an error msg into the console and log file
	SetConsoleTextAttribute(Log::handle_, RED);
	PrintHelper("ERROR: ", e.GetStr());
	SetConsoleTextAttribute(Log::handle_, WHITE);
}

///////////////////////////////////////////////////////////

bool Log::InitHelper()
{
	//
	// this function creates and opens a Logger text file
	//

	if (fopen_s(&pFile_, "EngineCoreLog.txt", "w") == 0)
	{
		printf("Log::m_init(): the Log file is created successfully\n");

		char time[9];
		char date[9];

		_strtime_s(time, 9);
		_strdate_s(date, 9);

		fprintf(pFile_, "%s : %s| the Log file is created\n", time, date);
		fprintf(pFile_, "-------------------------------------------\n\n");
		return true;
	}
	else
	{
		printf("Log::m_init(): can't create the Log file\n");
		return false;
	}
}

///////////////////////////////////////////////////////////

void Log::CloseHelper()
{
	// print message about closing of the Logger file

	char time[9];
	char date[9];

	_strtime_s(time, 9);
	_strdate_s(date, 9);

	fprintf(pFile_, "\n-------------------------------------------\n");
	fprintf(pFile_, "%s : %s| the end of the Log file\n", time, date);
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

	msgsList_.push_back(str);

#if 0  // C-style

	char* buffer = nullptr;
	int size = snprintf(nullptr, 0, "[%05d]\t%s%s\n", clock(), levtext, text.c_str());

	buffer = new char[size + 1]{ '\0' };   // +1 because of '\0'

	sprintf(buffer, "[%05d]\t%s%s\n", clock(), levtext, text.c_str());
	printf(buffer);

	if (pFile_)
		fprintf(pFile_, buffer);

	msgsList_.push_back(std::string(buffer));
	SafeDeleteArr(buffer);

#endif
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

}
