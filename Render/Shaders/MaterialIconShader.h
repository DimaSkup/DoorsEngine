// =================================================================================
// Filename:     MaterialIconShader.h
// Description:  is used for rendering material icon (sphere + single material)
//               which is showing in the editor's material browser
//
// Created       30.04.2025 by DimaSkup
// =================================================================================
#pragma once

#include "VertexShader.h"
#include "PixelShader.h"
#include "SamplerState.h"        // wrapper over the ID3D11SamplerState
#include "ConstantBuffer.h"

#include "../Common/ConstBufferTypes.h"
#include "../Common/RenderTypes.h"

#include <d3d11.h>


namespace Render
{

class MaterialIconShader
{
public:
    MaterialIconShader();
    ~MaterialIconShader();

    // restrict copying of this class instance
    MaterialIconShader(const MaterialIconShader&) = delete;
    MaterialIconShader& operator=(const MaterialIconShader&) = delete;

    // ----------------------------------------------------

    bool Initialize(
        ID3D11Device* pDevice,
        const char* vsFilePath,
        const char* psFilePath);

    void ShaderHotReload(
        ID3D11Device* pDevice,
        const char* vsFilePath,
        const char* psFilePath);

    void PrepareRendering(
        ID3D11DeviceContext* pContext,
        ID3D11Buffer* vertexBuffer,
        ID3D11Buffer* indexBuffer,
        const int vertexSize);

    void Render(
        ID3D11DeviceContext* pContext,
        const int indexCount,
        ID3D11ShaderResourceView* const* ppTextures,
        const Render::Material& mat);

    void SetMatrix(
        ID3D11DeviceContext* pContext,
        const DirectX::XMMATRIX& world,
        const DirectX::XMMATRIX& view,
        const DirectX::XMMATRIX& proj);

    inline const char* GetShaderName() const { return className_; }

private:
    void InitializeHelper(
        ID3D11Device* pDevice,
        const char* vsFilePath,
        const char* psFilePath);

private:
    VertexShader                                vs_;
    PixelShader                                 ps_;
    SamplerState                                samplerState_;      // a sampler for texturing
    ConstantBuffer<ConstBufType::WorldViewProj> cbvsWorldViewProj_; // cbvs -- const buffer for vertex shader
    ConstantBuffer<ConstBufType::MaterialData>  cbpsMaterialData_;  // cbps -- const buffer for pixel shader

    char className_[32]{ "MaterialIconShader" };
};

} // namespace Render
