#include "StringHelper.h"
#include "log.h"

#pragma warning (disable : 4996) // because we use the wcsrtombs function


namespace ECS
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

	char buf[128]{'\0'};

	// calculating the length of the multibyte string
	const std::size_t len = wcstombs(nullptr, wstr.c_str(), 0) + 1;

	// convert wstring to string
	const size_t sz = std::wcstombs(buf, wstr.c_str(), len);

	// if we got some err we clear memory and return an empty line
	if (sz == (size_t)-1)
	{
		LogErr("didn't manage to convert wstr => str");
		return "";
	}

	return std::string(buf);
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

}; // namespace ECS
