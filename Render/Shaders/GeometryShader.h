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

    bool Initialize(ID3D11Device* pDevice, const char* shaderPath);
    
    void Shutdown();

    // public query API
    inline ID3D11GeometryShader* GetShader()    { return pShader_; };

private:
    ID3D11GeometryShader* pShader_ = nullptr;
    uint8_t* pShaderBuffer_ = nullptr;
    
};

};  // namespace Render
