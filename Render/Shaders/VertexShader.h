////////////////////////////////////////////////////////////////////
// Filename:     VertexShader.h
// Description:  this is a class for handling all the vertex shader stuff
//
// Revising:     05.11.22
////////////////////////////////////////////////////////////////////
#pragma once

#include <cstdint>
#include <d3d11.h>

namespace Render 
{

class VertexShader
{
public:
    ~VertexShader();

    bool LoadPrecompiled(
        ID3D11Device* pDevice,
        const char* shaderPath,
        const D3D11_INPUT_ELEMENT_DESC* layoutDesc,
        const UINT layoutElemNum);

    // hot reload
    bool CompileFromFile(
        ID3D11Device* pDevice,
        const char* shaderPath,
        const char* funcName,
        const char* shaderProfile,
        const D3D11_INPUT_ELEMENT_DESC* layoutDesc,
        const UINT layoutElemNum);
    
    void Shutdown();

    // public query API
    inline ID3D11VertexShader* GetShader()      { return pShader_; };
    inline ID3D11InputLayout*  GetInputLayout() { return pInputLayout_; };

private:
    ID3D11VertexShader* pShader_ = nullptr;
    ID3D11InputLayout*  pInputLayout_ = nullptr;
};

};  // namespace Render
