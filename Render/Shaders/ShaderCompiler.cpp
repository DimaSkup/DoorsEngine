// =================================================================================
// Filename: ShaderCompiler.cpp
// =================================================================================
#include "../Common/pch.h"
#include "ShaderCompiler.h"
#include <d3dx11async.h>           // for using D3DX11CompileFromFile() function
#include <d3dcompiler.h>           // for using shader flags
#include <vcruntime_new.h>         // std::nothrow
#pragma warning (disable : 4996)


namespace Render
{
//---------------------------------------------------------
// Desc:   compile shader from .hlsl file
// Args:   - path:           a path to HLSL shader relatively to the working directory
//         - funcName:       what function from the shader we want to compile
//         - shaderProfile:  what HLSL shader profile we want to use
//         - shaderOutput:   output container with compiled shader bytecode
// Ret:    result code 
//---------------------------------------------------------
HRESULT ShaderCompiler::CompileShaderFromFile(
    const char* path,
    const char* funcName,
    const char* shaderProfile,
    ID3D10Blob** shaderOutput)
{
    // check input args
    if (!path || path[0] == '\0')
    {
        LogErr(LOG, "input filename is empty");
        return S_FALSE;
    }

    if (!funcName || funcName[0] == '\0')
    {
        LogErr(LOG, "input function name is empty");
        return S_FALSE;
    }

    if (!shaderProfile || shaderProfile[0] == '\0')
    {
        LogErr(LOG, "input shader profile is empty");
        return S_FALSE;
    }


    ID3DBlob* pErrorMsgs = nullptr;
    DWORD compileFlags =
        D3D10_SHADER_WARNINGS_ARE_ERRORS |
        D3D10_SHADER_ENABLE_STRICTNESS |
        D3D10_SHADER_PARTIAL_PRECISION |
        D3D10_SHADER_OPTIMIZATION_LEVEL3;

#if defined(DEBUG) | defined(_DEBUG)
    compileFlags |= D3D10_SHADER_DEBUG;
    compileFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

    const HRESULT hr = D3DX11CompileFromFileA(
        path,                       // pSrcFile:      the name of the .hlsl/.fx file that contains the effect souce code we want to compile
        nullptr,                    // pDefines:      advanced option we do not use; see the SDK documentation;
        nullptr,                    // pInclude:      advanced option we do not use; see the SDK documentation;
        funcName,                   // pFunctionName: the shader function name entry point. This is only used when compiling shader programs individually. When using the effects framework, specify null, as the technique passes defined inside the effect file specify the shader entry points.
        shaderProfile,              // pProfile:      a string specifying the shader version we are using. For Direct3D 11 effects, we use shader version 5.0 ("fx_5_0")
        compileFlags,               // Flags1:        flags to specify how the shader code should be compiled
        0,                          // Flags2:        advanced effect compilation options we do not use; see the SDK documentation
        nullptr,                    // pPump:         advanced option we do not use to compile the shader asynchronously; see the SDK documentation
        shaderOutput,               // ppShader:      returns a point to a ID3D10Blob data structure that stores the compiled shader
        &pErrorMsgs,                // ppErrorMsgs:   returns a pointer to a ID3D11Blob data structure that stores a string containing the compilation errors, if any.
        nullptr);                   // pHResult:      used to obtain the returned error code if compiling asynchronously. Specify null if you specified null for pPump

    // If the shader failed to compile it should write something about the error
    if (pErrorMsgs != nullptr)
    {
        MessageBoxA(0, (char*)pErrorMsgs->GetBufferPointer(), 0, 0);
        LogErr((char*)(pErrorMsgs->GetBufferPointer()));

        pErrorMsgs->Release();
        pErrorMsgs = nullptr;
    }

    // even if there are no errorMsgs, check to make sure there were no other errors
    if (FAILED(hr))
    {
        LogErr(LOG, "can't compile a shader (funcName: %s; profile: %s)\n"
                    "\tfrom the shader file: %s", funcName, shaderProfile, path);
    }

    return hr;
}

//---------------------------------------------------------
// Desc:   read in bytecode from the input .CSO file (compiled shader object)
// Args: 
//---------------------------------------------------------
size_t ShaderCompiler::LoadCSO(const char* shaderPath, uint8_t*& outBytes)
{
    // check input args
    if (!shaderPath || shaderPath[0] == '\0')
    {
        LogErr(LOG, "input shader path is empty");
        return 0;
    }

    if (outBytes)
    {
        LogErr(LOG, "input ptr to output bytes container must be empty");
        return 0;
    }

    // open file
    FILE* pFile = fopen(shaderPath, "rb");
    if (!pFile)
    {
        LogErr(LOG, "can't open a file with shader bytecode: %s", shaderPath);
        return 0;
    }

    fseek(pFile, 0, SEEK_END);
    const size_t len = (size_t)ftell(pFile);

    // alloc memory
    outBytes = new (std::nothrow) uint8_t[len]{ 0 };
    if (!outBytes)
    {
        fclose(pFile);
        LogErr(LOG, "can't alloc memory for shader bytecode from file: %s", shaderPath);
        return 0;
    }

    fseek(pFile, 0, SEEK_SET);

    // read data from file
    size_t readCount = fread(outBytes, 1, len, pFile);
    if (readCount != len)
    {
        fclose(pFile);
        SafeDeleteArr(outBytes);
        LogErr(LOG, "can't read data from a file with shader bytecode: %s", shaderPath);
        return 0;
    }

    fclose(pFile);
    return len;
}

} // namespace
