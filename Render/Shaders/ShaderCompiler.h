// =================================================================================
// Filename:     ShaderCompiler.h
// Description:  functional for load precompiled shader from .cso files
//               and for compilation of .hlsl shaders
// =================================================================================
#pragma once

#include <d3d11.h>

namespace Render
{

class ShaderCompiler
{
public:
    static HRESULT CompileShaderFromFile(
        const char* path,
        const char* funcName,
        const char* shaderProfile,
        ID3D10Blob** shaderOutput);

    static size_t LoadCSO(const char* path, uint8_t*& bytes);
};

}

