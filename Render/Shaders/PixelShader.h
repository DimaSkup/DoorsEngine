////////////////////////////////////////////////////////////////////
// Filename:     PixelShader.h
// Description:  this is a class for handling all the pixel shader stuff
//
// Revising:     05.11.22
////////////////////////////////////////////////////////////////////
#pragma once

#include <cstdint>
#include <d3d11.h>

namespace Render
{

class PixelShader
{
public:
    PixelShader();
    ~PixelShader();

    bool Initialize(
        ID3D11Device* pDevice,
        const char* shaderPath);

    bool CompileShaderFromFile(
        ID3D11Device* pDevice,
        const char* shaderPath,
        const char* funcName,
        const char* shaderProfile);

    void Shutdown();

    inline ID3D11PixelShader* GetShader() { return pShader_; };

private:
    ID3D11PixelShader* pShader_ = nullptr;
};

} // namespace
