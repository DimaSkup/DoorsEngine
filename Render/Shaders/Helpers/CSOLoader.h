#pragma once

#include "../../Common/MemHelpers.h"
#include "../../Common/Assert.h"
#include "../../Common/log.h"
#include <fstream>

#pragma warning (disable : 4996)


namespace Render
{

static bool LoadCSO(const char* shaderPath, uint8_t** blob, std::streampos& len)
{
    // read in bytecode from the input .CSO file (compiled shader object)

    std::ifstream inShaderFile;

    try
    {
        // try to open a file
        inShaderFile.open(shaderPath, std::ios::binary | std::ios::ate);
        if (!inShaderFile.is_open())
        {
            sprintf(g_String, "can't open a file with shader bytecode: %s", shaderPath);
            LogErr(g_String);
            return false;
        }

        // define a length of the compiled shader object (CSO)
        len = inShaderFile.tellg();
        if (inShaderFile.fail())
        {
            sprintf(g_String, "can't define the length of a file with shader bytecode: %s", shaderPath);
            LogErr(g_String);
            return false;
        }


        // alloc memory for the shader bytecode
        *blob = new uint8_t[len]{ 0 };

        inShaderFile.seekg(0, std::ios::beg);


        // read in shader's bytecode
        inShaderFile.read((char*)*blob, len);
        if (inShaderFile.fail())
        {
            SafeDeleteArr(*blob);
            inShaderFile.close();
            sprintf(g_String, "can't read data from a file with shader bytecode: %s", shaderPath);
            LogErr(g_String);
            return false;
        }

        inShaderFile.close();
    }
    catch (std::bad_alloc& e)
    {
        inShaderFile.close();
        sprintf(g_String, "can't allocate memory for shader bytecode from file: %s", shaderPath);

        LogErr(e.what());
        LogErr(g_String);
        return false;
    }

    return true;
}

} // namespace Render
