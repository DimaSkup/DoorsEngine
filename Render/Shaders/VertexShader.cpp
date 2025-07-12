////////////////////////////////////////////////////////////////////
// Filename: VertexShader.cpp
// Revising: 05.11.22
////////////////////////////////////////////////////////////////////
#include "../Common/pch.h"
#include "VertexShader.h"

#include "Helpers/CSOLoader.h"
#include "Helpers/ShaderCompiler.h"


namespace Render
{

VertexShader::~VertexShader()
{
}

///////////////////////////////////////////////////////////

inline void BuffersSafeRelease(
    ID3D11VertexShader* pShader,
    ID3D10Blob* pShaderBuffer,
    ID3D11InputLayout* pInputLayout)
{
    // a helper to release objects 
    LogDbg(LOG, "Shutdown");

    SafeRelease(&pShader);
    SafeRelease(&pShaderBuffer);
    SafeRelease(&pInputLayout);
}

///////////////////////////////////////////////////////////

bool VertexShader::Initialize(
    ID3D11Device* pDevice,
    const char* shaderPath,
    const D3D11_INPUT_ELEMENT_DESC* layoutDesc,
    const UINT layoutElemNum)
{
    // load a CSO (compiled shader object) file by shaderPath;
    // compiles this shader into buffer, and then creates
    // a vertex shader object and an input layout using this buffer;

    if (StrHelper::IsEmpty(shaderPath))
    {
        LogErr("input path to vertex shader file is empty!");
        return false;
    }

    HRESULT hr = S_OK;
    uint8_t* buffer = nullptr;

    // load in shader bytecode from the .cso file
    const size_t len = LoadCSO(shaderPath, buffer);
    if (!len)
    {
        sprintf(g_String, "Failed to load .CSO-file of vertex shader: %s", shaderPath);
        LogErr(g_String);
        Shutdown();
        return false;
    }

    hr = pDevice->CreateVertexShader((void*)buffer, len, nullptr, &pShader_);
    if (FAILED(hr))
    {
        SafeDeleteArr(buffer);
        sprintf(g_String, "Failed to create a vertex shader obj: %s", shaderPath);
        LogErr(g_String);
        return false;
    }

    hr = pDevice->CreateInputLayout(layoutDesc, layoutElemNum, (void*)buffer, len, &pInputLayout_);
    if (FAILED(hr))
    {
        SafeDeleteArr(buffer);
        SafeRelease(&pShader_);
        sprintf(g_String, "Failed to create the input layout for vertex shader: %s", shaderPath);
        LogErr(g_String);
        return false;
    }

    // Release the vertex shader buffer since it is no longer needed.
    SafeDeleteArr(buffer);

    return true;  
}

///////////////////////////////////////////////////////////

bool VertexShader::CompileShaderFromFile(
    ID3D11Device* pDevice,
    const char* shaderPath,
    const char* funcName,
    const char* shaderProfile,
    const D3D11_INPUT_ELEMENT_DESC* layoutDesc,
    const UINT layoutElemNum)
{
    // is using for hot reload:
    // compile an HLSL shader by shaderPath and reinit shader object

    if (StrHelper::IsEmpty(shaderPath) || StrHelper::IsEmpty(funcName) || StrHelper::IsEmpty(shaderProfile))
    {
        LogErr("input arguments are invalid: some string is empty");
        return false;
    }


    ID3D10Blob*         pShaderBuffer = nullptr;
    ID3D11VertexShader* pShader = nullptr;
    ID3D11InputLayout*  pInputLayout = nullptr;
    HRESULT hr = S_OK;

    // compile a vertex shader and load bytecode into the buffer
    hr = ShaderCompiler::CompileShaderFromFile(
        shaderPath,
        funcName,
        shaderProfile,
        &pShaderBuffer);
    if (FAILED(hr))
    {
        SafeRelease(&pShaderBuffer);
        sprintf(g_String, "can't compile a vertex shader from file: %s", shaderPath);
        LogErr(g_String);
        return false;
    }

    const size_t bufSize = pShaderBuffer->GetBufferSize();

    hr = pDevice->CreateVertexShader(
        pShaderBuffer->GetBufferPointer(),          
        pShaderBuffer->GetBufferSize(),
        nullptr,
        &pShader);
    if (FAILED(hr))
    {
        BuffersSafeRelease(pShader, pShaderBuffer, pInputLayout);
        sprintf(g_String, "Failed to create a vertex shader obj: %s", shaderPath);
        LogErr(g_String);
        return false;
    }

    hr = pDevice->CreateInputLayout(
        layoutDesc,
        layoutElemNum,
        pShaderBuffer->GetBufferPointer(),
        pShaderBuffer->GetBufferSize(),
        &pInputLayout);
    if (FAILED(hr))
    {
        BuffersSafeRelease(pShader, pShaderBuffer, pInputLayout);
        sprintf(g_String, "Failed to create the input layout for vertex shader: %s", shaderPath);
        LogErr(g_String);
        return false;
    }

    // Release the vertex shader buffer since it is no longer needed.
    SafeRelease(&pShaderBuffer);

    // release previous shader's data if we have any
    Shutdown();

    pShader_        = pShader;
    pInputLayout_   = pInputLayout;

    return true;
}

///////////////////////////////////////////////////////////

void VertexShader::Shutdown()
{
    // Shutting down of the class object, releasing of the memory, etc.
    LogDbg(LOG, "Shutdown");

    SafeRelease(&pInputLayout_);
    SafeRelease(&pShader_);
}

}; // namespace Render
