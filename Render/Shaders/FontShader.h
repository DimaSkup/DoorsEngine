////////////////////////////////////////////////////////////////////
// Filename:     FontShader.h
// Description:  this is a class for rendering fonts images
//               using HLSL shaders.
//
// Revising:     23.07.22
////////////////////////////////////////////////////////////////////
#pragma once

#include "../Common/Types.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "SamplerState.h"
#include "ConstantBuffer.h"

#include "../Common/ConstBufferTypes.h"

#include <d3d11.h>
#include <DirectXMath.h>

namespace Render
{

class FontShader
{
public:
    using SRV = ID3D11ShaderResourceView;

public:
    FontShader();
    ~FontShader();

    // restrict a copying of this class instance
    FontShader(const FontShader& obj) = delete;
    FontShader& operator=(const FontShader& obj) = delete;


    bool Initialize(
        ID3D11Device* pDevice,
        const DirectX::XMMATRIX& WVO,
        const char* vsFilePath,
        const char* psFilePath);

    // Public rendering API
    void Render(
        ID3D11DeviceContext* pContext,
        ID3D11Buffer* const* vertexBuffers,            // array of text vertex buffers
        ID3D11Buffer* const* indexBuffers,             // array of text indices buffers
        const uint32_t* indexCounts,                  // array of index counts in each index buffer
        const size numSentences,
        const uint32_t fontVertexSize,
        SRV* const* ppFontTexSRV);
    
    // Public modification API
    void SetWorldViewOrtho(ID3D11DeviceContext* pContext, const DirectX::XMMATRIX& WVO);
    void SetFontColor     (ID3D11DeviceContext* pContext, const DirectX::XMFLOAT3& color);
    

    // Public query API
    inline const char* GetShaderName()      const { return className_; }
    inline ID3D11Buffer* GetConstBufferVS() const { return matrixBuffer_.Get(); }
    inline ID3D11Buffer* GetConstBufferPS() const { return pixelBuffer_.Get(); }

private:
    // initializes the HLSL shaders, input layout, sampler state and buffers
    void InitializeShaders(
        ID3D11Device* pDevice,
        const DirectX::XMMATRIX& WVO,
        const char* vsFilePath,
        const char* psFilePath);

private:
    VertexShader   vs_;
    PixelShader    ps_;
    SamplerState   samplerState_;

    ConstantBuffer<ConstBufType::ConstantMatrixBuffer_FontVS> matrixBuffer_;
    ConstantBuffer<ConstBufType::ConstantPixelBuffer_FontPS>  pixelBuffer_;   // text colour for the pixel shader

    char className_[32]{"FontShader"};
};

}
