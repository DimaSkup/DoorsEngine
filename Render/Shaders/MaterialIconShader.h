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

    void Render(
        ID3D11DeviceContext* pContext,
        ID3D11Buffer* vertexBuffer,
        ID3D11Buffer* indexBuffer,
        const int indexCount,
        ID3D11ShaderResourceView* const* ppTextures,
        const int vertexSize);

    inline const char* GetShaderName() const { return className_; }

private:
    void InitializeHelper(
        ID3D11Device* pDevice,
        const char* vsFilePath,
        const char* psFilePath);

private:
    VertexShader vs_;
    PixelShader  ps_;
    SamplerState samplerState_;                    // a sampler for texturing

    char className_[32]{ "MaterialIconShader" };
};

} // namespace Render
