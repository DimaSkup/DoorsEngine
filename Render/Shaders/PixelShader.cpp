////////////////////////////////////////////////////////////////////
// Filename: PixelShader.cpp
// Revising: 05.11.22
////////////////////////////////////////////////////////////////////
#include "PixelShader.h"

#include "../Common/StrHelper.h"
#include "../Common/MemHelpers.h"
#include "../Common/Assert.h"
#include "../Common/Log.h"

#include "Helpers/CSOLoader.h"
#include "Helpers/ShaderCompiler.h"


namespace Render
{

PixelShader::PixelShader()
{
}

PixelShader::~PixelShader()
{
    Shutdown();
}

///////////////////////////////////////////////////////////

bool PixelShader::Initialize(ID3D11Device* pDevice, const char* shaderPath)
{
    // initializing of a pixel shader object

    if (StrHelper::IsEmpty(shaderPath))
    {
        LogErr("input path to pixel shader file is empty!");
        return false;
    }

    uint8_t* buffer = nullptr;

    // load in shader bytecode
    const size_t len = LoadCSO(shaderPath, buffer);
    if (!len)
    {
        SafeDeleteArr(buffer);
        sprintf(g_String, "Failed to load .CSO-file of pixel shader: %s", shaderPath);
        LogErr(g_String);
        return false;
    }

    HRESULT hr = pDevice->CreatePixelShader((void*)buffer, len, nullptr, &pShader_);
    if (FAILED(hr))
    {
        SafeDeleteArr(buffer);
        sprintf(g_String, "Failed to create a pixel shader obj: %s", shaderPath);
        LogErr(g_String);
        Shutdown();
        return false;
    }

    // Release the pixel shader buffer since it is no longer needed.
    SafeDeleteArr(buffer);

    return true;
}

///////////////////////////////////////////////////////////

bool PixelShader::CompileShaderFromFile(
    ID3D11Device* pDevice,
    const char* shaderPath,
    const char* funcName,
    const char* shaderProfile)
{
    // is using for hot reload

    if (StrHelper::IsEmpty(shaderPath) || StrHelper::IsEmpty(funcName) || StrHelper::IsEmpty(shaderProfile))
    {
        LogErr("input arguments are invalid: some string is empty");
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
        sprintf(g_String, "can't compile a pixel shader from file: %s", shaderPath);
        LogErr(g_String);
        return false;
    }


    hr = pDevice->CreatePixelShader(
        pShaderBuffer->GetBufferPointer(),
        pShaderBuffer->GetBufferSize(),
        nullptr,
        &pShader);
    if (FAILED(hr))
    {
        sprintf(g_String, "Failed to create a pixel shader obj: %s", shaderPath);
        LogErr(g_String);
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

///////////////////////////////////////////////////////////

void PixelShader::Shutdown()
{
    LogDbg("shutdown");
    SafeRelease(&pShader_);
}

} // namespace 
