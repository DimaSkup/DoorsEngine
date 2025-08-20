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

//---------------------------------------------------------
// Decs:   Load CSO / compile HLSL shaders and init this shader class instance
// Args:   - vsPath:  a path to compiled (.cso) vertex shader file
//         - psPath:  a path to compiled (.cso) pixel shader file
//---------------------------------------------------------
bool SkyDomeShader::Initialize(
    ID3D11Device* pDevice,
    const char* vsPath,
    const char* psPath)
{
    try
    {
        CAssert::True(!StrHelper::IsEmpty(vsPath), "input path to vertex shader is empty");
        CAssert::True(!StrHelper::IsEmpty(psPath), "input path to pixel shader is empty");

        bool result = false;
        HRESULT hr = S_OK;
        const InputLayoutSkyDomeShader layout;

        // initialize: VS, PS, sampler state
        result = vs_.LoadPrecompiled(pDevice, vsPath, layout.desc, layout.numElems);
        CAssert::True(result, "can't initialize the vertex shader");

        result = ps_.LoadPrecompiled(pDevice, psPath);
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

//---------------------------------------------------------
// Desc:   render sky dome onto the screen
//---------------------------------------------------------
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

//---------------------------------------------------------
// Desc:   setup the sky gradient from the horizon up to the apex
// Args:   - colorCenter:   sky horizon color
//         - colorApex:     sky top color
//---------------------------------------------------------
void SkyDomeShader::SetSkyGradient(
    ID3D11DeviceContext* pContext,
    const DirectX::XMFLOAT3& colorCenter,
    const DirectX::XMFLOAT3& colorApex)
{
    cbpsRareChanged_.data.colorCenter_ = colorCenter;
    cbpsRareChanged_.data.colorApex_   = colorApex;
    cbpsRareChanged_.ApplyChanges(pContext);
}

//---------------------------------------------------------
// Desc:   setup only the sky horizon color for the gradient
// Args:   - color:   sky horizon color
//---------------------------------------------------------
void SkyDomeShader::SetSkyColorCenter(ID3D11DeviceContext* pContext, const DirectX::XMFLOAT3& color)
{
    SetSkyGradient(pContext, color, cbpsRareChanged_.data.colorApex_);
}

//---------------------------------------------------------
// Desc:   setup only the sky apex(top) color for the gradient
// Args:   - color:   sky apex color
//---------------------------------------------------------
void SkyDomeShader::SetSkyColorApex(ID3D11DeviceContext* pContext, const DirectX::XMFLOAT3& color)
{
    SetSkyGradient(pContext, cbpsRareChanged_.data.colorCenter_, color);
}

} // namespace Render
