// ================================================================================
// Filename:     StrHelper.h
// Description:  - functional for strings convertation;
// 
// Created:      
// ================================================================================
#pragma once

#include "log.h"
#include <string.h>
#pragma warning (disable : 4996)



class StrHelper
{
  
public:
    //-----------------------------------------------------
    // Return true if input string is empty
    //-----------------------------------------------------
    inline static bool IsEmpty(const char* str)     { return ((str == nullptr) || (str[0] == '\0')); }

    //-----------------------------------------------------
    // Return true if input string is empty
    //-----------------------------------------------------
    inline static bool IsEmpty(const wchar_t* wstr) { return ((wstr == nullptr) || (wstr[0] == L'\0')); }


    //-----------------------------------------------------
    // Desc:  convert input char string into wide string
    // Args:  - str:      input source string of chars
    //        - outWStr:  output string of wide chars
    //-----------------------------------------------------
    inline static void StrToWide(const char* str, wchar_t* outWStr)
    {
        if (outWStr == nullptr)
        {
            LogErr("in-out wide string == nullptr: so we cannot convert str => wstr");
            return;
        }

        if (IsEmpty(str))
        {
            LogErr("input string is empty");
            outWStr[0] = L'\0';
            return;
        }

        const size_t sz = strlen(str) + 1;
        mbstowcs(outWStr, str, sz);
    }

    //-----------------------------------------------------
     // Desc:  convert input wide string into char string
     // Args:  - wstr:    input source string of wide chars
     //        - outStr:  output string of chars
     //-----------------------------------------------------
    static void ToStr(const wchar_t* wstr, char* outStr)
    {
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
        size_t len = std::wcstombs(nullptr, wstr, 0) + 1;

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
            LogErr(LOG, "didn't manage to convert wstr => str");
            outStr[0] = '\0';
            return;
        }
    }

};
