#pragma once

#include <CAssert.h>
#include <log.h>
#include <stdexcept>

#pragma warning (disable : 4996)


namespace Render
{

static size_t LoadCSO(const char* shaderPath, uint8_t*& bytes)
{
    // read in bytecode from the input .CSO file (compiled shader object)
    try
    {
        FILE* pFile = fopen(shaderPath, "rb");
        if (!pFile)
        {
            sprintf(g_String, "can't open a file with shader bytecode: %s", shaderPath);
            LogErr(g_String);
            return 0;
        }

        fseek(pFile, 0, SEEK_END);
        const size_t len = (size_t)ftell(pFile);

        bytes = new uint8_t[len]{ 0 };
        fseek(pFile, 0, SEEK_SET);

        size_t readCount = fread(bytes, 1, len, pFile);
        if (readCount != len)
        {
            SafeDeleteArr(bytes);
            sprintf(g_String, "can't read data from a file with shader bytecode: %s", shaderPath);
            LogErr(g_String);
            return 0;
        }

        fclose(pFile);
        return len;
    }
    catch (std::bad_alloc& e)
    {
        sprintf(g_String, "can't allocate memory for shader bytecode from file: %s", shaderPath);

        LogErr(e.what());
        LogErr(g_String);
        return false;
    }

    return true;
}

} // namespace Render
