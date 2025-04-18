// *********************************************************************************
// Filename:     Filesystem.h
// Description:  utils for working with files paths
// 
// Created:      18.04.2025 by DimaSkup
// *********************************************************************************
#pragma once

#include "log.h"
#include <stdio.h>
#include <string.h>


namespace Game
{

class FileSys
{
public:

    inline static bool Exists(const char* filePath)
    {
        // check if file by filePath (relatively to the working directory) exists

        if (IsEmpty(filePath))
        {
            LogErr("input path is empty!");
            return false;
        }

        FILE* pFile = nullptr;

        // check if such file exists
        if ((pFile = fopen(filePath, "r+")) == nullptr)
        {
            sprintf(g_String, "there is no texture by path: %s", filePath);
            LogErr(g_String);
            return false;
        }

        fclose(pFile);
        return true;
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

        if (!outPath)
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

    inline static ptrdiff_t GetLastSlashOffset(const char* path)
    {
        const char* offset1 = strrchr(path, '\\');
        const char* offset2 = strrchr(path, '/');

        if ((offset1 == NULL) && (offset2 == NULL))
        {
            // if no slash or backslash
            return 0;
        }
        else if (offset1 == NULL)
        {
            return offset2 - path;
        }
        else
        {
            return offset1 - path;
        }
    }

    ///////////////////////////////////////////////////////////

    inline static void GetParentPath(const char* path, char* outPath)
    {
        // returns the path of the parent path
        const char* lastSlash = path + GetLastSlashOffset(path);
        const ptrdiff_t pathLen = lastSlash - path + 1;

        strncpy(outPath, path, pathLen);
        outPath[pathLen] = '\0';                           // put extra NULL
    }

    ///////////////////////////////////////////////////////////

    inline static void GetFileName(const char* path, char* outFilename)
    {
        // returns the filename path component
        const char* lastSlash = path + GetLastSlashOffset(path) + 1;
        strcpy(outFilename, lastSlash);
        outFilename[lastSlash - path] = '\0';              // put extra NULL
    }

    ///////////////////////////////////////////////////////////

    inline static void GetFileStem(const char* path, char* outStem)
    {
        // returns the stem path component (filename without the final extension)

        // define where stem starts (what is the offset)
        const ptrdiff_t offset = GetLastSlashOffset(path);
        const char* stemStart = path + offset + 1;

        // define where is the dot
        const char* dotPos = strrchr(path, '.');
        ptrdiff_t stemLen = 0;

        if (dotPos)
            stemLen = dotPos - stemStart;
        else
            stemLen = strlen(path) - offset;

        // make the output string
        strncpy(outStem, stemStart, stemLen);
        outStem[stemLen] = '\0';                           // put extra NULL
    }

    ///////////////////////////////////////////////////////////

    inline static void GetFileExt(const char* path, char* outExt)
    {
        // returns the file extension path component
        const char* dotPos = strrchr(path, '.');

        if (dotPos)
        {
            strcpy(outExt, dotPos);
            outExt[strlen(dotPos)] = '\0';                     // put extra NULL
        }
        else
        {
            strcpy(outExt, "invalid");
        }
    }

private:
    inline static bool IsEmpty(const char* str)     { return ((str == nullptr) || (str[0] == '\0')); }
    inline static bool IsEmpty(const wchar_t* wstr) { return ((wstr == nullptr) || (wstr[0] == L'\0')); }
};

} // namespace
