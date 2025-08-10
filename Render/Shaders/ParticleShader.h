// =================================================================================
// Filename:   ParticleShader.h
// Desc:       a shader class which is used for particles rendering
//
// Created:    ??.07.2025  by DimaSkup
// =================================================================================
#pragma once

#include "VertexShader.h"
#include "GeometryShader.h"
#include "PixelShader.h"
#include <d3d11.h>


namespace Render
{

class ParticleShader
{
    using SRV = ID3D11ShaderResourceView;

public:
    ParticleShader();
    ~ParticleShader();

    // restrict a copying of this class instance
    ParticleShader(const ParticleShader& obj) = delete;
    ParticleShader& operator=(const ParticleShader& obj) = delete;

    // Public modification API
    bool Initialize(
        ID3D11Device* pDevice,
        const char* vsFilePath,
        const char* psFilePath,
        const char* gsFilePath);

    void ShaderHotReload(
        ID3D11Device* pDevice,
        const char* vsFilePath,
        const char* gsFilePath,
        const char* psFilePath);

    void Prepare(
        ID3D11DeviceContext* pContext,
        ID3D11Buffer* pVB,
        const UINT stride);

    void Render(
        ID3D11DeviceContext* pContext,
        const UINT baseVertex,
        const UINT numVertices);

    // Public query API
    inline const char* GetShaderName() const { return className_; }

private:
    void InitializeShaders(
        ID3D11Device* pDevice,
        const char* vsFilename,
        const char* gsFilename,
        const char* psFilename);

private:
    VertexShader        vs_;
    GeometryShader      gs_;
    PixelShader         ps_;

    char className_[32]{"ParticleShader"};
};

}  // namespace Render
