// ================================================================================
// Filename:     StrHelper.h
// Description:  - functional for strings convertation;
//               - functional for work with paths (get parent path, filename, etc.)
// 
// Created:      
// ================================================================================
#pragma once

#include "log.h"
#include <string.h>
#pragma warning (disable : 4996)


namespace Core
{

class StrHelper
{
  
public:
    inline static bool IsEmpty(const char* str)     { return ((str == nullptr) || (str[0] == '\0')); }
    inline static bool IsEmpty(const wchar_t* wstr) { return ((wstr == nullptr) || (wstr[0] == L'\0')); }


    inline static void StrToWide(const char* str, wchar_t* outWStr)
    {
        // dummy way (or not?) to convert str => wstr

        if (outWStr == nullptr)
        {
            LogErr("in-out wide string == nullptr: so we cannot convert str => wstr");
            return;
        }

        if (IsEmpty(str))
        {
            outWStr[0] = L'\0';
            return;
        }

        const size_t sz = strlen(str) + 1;
        mbstowcs(outWStr, str, sz);
    }

    ///////////////////////////////////////////////////////////

    static void ToStr(const wchar_t* wstr, char* outStr)
    {
        // convert std::wstring to std::string

        if (outStr == nullptr)
        {
            LogErr("in-out string == nullptr: so we cannot convert wstr => str");
            return;
        }

        if (IsEmpty(wstr))
        {
            outStr[0] = '\0';
            return;
        }

        // calculating the length of the multibyte string
        size_t len = wcstombs(nullptr, wstr, 0) + 1;

        if (len == 0)
        {
            outStr[0] = '\0';
            return;
        }

        // convert wstring to string
        size_t sz = std::wcstombs(outStr, wstr, len);

        // if we got some err
        if (sz == (size_t)-1)
        {
            LogErr("didn't manage to convert wstr => str");
            outStr[0] = '\0';
            return;
        }
    }

    ///////////////////////////////////////////////////////////

    inline static void GetPathFromProjRoot(const char* fullPath, char* outPath)
    {
        // return relative path from the project root

        if (IsEmpty(fullPath))
        {
            LogErr("input path is empty!");
            return;
        }

        if (outPath == nullptr)
        {
            LogErr("in-out path == nullptr");
            return;
        }

        const char* found = strstr(fullPath, "DoorsEngine\\");

        // if we found the substring
        if (found != nullptr)
        {
            strcpy(outPath, found + strlen("DoorsEngine\\"));
        }
        else
        {
            outPath[0] = '\0';
        }
    }

    ///////////////////////////////////////////////////////////

    inline static void GetParentPath(const char* path, char* outPath)
    {
        // returns the path of the parent path
        const char* lastSlash = strrchr(path, '\\') + 1;
        const ptrdiff_t pathLen = lastSlash - path;

        strncpy(outPath, path, pathLen);
        outPath[pathLen] = '\0';                           // put extra NULL
    }

    ///////////////////////////////////////////////////////////

    inline static void GetFileName(const char* path, char* outFilename)
    {
        // returns the filename path component
        const char* lastSlash = strrchr(path, '\\') + 1;
        strcpy(outFilename, lastSlash);
        outFilename[lastSlash - path] = '\0';              // put extra NULL
    }

    ///////////////////////////////////////////////////////////

    inline static void GetFileStem(const char* path, char* outStem)
    {
        // returns the stem path component (filename without the final extension)
        const char* lastSlash = strrchr(path, '\\') + 1;
        const char* dotPos = strrchr(path, '.');
        const ptrdiff_t stemLen = dotPos - lastSlash;

        strncpy(outStem, lastSlash, stemLen);
        outStem[stemLen] = '\0';                           // put extra NULL
    }

    ///////////////////////////////////////////////////////////

    inline static void GetFileExt(const char* path, char* outExt)
    {
        // returns the file extension path component
        const char* dotPos = strrchr(path, '.');
        strcpy(outExt, dotPos);
        outExt[strlen(dotPos)] = '\0';                     // put extra NULL
    }
};

}
