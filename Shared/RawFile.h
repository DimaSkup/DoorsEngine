// =================================================================================
// Filename:   RawFile.h
// Desc:       loader/saver for the RAW files
//
// Created:    22.06.2025  by DimaSkup
// =================================================================================
#pragma once

#include "MemHelpers.h"
#include "log.h"
#include <stdint.h>
#include <stdio.h>

#pragma warning (disable : 4996)
using uint8 = uint8_t;

// --------------------------------------------------------
// Desc:   save input data into RAW file
// Args:   - filename: path to the raw file
//         - data:     actual raw data
//         - numBytes: size of the raw data (number of bytes)
// Ret:    true if the file was successfully saved
// --------------------------------------------------------
static bool SaveRAW(const char* filename, const uint8* data, const int numBytes)
{
    FILE* pFile = nullptr;

    // check input args
    if (!filename || filename[0] == '\0')
    {
        LogErr("input filename is empty!");
        return false;
    }

    if (!data)
    {
        LogErr("input ptr to data == nullptr");
        return false;
    }

    if (numBytes <= 0)
    {
        LogErr("input number of bytes (file size) must be > 0");
        return false;
    }


    // open a file for the RAW data to be saved to
    pFile = fopen(filename, "wb");
    if (!pFile)
    {
        sprintf(g_String, "can't open file: %s", filename);
        LogErr(g_String);
        return false;
    }

    // write the light map to the file
    if (fwrite(data, 1, numBytes, pFile) != numBytes)
    {
        sprintf(g_String, "can't write raw data into file: %s", filename);
        LogErr(g_String);
        return false;
    }

    fclose(pFile);

    // great success!
    sprintf(g_String, "Saved RAW file: %s", filename);
    LogMsg(g_String);
    return true;
}

// --------------------------------------------------------
// Desc:   load data from RAW file
// Args:   - filename: path to the file
//         - outData:  output array of loaded data
//         - outSize:  output size of the file
// Ret:    true if we successfully loaded the file
// --------------------------------------------------------
static bool LoadRAW(const char* filename, uint8** outData, int& outSize)
{
    FILE* pFile = nullptr;

    // check input params
    if (!filename || filename[0] == '\0')
    {
        LogErr("input filename is empty!");
        return false;
    }

    // open the file
    pFile = fopen(filename, "rb");
    if (!pFile)
    {
        sprintf(g_String, "can't open file: %s", filename);
        LogErr(g_String);
        return false;
    }

    // release memory from prev data (if we have any)
    SafeDeleteArr(*outData);

    // define file size
    fseek(pFile, 0, SEEK_END);
    outSize = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    // allocate the memory for our data
    *outData = new uint8[outSize]{ 0 };

    // read in the file content
    size_t res = fread(*outData, 1, outSize, pFile);
    if (res != outSize)
    {
        sprintf(g_String, "can't read in data from the file: %s", filename);
        LogErr(g_String);

        fclose(pFile);
        SafeDeleteArr(*outData);
        return false;
    }

    // close the file
    fclose(pFile);

    // great success!
    sprintf(g_String, "Loaded RAW file: %s", filename);
    LogMsg(g_String);
    return true;
}
