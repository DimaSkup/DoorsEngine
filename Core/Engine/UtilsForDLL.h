////////////////////////////////////////////////////////////////////
// Filename:     UtilsForDLL.h
// Description:  contains different utils for work with
//               dynamic link libraries (DLL)
//
// Created:      01.08.23
////////////////////////////////////////////////////////////////////
#pragma once


#include "../Common/log.h"
#include "../Common/StringHelper.h"
#include <windows.h>


//////////////////////////////////
// TYPEDEFS
//////////////////////////////////

// is needed for calling DLL ModelConverter's import function; here we store a pointer to the DLL's process;
typedef VOID(*DLLPROC) (const char* inputDataFile, const char* outputDataFile);

//////////////////////////////////
// Class name: UtilsForDLL
//////////////////////////////////
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
