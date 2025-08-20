////////////////////////////////////////////////////////////////////
// Filename:     GeometryShader.h
// Description:  this is a class for handling all the geometry shader stuff
//
// Created:      01.03.25  by DimaSkup
// =================================================================================
#pragma once

#include <cstdint>
#include <d3d11.h>

namespace Render 
{

class GeometryShader
{
public:
    ~GeometryShader();

    bool LoadPrecompiled(ID3D11Device* pDevice, const char* shaderPath);

    // hot reload
    bool CompileFromFile(
        ID3D11Device* pDevice,
        const char* shaderPath,
        const char* funcName,
        const char* shaderProfile);
    
    void Shutdown();

    // public query API
    inline ID3D11GeometryShader* GetShader() { return pShader_; };

private:
    ID3D11GeometryShader* pShader_ = nullptr;
};

};  // namespace Render
