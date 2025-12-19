/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: parse_helpers.h
    Desc:     helpers for parsing values from char buffer or from file

    Created:  02.12.2025  by DimaSkup
\**********************************************************************************/
#pragma once

#include <log.h>
#include <assert.h>
#include <stdio.h>


//---------------------------------------------------------
// Desc:  helpers for reading float/float3/float4/string from input CHAR BUFFER
//---------------------------------------------------------
inline void ReadFloat(const char* buf, const char* fmt, float* outFloat)
{
    assert(buf && fmt && outFloat);

    if (sscanf(buf, fmt, outFloat) != 1)
        LogErr(LOG, "can't read float from buffer: %s", buf);
}

//---------------------------------------------------------

inline void ReadFloat3(const char* buf, const char* fmt, float* arr)
{
    assert(buf && fmt && arr);

    if (sscanf(buf, fmt, arr, arr + 1, arr + 2) != 3)
        LogErr(LOG, "can't read float3 from buffer: %s", buf);
}

//---------------------------------------------------------

inline void ReadFloat4(const char* buf, const char* fmt, float* arr)
{
    assert(buf && fmt && arr);

    if (sscanf(buf, fmt, arr, arr + 1, arr + 2, arr + 3) != 4)
        LogErr(LOG, "can't read float4 from buffer: %s", buf);
}

//---------------------------------------------------------

inline void ReadStr(const char* buf, const char* fmt, char* outStr)
{
    assert(buf && fmt && outStr);

    if (sscanf(buf, fmt, outStr) != 1)
        LogErr(LOG, "can't read a string from buffer: %s", buf);
}

//---------------------------------------------------------

inline void ReadInt(const char* buf, const char* fmt, int* outInt)
{
    assert(buf && fmt && outInt);

    if (sscanf(buf, fmt, outInt) != 1)
        LogErr(LOG, "can't read an integer from buffer: %s", buf);
}

//---------------------------------------------------------
// Desc:  helpers for reading data from FILE
//---------------------------------------------------------
inline void ReadFileStr(FILE* pFile, const char* key, char* outStr)
{
    assert(pFile && key && outStr);

    char buf[64]{ '\0' };
    if (fscanf(pFile, "%s", buf) != 1)
        LogErr(LOG, "can't read a key from file (you expect: %s)", key);

    if (strcmp(buf, key) != 0)
        LogErr(LOG, "invalid key (you pass: %s, you've read: %s)", key, buf);

    if (fscanf(pFile, " %s\n", outStr) != 1)
        LogErr(LOG, "can't read str from file (key: %s, you've read: %s)", key, outStr);
}

//---------------------------------------------------------

inline void ReadFileInt(FILE* pFile, const char* key, int* outInt)
{
    assert(pFile && key && outInt);

    char buf[64]{ '\0' };
    if (fscanf(pFile, "%s", buf) != 1)
        LogErr(LOG, "can't read a key from file (you expect: %s)", key);

    if (strcmp(buf, key) != 0)
        LogErr(LOG, "invalid key (you pass: %s, you read: %s)", key, buf);

    if (fscanf(pFile, " %d\n", outInt) != 1)
        LogErr(LOG, "can't parse an integer from file (key: %s, you've read: %d)", key, outInt);
}

//---------------------------------------------------------

inline void ReadFileFloat(FILE* pFile, const char* key, float* outFlt)
{
    assert(pFile && key && outFlt);

    char buf[64]{ '\0' };
    if (fscanf(pFile, "%s", buf) != 1)
        LogErr(LOG, "can't read a key from file (you expect: %s)", key);

    if (strcmp(buf, key) != 0)
        LogErr(LOG, "invalid key (you pass: %s, you read: %s)", key, buf);

    if (fscanf(pFile, " %f\n", outFlt) != 1)
        LogErr(LOG, "can't parse a float from file (key: %s, you've read: %f)", key, outFlt);
}

//---------------------------------------------------------

inline void ReadFileFloat2(FILE* pFile, const char* key, float* arr)
{
    assert(pFile && key && arr);

    char buf[64]{ '\0' };
    if (fscanf(pFile, "%s", buf) != 1)
        LogErr(LOG, "can't read a key from file (you expect: %s)", key);

    if (strcmp(buf, key) != 0)
        LogErr(LOG, "invalid key (you pass: %s, you read: %s)", key, buf);

    if (fscanf(pFile, " %f %f\n", arr + 0, arr + 1) != 2)
        LogErr(LOG, "can't parse a float2 from file (key: %s, you've read: %f %f)", key, arr[0], arr[1]);
}

//---------------------------------------------------------

inline void ReadFileFloat3(FILE* pFile, const char* key, float* arr)
{
    assert(pFile && key && arr);

    char buf[64]{ '\0' };
    if (fscanf(pFile, "%s", buf) != 1)
        LogErr(LOG, "can't read a key from file (you expect: %s)", key);

    if (strcmp(buf, key) != 0)
        LogErr(LOG, "invalid key (you pass: %s, you read: %s)", key, buf);

    if (fscanf(pFile, " %f %f %f\n", arr + 0, arr + 1, arr + 2) != 3)
        LogErr(LOG, "can't parse a float3 from file (key: %s, you've read: %f %f %f)", key, arr[0], arr[1], arr[2]);
}

//---------------------------------------------------------

inline void ReadFileFloat4(FILE* pFile, const char* key, float* arr)
{
    assert(pFile && key && arr);

    char buf[64]{ '\0' };
    if (fscanf(pFile, "%s", buf) != 1)
        LogErr(LOG, "can't read a key from file (you expect: %s)", key);

    if (strcmp(buf, key) != 0)
        LogErr(LOG, "invalid key (you pass: %s, you read: %s)", key, buf);

    if (fscanf(pFile, " %f %f %f %f\n", arr, arr + 1, arr + 2, arr + 3) != 4)
        LogErr(LOG, "can't parse a float3 from file (key: %s, you've read: %f %f %f %f)", key, arr[0], arr[1], arr[2], arr[3]);
}
