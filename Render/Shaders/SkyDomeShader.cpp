// =================================================================================
// Filename:     SkyDomeShader.cpp
// Description:  this shader is used to render sky model
//               (sky box / sky sphere / etc.)
// 
// Created:      21.12.24
// =================================================================================
#include "../Common/pch.h"
#include "SkyDomeShader.h"

#pragma warning (disable : 4996)


namespace Render
{

SkyDomeShader::SkyDomeShader()
{
    strcpy(className_, __func__);
}

SkyDomeShader::~SkyDomeShader()
{
}

// =================================================================================
//                             public methods                                      
// =================================================================================
bool SkyDomeShader::Initialize(
    ID3D11Device* pDevice,
    const char* vsFilePath,
    const char* psFilePath)
{
    try
    {
        InitializeShaders(pDevice, vsFilePath, psFilePath);
        LogDbg(LOG, "is initialized");
        return true;
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        LogErr(LOG, "can't initialize the sky dome shader class");
        return false;
    }
}

///////////////////////////////////////////////////////////

void SkyDomeShader::Render(
    ID3D11DeviceContext* pContext,
    const SkyInstance& sky,
    const DirectX::XMMATRIX& worldViewProj)    
{
    // update constant buffers (cbvs/cbps - const buffer for vertex/pixel shader)
    cbvsPerFrame_.data.worldViewProj_ = worldViewProj;
    cbvsPerFrame_.ApplyChanges(pContext);
    
    // prepare input assembler (IA) stage before the rendering process
    UINT offset = 0;

    pContext->IASetVertexBuffers(0, 1, &sky.pVB, &sky.vertexStride, &offset);
    pContext->IASetIndexBuffer(sky.pIB, DXGI_FORMAT_R16_UINT, 0);
    pContext->IASetInputLayout(vs_.GetInputLayout());

    // bind shaders
    pContext->VSSetShader(vs_.GetShader(), nullptr, 0);
    pContext->PSSetShader(ps_.GetShader(), nullptr, 0);

    // render the sky
    pContext->DrawIndexed(sky.indexCount, 0, 0);
}

///////////////////////////////////////////////////////////

void SkyDomeShader::SetSkyGradient(
    ID3D11DeviceContext* pContext,
    const DirectX::XMFLOAT3& colorCenter,  // sky horizon color
    const DirectX::XMFLOAT3& colorApex)    // sky top color
{
    cbpsRareChanged_.data.colorCenter_ = colorCenter;
    cbpsRareChanged_.data.colorApex_   = colorApex;
    cbpsRareChanged_.ApplyChanges(pContext);
}

void SkyDomeShader::SetSkyColorCenter(ID3D11DeviceContext* pContext, const DirectX::XMFLOAT3& color)
{
    // update only center color
    SetSkyGradient(pContext, color, cbpsRareChanged_.data.colorApex_);
}

void SkyDomeShader::SetSkyColorApex(ID3D11DeviceContext* pContext, const DirectX::XMFLOAT3& color)
{
    // update only apex color
    SetSkyGradient(pContext, cbpsRareChanged_.data.colorCenter_, color);
}


// =================================================================================
//                              private methods                                       
// =================================================================================
void SkyDomeShader::InitializeShaders(
    ID3D11Device* pDevice,
    const char* vsFilePath,
    const char* psFilePath)
{
    // helps to initialize the HLSL shaders, layout, sampler state

    bool result = false;
    HRESULT hr = S_OK;

    // input layout desc for the vertex shader
    constexpr UINT numInputLayoutElems = 1U;
    const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[numInputLayoutElems] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };


    
    // initialize: VS, PS, sampler state
    result = vs_.Initialize(pDevice, vsFilePath, inputLayoutDesc, numInputLayoutElems);
    CAssert::True(result, "can't initialize the vertex shader");

    result = ps_.Initialize(pDevice, psFilePath);
    CAssert::True(result, "can't initialize the pixel shader");;

    hr = cbvsPerFrame_.Initialize(pDevice);
    CAssert::NotFailed(hr, "can't init a const buffer (VS)");

    hr = cbpsRareChanged_.Initialize(pDevice);
    CAssert::NotFailed(hr, "can't init a const buffer (PS)");


    // setup the constant buffers with default values and load data into GPU
    ID3D11DeviceContext* pContext = nullptr;
    pDevice->GetImmediateContext(&pContext);

    cbvsPerFrame_.ApplyChanges(pContext);
    cbpsRareChanged_.ApplyChanges(pContext);
}

} // namespace Render
