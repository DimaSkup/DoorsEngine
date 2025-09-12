// =================================================================================
// Filename: GeometryShader.cpp
// Created:  01.03.25  by DimaSkup
// =================================================================================
#include "../Common/pch.h"
#include "GeometryShader.h"
#include "ShaderCompiler.h"
#pragma warning (disable : 4996)


namespace Render
{

GeometryShader::~GeometryShader()
{
    Shutdown();
}

//---------------------------------------------------------
// Desc:   load a compiled shader object (CSO) from a file by shaderPath;
//         and create a geometry shader object
// Args:   - shaderPath:   a path to CSO file relatively to the working directory
//---------------------------------------------------------
bool GeometryShader::LoadPrecompiled(ID3D11Device* pDevice, const char* shaderPath)
{
    if (StrHelper::IsEmpty(shaderPath))
    {
        LogErr("input path to geometry shader file is empty!");
        return false;
    }

    uint8_t* buf = nullptr;

    // load shader bytecode into the buffer
    const size_t len = ShaderCompiler::LoadCSO(shaderPath, buf);
    if (!len)
    {
        LogErr(LOG, "Failed to load .CSO-file of geometry shader: %s", shaderPath);
        Shutdown();
        return false;
    }

    const HRESULT hr = pDevice->CreateGeometryShader(buf, len, nullptr, &pShader_);
    if (FAILED(hr))
    {
        LogErr(LOG, "Failed to create a geometry shader obj: %s", shaderPath);
        Shutdown();
        return false;
    }

    // Release the temp CSO buffer since it is no longer needed.
    SafeDeleteArr(buf);

    return true;
}

//---------------------------------------------------------
// Desc:   is used for hot reload:
//         compile an HLSL shader by shaderPath and reinit shader object
// Args:   - path:           a path to HLSL shader file relatively to the working directory
//         - funcName:       what function from the shader we want to compile
//         - shaderModel:    what HLSL shader model we want to use
//---------------------------------------------------------
bool GeometryShader::CompileFromFile(
    ID3D11Device* pDevice,
    const char* path,
    const char* funcName,
    const char* shaderModel)
{
    if (StrHelper::IsEmpty(path) || StrHelper::IsEmpty(funcName) || StrHelper::IsEmpty(shaderModel))
    {
        LogErr("input arguments are invalid: some path is empty");
        return false;
    }

    ID3D10Blob*           pShaderBuffer = nullptr;
    ID3D11GeometryShader* pShader = nullptr;
    HRESULT               hr = S_OK;

    // generate full shader profile for this shader
    char shaderProfile[8]{ '\0' };
    strcat(shaderProfile, "gs_");
    strcat(shaderProfile, shaderModel);

    // compile a shader and load bytecode into the buffer
    hr = ShaderCompiler::CompileShaderFromFile(
        path,
        funcName,
        shaderProfile,
        &pShaderBuffer);
    if (FAILED(hr))
    {
        SafeRelease(&pShaderBuffer);
        LogErr(LOG, "can't compile a geometry shader from file: %s", path);
        return false;
    }

    hr = pDevice->CreateGeometryShader(
        pShaderBuffer->GetBufferPointer(),
        pShaderBuffer->GetBufferSize(),
        nullptr,
        &pShader);
    if (FAILED(hr))
    {
        LogErr(LOG, "Failed to create a geometry shader obj: %s", path);
        SafeRelease(&pShaderBuffer);
        Shutdown();
        return false;
    }

    // Release the geometry shader buffer since it is no longer needed.
    SafeRelease(&pShaderBuffer);

    // release previous shader's data if we have any
    Shutdown();

    pShader_ = pShader;

    return true;
}

//---------------------------------------------------------
// Desc:  Shutting down of the class object, releasing of the memory, etc.
//---------------------------------------------------------
void GeometryShader::Shutdown()
{
    SafeRelease(&pShader_);
}

}; // namespace Render
