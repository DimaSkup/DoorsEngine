////////////////////////////////////////////////////////////////////
// Filename: PixelShader.cpp
// Revising: 05.11.22
////////////////////////////////////////////////////////////////////
#include "../Common/pch.h"
#include "PixelShader.h"
#include "ShaderCompiler.h"


namespace Render
{

PixelShader::PixelShader()
{
}

PixelShader::~PixelShader()
{
    Shutdown();
}

//---------------------------------------------------------
// Desc:   load a CSO (compiled shader object) file by path;
//         compiles this shader into buffer, and then creates pixel shader object
// 
// Args:   - shaderPath:  a path to CSO file relatively to the working directory
//---------------------------------------------------------
bool PixelShader::LoadPrecompiled(ID3D11Device* pDevice, const char* path)
{
    if (StrHelper::IsEmpty(path))
    {
        LogErr(LOG, "input path to pixel shader file is empty!");
        return false;
    }

    uint8_t* buffer = nullptr;

    // load in shader bytecode
    const size_t len = ShaderCompiler::LoadCSO(path, buffer);
    if (!len)
    {
        SafeDeleteArr(buffer);
        LogErr(LOG, "Failed to load .CSO-file of pixel shader: %s", path);
        return false;
    }

    HRESULT hr = pDevice->CreatePixelShader((void*)buffer, len, nullptr, &pShader_);
    if (FAILED(hr))
    {
        SafeDeleteArr(buffer);
        LogErr(LOG, "Failed to create a pixel shader obj: %s", path);
        Shutdown();
        return false;
    }

    // Release the pixel shader buffer since it is no longer needed.
    SafeDeleteArr(buffer);

    return true;
}

//---------------------------------------------------------
// Desc:   is used for hot reload:
//         compile an HLSL shader by shaderPath and reinit shader object
// Args:   - shaderPath:     a path to HLSL shader relatively to the working directory
//         - funcName:       what function from the shader we want to compile
//         - shaderProfile:  what HLSL shader profile we want to use
//         - layoutDesc:     description for the vertex input layout
//         - layoutElemNum:  how many elems we have in the input layout
//---------------------------------------------------------
bool PixelShader::CompileFromFile(
    ID3D11Device* pDevice,
    const char* shaderPath,
    const char* funcName,
    const char* shaderProfile)
{
    if (StrHelper::IsEmpty(shaderPath) || StrHelper::IsEmpty(funcName) || StrHelper::IsEmpty(shaderProfile))
    {
        LogErr(LOG, "input arguments are invalid: some string is empty");
        return false;
    }


    ID3D10Blob*        pShaderBuffer = nullptr;
    ID3D11PixelShader* pShader = nullptr;
    HRESULT hr = S_OK;

    // compile a pixel shader and load bytecode into the buffer
    hr = ShaderCompiler::CompileShaderFromFile(
        shaderPath,
        funcName,
        shaderProfile,
        &pShaderBuffer);
    if (FAILED(hr))
    {
        SafeRelease(&pShaderBuffer);
        LogErr(LOG, "can't compile a pixel shader from file: %s", shaderPath);
        return false;
    }


    hr = pDevice->CreatePixelShader(
        pShaderBuffer->GetBufferPointer(),
        pShaderBuffer->GetBufferSize(),
        nullptr,
        &pShader);
    if (FAILED(hr))
    {
        LogErr(LOG, "Failed to create a pixel shader obj: %s", shaderPath);
        Shutdown();
        return false;
    }

    // Release the pixel shader buffer since it is no longer needed.
    SafeRelease(&pShaderBuffer);

    // release previous shader's data if we have any
    Shutdown();

    pShader_ = pShader;

    return true;
}

//---------------------------------------------------------
// Desc:   Shutting down of the pixel shader class object
//---------------------------------------------------------
void PixelShader::Shutdown()
{
    SafeRelease(&pShader_);
}

} // namespace 
