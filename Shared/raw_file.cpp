#include "raw_file.h"
#include "mem_helpers.h"
#include "log.h"
#include <stdint.h>
#include <stdio.h>

#pragma warning (disable : 4996)

// --------------------------------------------------------
// Desc:   save input data into RAW file
// Args:   - filename: path to the raw file
//         - data:     actual raw data
//         - numBytes: size of the raw data (number of bytes)
// Ret:    true if the file was successfully saved
// --------------------------------------------------------
bool SaveRAW(const char* filename, const uint8* data, const int numBytes)
{
    // check input args
    if (!filename || filename[0] == '\0')
    {
        LogErr(LOG, "empty filename");
        return false;
    }
    if (!data)
    {
        LogErr(LOG, "empty data arr (== nullptr)");
        return false;
    }
    if (numBytes <= 0)
    {
        LogErr(LOG, "input number of bytes (file size) must be > 0");
        return false;
    }


    // open a file for the RAW data to be saved to
    FILE* pFile = fopen(filename, "wb");
    if (!pFile)
    {
        LogErr(LOG, "can't open file: %s", filename);
        return false;
    }

    // write the light map to the file
    if (fwrite(data, 1, numBytes, pFile) != numBytes)
    {
        LogErr(LOG, "can't write raw data into file: %s", filename);
        return false;
    }

    fclose(pFile);

    // great success!
    LogMsg(LOG, "Saved RAW file: %s", filename);
    return true;
}

// --------------------------------------------------------
// Desc:   load data from RAW file
// Args:   - filename: path to the file
//         - outData:  output array of loaded data
//         - outSize:  output size of the file
// Ret:    true if we successfully loaded the file
// --------------------------------------------------------
bool LoadRAW(const char* filename, uint8** outData, int& outSize)
{
    // check input params
    if (!filename || filename[0] == '\0')
    {
        LogErr(LOG, "empty filename");
        return false;
    }

    // open the file
    FILE* pFile = fopen(filename, "rb");
    if (!pFile)
    {
        LogErr(LOG, "can't open file: %s", filename);
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
        LogErr(LOG, "can't read in data from the file: %s", filename);

        fclose(pFile);
        SafeDeleteArr(*outData);
        return false;
    }

    // close the file
    fclose(pFile);

    // great success!
    LogMsg(LOG, "Loaded RAW file: %s", filename);
    return true;
}
