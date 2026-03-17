// *********************************************************************************
// Filename:     file_system.cpp
// Description:  implementation of utils for working with files paths
// 
// Created:      28.02.2025 by DimaSkup
// *********************************************************************************
#include "file_system.h"
#include "log.h"
#include <stdio.h>        // for using FILE
#include <ctype.h>        // for using isalpha, isdigit
#include <string.h>

#pragma warning (disable : 4996)

//-----------------------------------------------------
// check if file by filePath (relatively to the working directory) exists
//-----------------------------------------------------
bool FileSys::Exists(const char* filePath)
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
void FileSys::GetPathFromProjRoot(const char* fullPath, char* outPath)
{
    if ((!fullPath) || (fullPath[0] == '\0'))
    {
        LogErr(LOG, "empty input path");
        return;
    }

    if (!outPath)
    {
        LogErr(LOG, "in-out path == nullptr");
        return;
    }

    const char* found = strstr(fullPath, "DoorsEngine\\");

    // if we found the substring we copy all the text after "DoorsEngine\"
    if (found != nullptr)
        strcpy(outPath, found + strlen("DoorsEngine\\"));

    else
        outPath[0] = '\0';
}

//-----------------------------------------------------
// Desc:   check if engine supports this filepath
//         (path can contain only: alpha symbols, digit, \\, /, -, _, . (dot) )
//-----------------------------------------------------
bool FileSys::IsPathSupported(const char* path)
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
bool FileSys::IsPathSupported(const wchar_t* path)
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
int FileSys::GetLastSlashOffset(const char* path)
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

    return (int)offset;
}

//-----------------------------------------------------
// Desc:   get parent path by input path
//-----------------------------------------------------
void FileSys::GetParentPath(const char* path, char* outPath)
{
    const char* lastSlash = path + GetLastSlashOffset(path);
    const ptrdiff_t pathLen = lastSlash - path + 1;

    strncpy(outPath, path, pathLen);
    outPath[pathLen] = '\0';                           // put extra NULL
}

//-----------------------------------------------------
// returns the filename by input path
//-----------------------------------------------------
void FileSys::GetFileName(const char* path, char* outFilename)
{
    const ptrdiff_t offset = GetLastSlashOffset(path);
    strcpy(outFilename, path + offset + 1);
    //outFilename[offset] = '\0';              // put extra NULL
}

//-----------------------------------------------------
// returns the stem path component (filename without the final extension)
//-----------------------------------------------------
void FileSys::GetFileStem(const char* path, char* outStem)
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
void FileSys::GetFileExt(const char* path, char* outExt)
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
