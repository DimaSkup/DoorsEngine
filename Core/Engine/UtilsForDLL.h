////////////////////////////////////////////////////////////////////
// Filename:     UtilsForDLL.h
// Description:  contains different utils for work with
//               dynamic link libraries (DLL)
//
// Created:      01.08.23
////////////////////////////////////////////////////////////////////
#pragma once

#include <windows.h>

namespace Core
{

// is needed for calling DLL ModelConverter's import function; here we store a pointer to the DLL's process;
typedef VOID(*DLLPROC) (const char* inputDataFile, const char* outputDataFile);


class UtilsForDLL
{
public:
	UtilsForDLL(const wchar_t* dllName);
	~UtilsForDLL();

	DLLPROC GetProcAddrFromDLL(const char* funcName);

private:
	HINSTANCE hinstDLL = NULL;
	DLLPROC procAddr = nullptr;
};

}