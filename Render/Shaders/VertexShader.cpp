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
    Shutdown();
}

//---------------------------------------------------------
// Desc:   a helper to release objects when we failed an initialization
//---------------------------------------------------------
inline void BuffersSafeRelease(
    ID3D11VertexShader* pShader,
    ID3D10Blob* pShaderBuffer,
    ID3D11InputLayout* pInputLayout)
{
    SafeRelease(&pShader);
    SafeRelease(&pShaderBuffer);
    SafeRelease(&pInputLayout);
}

//---------------------------------------------------------
// Desc:   load a CSO (compiled shader object) file by shaderPath;
//         compiles this shader into buffer, and then creates
//         a vertex shader object and an input layout using this buffer;
// 
// Args:   - shaderPath:     a path to CSO file relatively to the working directory
//         - layoutDesc:     description for the vertex input layout
//         - layoutElemNum:  how many elems we have in the input layout
//---------------------------------------------------------
bool VertexShader::Initialize(
    ID3D11Device* pDevice,
    const char* shaderPath,
    const D3D11_INPUT_ELEMENT_DESC* layoutDesc,
    const UINT layoutElemNum)
{
    if (StrHelper::IsEmpty(shaderPath))
    {
        LogErr("input path to vertex shader file is empty!");
        return false;
    }

    HRESULT hr = S_OK;
    uint8_t* buf = nullptr;

    // load in shader bytecode from the .cso file
    const size_t len = LoadCSO(shaderPath, buf);
    if (!len)
    {
        LogErr(LOG, "Failed to load .CSO-file of vertex shader: %s", shaderPath);
        Shutdown();
        return false;
    }

    hr = pDevice->CreateVertexShader((void*)buf, len, nullptr, &pShader_);
    if (FAILED(hr))
    {
        SafeDeleteArr(buf);
        LogErr(LOG, "Failed to create a vertex shader obj: %s", shaderPath);
        return false;
    }

    hr = pDevice->CreateInputLayout(layoutDesc, layoutElemNum, (void*)buf, len, &pInputLayout_);
    if (FAILED(hr))
    {
        SafeDeleteArr(buf);
        SafeRelease(&pShader_);
        LogErr(LOG, "Failed to create the input layout for vertex shader: %s", shaderPath);
        return false;
    }

    // Release the temp CSO buffer since it is no longer needed.
    SafeDeleteArr(buf);

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
bool VertexShader::CompileShaderFromFile(
    ID3D11Device* pDevice,
    const char* shaderPath,
    const char* funcName,
    const char* shaderProfile,
    const D3D11_INPUT_ELEMENT_DESC* layoutDesc,
    const UINT layoutElemNum)
{
    if (StrHelper::IsEmpty(shaderPath) || StrHelper::IsEmpty(funcName) || StrHelper::IsEmpty(shaderProfile))
    {
        LogErr("input arguments are invalid: some path is empty");
        return false;
    }

    ID3D10Blob*         pShaderBuffer = nullptr;
    ID3D11VertexShader* pShader = nullptr;
    ID3D11InputLayout*  pInputLayout = nullptr;
    HRESULT             hr = S_OK;

    // compile a shader and load bytecode into the buffer
    hr = ShaderCompiler::CompileShaderFromFile(
        shaderPath,
        funcName,
        shaderProfile,
        &pShaderBuffer);
    if (FAILED(hr))
    {
        SafeRelease(&pShaderBuffer);
        LogErr(LOG, "can't compile a vertex shader from file: %s", shaderPath);
        return false;
    }

    hr = pDevice->CreateVertexShader(
        pShaderBuffer->GetBufferPointer(),          
        pShaderBuffer->GetBufferSize(),
        nullptr,
        &pShader);
    if (FAILED(hr))
    {
        BuffersSafeRelease(pShader, pShaderBuffer, pInputLayout);
        LogErr(LOG, "Failed to create a vertex shader obj: %s", shaderPath);
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
        LogErr(LOG, "Failed to create the input layout for vertex shader: %s", shaderPath);
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

//---------------------------------------------------------
// Desc:   Shutting down of the vertex shader class object
//---------------------------------------------------------
void VertexShader::Shutdown()
{
    SafeRelease(&pInputLayout_);
    SafeRelease(&pShader_);
}

}; // namespace Render
