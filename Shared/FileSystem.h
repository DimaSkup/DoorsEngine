// *********************************************************************************
// Filename:     Filesystem.h
// Description:  utils for working with files paths
// 
// Created:      17.04.2025 by DimaSkup
// *********************************************************************************
#pragma once

#include "log.h"
#include <string.h>


class FileSys
{
public:

    //-----------------------------------------------------
    // check if file by filePath (relatively to the working directory) exists
    //-----------------------------------------------------
    inline static bool Exists(const char* filePath)
    {
        if (IsEmpty(filePath))
        {
            LogErr(LOG, "input path is empty!");
            return false;
        }

        FILE* pFile = nullptr;

        // check if such file exists
        if ((pFile = fopen(filePath, "r+")) == nullptr)
        {
            LogErr(LOG, "there is no file by path: %s", filePath);
            return false;
        }

        fclose(pFile);
        return true;
    }

    //-----------------------------------------------------
    // return relative path from the project root
    //-----------------------------------------------------
    inline static void GetPathFromProjRoot(const char* fullPath, char* outPath)
    {
        if (IsEmpty(fullPath))
        {
            LogErr(LOG, "input path is empty!");
            return;
        }

        if (!outPath)
        {
            LogErr(LOG, "in-out path == nullptr");
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

    //-----------------------------------------------------
    // Desc:   check if engine supports this filepath
    //         (path can contain only: alpha symbols, digit, \\, /, -, _, . (dot) )
    //-----------------------------------------------------
    static bool IsPathSupported(const char* path)
    {
        if (IsEmpty(path))
        {
            LogErr(LOG, "input path is empty");
            return false;
        }

        for (int i = 0; i < path[i] != '\0'; ++i)
        {
            bool isValid = false;

            isValid |= (bool)isalpha(path[i]);
            isValid |= (bool)isdigit(path[i]);
            isValid |= (path[i] == '\\');
            isValid |= (path[i] == '/');
            isValid |= (path[i] == '_');
            isValid |= (path[i] == '-');
            isValid |= (path[i] == '.');
            isValid |= (path[i] == ':');

            if (!isValid)
            {
                LogErr(LOG, "path isn't supported: %s\n (path can contain only: alpha symbols, digit, \\, /, and _)", path);
                return false;
            }
        }

        return true;
    }

    //-----------------------------------------------------
    // Desc:   check if engine supports this filepath
    //         (path can contain only: alpha symbols, digit, \\, /, -, _, . (dot) )
    //-----------------------------------------------------
    static bool IsPathSupported(const wchar_t* path)
    {
        if (IsEmpty(path))
        {
            LogErr(LOG, "input path is empty");
            return false;
        }

        for (int i = 0; i < path[i] != '\0'; ++i)
        {
            bool isValid = false;

            isValid |= (bool)isalpha(path[i]);
            isValid |= (bool)isdigit(path[i]);
            isValid |= (path[i] == L'\\');
            isValid |= (path[i] == L'/');
            isValid |= (path[i] == L'_');
            isValid |= (path[i] == L'-');
            isValid |= (path[i] == L'.');
            isValid |= (path[i] == L':');

            if (!isValid)
            {
                LogErr(LOG, "path isn't supported: %ls\n (path can contain only: alpha symbols, digit, \\, /, and _)", path);
                return false;
            }
        }

        return true;
    }

    //-----------------------------------------------------
    // Desc:   get position of a last slash in the input path
    //-----------------------------------------------------
    inline static ptrdiff_t GetLastSlashOffset(const char* path)
    {
        if (IsEmpty(path))
        {
            LogErr(LOG, "input path is empty");
            return -1;
        }

        const char* offset1 = strrchr(path, '\\');
        const char* offset2 = strrchr(path, '/');

        // if no slash or backslash
        if ((offset1 == NULL) && (offset2 == NULL))
            return 0;


        ptrdiff_t offset = 0;

        if (offset1 != NULL)
        {
            const ptrdiff_t off = offset1 - path;

            if (offset < off)
                offset = off;
        }

        if (offset2 != NULL)
        {
            const ptrdiff_t off = offset2 - path;
            if (offset < off)
                offset = off;
        }

        return offset;
    }

    //-----------------------------------------------------
    // Desc:   get parent path by input path
    //-----------------------------------------------------
    inline static void GetParentPath(const char* path, char* outPath)
    {
        const char* lastSlash = path + GetLastSlashOffset(path);
        const ptrdiff_t pathLen = lastSlash - path + 1;

        strncpy(outPath, path, pathLen);
        outPath[pathLen] = '\0';                           // put extra NULL
    }

    //-----------------------------------------------------
    // returns the filename by input path
    //-----------------------------------------------------
    inline static void GetFileName(const char* path, char* outFilename)
    {
        const ptrdiff_t offset = GetLastSlashOffset(path);
        strcpy(outFilename, path+offset+1);
        //outFilename[offset] = '\0';              // put extra NULL
    }

    //-----------------------------------------------------
    // returns the stem path component (filename without the final extension)
    //-----------------------------------------------------
    inline static void GetFileStem(const char* path, char* outStem)
    {
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

    //-----------------------------------------------------
    // returns the file extension path component
    //-----------------------------------------------------
    inline static void GetFileExt(const char* path, char* outExt)
    {
        if (IsEmpty(path))
        {
            LogErr(LOG, "input path is empty");
            return;
        }

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
    //-----------------------------------------------------
    // Desc:   check if input string is empty
    //-----------------------------------------------------
    inline static bool IsEmpty(const char* str)     { return ((str == nullptr) || (str[0] == '\0')); }
    inline static bool IsEmpty(const wchar_t* wstr) { return ((wstr == nullptr) || (wstr[0] == L'\0')); }
};
