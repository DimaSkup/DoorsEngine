// ================================================================================
// Filename:     StringHelper.cpp
// Description:  functional for strings convertation, and other handling
// 
// Created:      
// ================================================================================
#include "StringHelper.h"

#include "MemHelpers.h"
#include "log.h"
#include <cstdlib>

#pragma warning(disable : 4996)


namespace Core
{

// ================================================================================
//                          PUBLIC STATIC METHODS
// ================================================================================

std::wstring StringHelper::StringToWide(const std::string& str)
{
	// dummy way to convert str => wstr
	
	if (str.empty())
		return L"";

	return std::wstring(str.begin(), str.end());
}

///////////////////////////////////////////////////////////

std::string StringHelper::ToString(const std::wstring& wstr)
{
	// convert std::wstring to std::string

	if (wstr.empty())
		return "";

	char* buff = nullptr;

	try
	{
		// calculating the length of the multibyte string
		std::size_t len = wcstombs(nullptr, wstr.c_str(), 0) + 1;

		if (len == 0)
			return "";

		// creating a buffer to hold the multibyte string
		buff = new char[len] { '\0' };
		
		// convert wstring to string
		size_t sz = std::wcstombs(buff, wstr.c_str(), len);

		// if we got some err we clear memory and return an empty line
		if (sz == (size_t)-1)
		{
			Log::Error("didn't manage to convert wstr => str");
			SafeDeleteArr(buff);
			return "";
		}

		// create std::string from char buffer
		std::string temp = buff;

		// clean up the buffer
		SafeDeleteArr(buff);

		return temp;
	}
	catch (const std::bad_alloc& e)
	{
		SafeDeleteArr(buff);

		Log::Error(e.what());
		Log::Error("can't allocate memory during convertation wstr => str");

		return "";
	}
}

///////////////////////////////////////////////////////////

std::string StringHelper::GetPathFromProjRoot(const std::string& fullPath)
{
	// return relative path from the project root

	std::size_t found = fullPath.find("DoorsEngine\\");

	if (found != std::string::npos)
		return fullPath.substr(found + strlen("DoorsEngine\\"));   
	
	return "invalid_path";
}

} // namespace Core