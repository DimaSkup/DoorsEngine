// ================================================================================
// Filename:     StrHelper.h
// Description:  functional for work with strings
// 
// Created:      17.04.2025 by DimaSkup
// ================================================================================
#pragma once

namespace Render
{

class StrHelper
{
public:
    inline static bool IsEmpty(const char* str)     { return ((str == nullptr) || (str[0] == '\0')); }
    inline static bool IsEmpty(const wchar_t* wstr) { return ((wstr == nullptr) || (wstr[0] == L'\0')); }

    static void StrToWide(const char* str, wchar_t* outWStr);
    static void ToStr(const wchar_t* wstr, char* outStr);
};

} // namespace
