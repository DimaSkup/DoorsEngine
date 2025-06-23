// =================================================================================
// Filename:     TerrainShader.h
// Description:  a wrapper class for terrain rendering
//
// Created:      29.05.2025 by DimaSkup
// =================================================================================
#pragma once

#include "VertexShader.h"
#include "PixelShader.h"
#include "SamplerState.h"
#include "ConstantBuffer.h"

#include "../Common/ConstBufferTypes.h"
#include "../Common/RenderTypes.h"
#include <d3d11.h>


namespace Render
{

class TerrainShader
{
public:
    TerrainShader();
    ~TerrainShader();

    // restric copying of this class instance
    TerrainShader(const TerrainShader&) = delete;
    TerrainShader& operator=(const TerrainShader&) = delete;

    // ----------------------------------------------------

    bool Initialize(
        ID3D11Device* pDevice,
        const char* vsFileName,
        const char* psFileName);

    void Render(
        ID3D11DeviceContext* pContext,
        const TerrainInstance& instance);

    void RenderVertices(
        ID3D11DeviceContext* pContext,
        const TerrainInstance& instance);

    void ShaderHotReload(
        ID3D11Device* pDevice,
        const char* vsFilename,
        const char* psFilename);

    inline ID3D11Buffer* GetConstBufferPS() const { return cbpsMaterialData_.Get(); }
    inline const char*   GetShaderName()    const { return className_; }

private:
    void InitializeShaders(
        ID3D11Device* pDevice,
        const char* vsFilename,
        const char* psFilename);

private:
    VertexShader                               vs_;
    PixelShader                                ps_;
    SamplerState                               samplerState_;       // for texturing
    ConstantBuffer<ConstBufType::MaterialData> cbpsMaterialData_;   // cbps -- const buffer for pixel shader

    char className_[16] = "TerrainShader";
};

} // namespace
