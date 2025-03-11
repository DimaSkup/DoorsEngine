#pragma once

#include <Windows.h>
#include <DbgHelp.h>
#include <stdio.h>
#pragma comment(lib, "DbgHelp.lib")

LONG WINAPI UnhandleExceptionFilter(_EXCEPTION_POINTERS* ExceptionInfo)
{
    SYSTEMTIME time;
    GetLocalTime(&time);

    char executableName[256];
    GetModuleFileNameA(NULL, executableName, 256);

    char minidumpPath[256];
    snprintf(minidumpPath, sizeof(minidumpPath), "%s_%i-%i-%i_%i-%i-%i-%i.dmp",
        executableName, time.wYear, time.wMonth, time.wDay,
        time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);

    HANDLE minidumpFile = CreateFileA(minidumpPath, GENERIC_WRITE,
        0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (minidumpFile != INVALID_HANDLE_VALUE)
    {
        MINIDUMP_EXCEPTION_INFORMATION minidumpInfo;
        minidumpInfo.ThreadId = GetCurrentThreadId();
        minidumpInfo.ExceptionPointers = ExceptionInfo;
        minidumpInfo.ClientPointers = FALSE;

        DWORD minidumpType =
            MiniDumpNormal |
            MiniDumpFilterMemory |
            MiniDumpScanMemory |
            MiniDumpWithDataSegs |
            MiniDumpWithIndirectlyReferencedMemory;

        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), minidumpFile,
            (MINIDUMP_TYPE)minidumpType, &minidumpInfo, 0, 0);

        CloseHandle(minidumpFile);
    }

    return EXCEPTION_CONTINUE_SEARCH;
}