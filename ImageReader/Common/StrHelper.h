// ================================================================================
// Filename:     StrHelper.h
// Description:  functional for strings convertation;
// 
// Created:      17.04.2025 by DimaSkup
// ================================================================================
#pragma once

#include "log.h"
#include <string.h>
#pragma warning (disable : 4996)


namespace ImgReader
{

class StrHelper
{
  
public:
    inline static bool IsEmpty(const char* str)     { return ((str == nullptr) || (str[0] == '\0')); }
    inline static bool IsEmpty(const wchar_t* wstr) { return ((wstr == nullptr) || (wstr[0] == L'\0')); }

    ///////////////////////////////////////////////////////////

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
            LogErr("input string is empty");
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

}; // class StrHelper

} // namespace
