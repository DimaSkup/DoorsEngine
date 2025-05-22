// =================================================================================
// Filename:     ShaderCompiler.h
// Description:  this is a base shader class which contains common
//               utils for shader classes to work with HLSL shaders
//
// Revising:     12.06.22
// =================================================================================
#pragma once

#include <d3d11.h>

namespace Render
{

class ShaderCompiler
{
public:
    static HRESULT CompileShaderFromFile(
        const char* filename, 
        LPCSTR functionName,
        LPCSTR shaderProfile,
        ID3D10Blob** shaderOutput);
};

}

