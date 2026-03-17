// *********************************************************************************
// Filename:     file_system.h
// Description:  utils for working with files paths
// 
// Created:      17.04.2025 by DimaSkup
// *********************************************************************************
#pragma once


class FileSys
{
public:

    static bool Exists(const char* filePath);
    static void GetPathFromProjRoot(const char* fullPath, char* outPath);

    static bool IsPathSupported(const char* path);
    static bool IsPathSupported(const wchar_t* path);

    static int GetLastSlashOffset(const char* path);

    static void GetParentPath(const char* path, char* outPath);
    static void GetFileName  (const char* path, char* outFilename);
    static void GetFileStem  (const char* path, char* outStem);
    static void GetFileExt   (const char* path, char* outExt);

private:
    static bool IsEmpty(const char* str);
    static bool IsEmpty(const wchar_t* wstr);
};


//==================================================================================
// inline functions
//==================================================================================

inline bool FileSys::IsEmpty(const char* str)
{
    return ((str == nullptr) || (str[0] == '\0'));
}

inline bool FileSys::IsEmpty(const wchar_t* wstr)
{
    return ((wstr == nullptr) || (wstr[0] == L'\0'));
}
