// =================================================================================
// Filename: GeometryShader.cpp
// Created:  01.03.25  by DimaSkup
// =================================================================================
#include "GeometryShader.h"

#include "../Common/MemHelpers.h"
#include "../Common/Log.h"

#include "Helpers/CSOLoader.h"


namespace Render
{

GeometryShader::~GeometryShader()
{
    Shutdown();
}

///////////////////////////////////////////////////////////

bool GeometryShader::Initialize(ID3D11Device* pDevice, const char* shaderPath)
{
    // THIS FUNC compiles/load an HLSL/CSO shader by shaderPath;
    // compiles this shader into buffer, and then creates a geometry shader object

    if ((shaderPath == nullptr) || (shaderPath[0] == '\0'))
    {
        LogErr("input path to geometry shader file is empty!");
        return false;
    }

#if 0
    // compile a vertex shader into the buffer
    hr = ShaderClass::CompileShaderFromFile(
        shaderPath.c_str(),
        funcName.c_str(),
        "vs_5_0",
        &pShaderBuffer_,
        errorMgr);
    Assert::NotFailed(hr, errorMgr);
#endif


    std::streampos len = 0;

    // load in shader bytecode
    const bool result = LoadCSO(shaderPath, &pShaderBuffer_, len);
    if (!result)
    {
        sprintf(g_String, "Failed to load .CSO-file of geometry shader: %s", shaderPath);
        LogErr(g_String);
        Shutdown();
        return false;
    }

    // --------------------------------------------

    const HRESULT hr = pDevice->CreateGeometryShader(
        pShaderBuffer_,                 //pShaderBuffer_->GetBufferPointer(),
        len,                            //pShaderBuffer_->GetBufferSize(),
        nullptr,
        &pShader_);

    if (FAILED(hr))
    {
        sprintf(g_String, "Failed to create a geometry shader obj: %s", shaderPath);
        LogErr(g_String);
        Shutdown();
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////

void GeometryShader::Shutdown()
{
    // Shutting down of the class object, releasing of the memory, etc.

    LogDbg("Shutdown");
    SafeDeleteArr(pShaderBuffer_);
    SafeRelease(&pShader_);
}

}; // namespace Render
