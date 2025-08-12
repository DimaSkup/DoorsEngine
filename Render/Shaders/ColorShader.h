/////////////////////////////////////////////////////////////////////
// Filename:       ColorShader.h
// Description:    We use this class to invoke HLSL shaders 
//                 for drawing our 3D models which are on the GPU
//
// Revising:       06.04.22
/////////////////////////////////////////////////////////////////////
#pragma once

#include "VertexShader.h"
#include "PixelShader.h"

#include "../Common/RenderTypes.h"

#include <d3d11.h>
#include <DirectXMath.h>


namespace Render
{

class ColorShader
{
private:
    struct ConstBuffPerFrame
    {
        DirectX::XMMATRIX viewProj;
    };

    struct InstancedData
    {
        DirectX::XMFLOAT4X4 world;
        DirectX::XMFLOAT4 color;
    };


public:
    ColorShader();
    ~ColorShader();

    // restrict a copying of this class instance
    ColorShader(const ColorShader& obj) = delete;
    ColorShader& operator=(const ColorShader& obj) = delete;

    bool Initialize(
        ID3D11Device* pDevice,
        const char* vsFilePath,
        const char* psFilePath);

    void Render(
        ID3D11DeviceContext* pContext,
        ID3D11Buffer* pInstancedBuffer,
        const InstanceBatch* instances,
        const int numModels,
        const UINT instancesBuffElemSize);

    inline const char* GetShaderName() const { return className_; }

private:
    VertexShader   vs_;
    PixelShader    ps_;

    char className_[32]{"ColorShader"};
};

} // namespace Render
