// ====================================================================================
// Filename: ParticleShader.cpp
// ====================================================================================
#include "../Common/pch.h"
#include "ParticleShader.h"

#pragma warning (disable : 4996)


namespace Render
{

ParticleShader::ParticleShader()
{
    strcpy(className_, __func__);
}

ParticleShader::~ParticleShader()
{
}

//---------------------------------------------------------
// Desc:   initialize the particle shader: setup hlsl shaders/sampler_state
// Args:   - vsFilePath, psFilePath, gsFilePath: paths to shaders (vertex, pixel, geometry)
// Ret:    true if we managed to init successfully
//---------------------------------------------------------
bool ParticleShader::Initialize(
    ID3D11Device* pDevice,
    const char* vsFilePath,
    const char* psFilePath,
    const char* gsFilePath)
{
    try
    {
        CAssert::True(!StrHelper::IsEmpty(vsFilePath), "input vertex shader filepath is empty");
        CAssert::True(!StrHelper::IsEmpty(psFilePath), "input pixel shader filepath is empty");
        CAssert::True(!StrHelper::IsEmpty(gsFilePath), "input geometry shader filepath is empty");

        InitializeShaders(pDevice, vsFilePath, psFilePath, gsFilePath);
        LogDbg(LOG, "is initialized");
        return true;
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        LogErr(LOG, "can't initialize the texture shader class");
        return false;
    }
}

//---------------------------------------------------------
// Desc:   prepare the shader for rendering (bind all necessary stuff)
// Args:   - pContext:         DirectX11 device context
//         - pVB:              a pointer to the vertex buffer
//         - stride:           size in bytes of a single vertex
//---------------------------------------------------------
void ParticleShader::Prepare(
    ID3D11DeviceContext* pContext,
    ID3D11Buffer* pVB,
    const UINT stride)
{
    pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    // bind input layout, shaders, samplers, const buffers
    pContext->VSSetShader(vs_.GetShader(), nullptr, 0);
    pContext->IASetInputLayout(vs_.GetInputLayout());
    pContext->PSSetShader(ps_.GetShader(), nullptr, 0);
    pContext->PSSetSamplers(0, 1, samplerState_.GetAddressOf());
    pContext->VSSetConstantBuffers(4, 1, cbvsParticlesOffset_.GetAddressOf());

    // bind vertex buffer
    UINT offset = 0;
    pContext->IASetVertexBuffers(0, 1, &pVB, &stride, &offset);
}

//---------------------------------------------------------
// Desc:   render an array of points as billboards
// Args:   - pContext:         DirectX11 device context
//         - posOffset:        position offset for each particle
//         - baseVertex:       start rendering from this vertex idx
//         - numVertices:      how many vertices will we render as billboards
//---------------------------------------------------------
void ParticleShader::Render(
    ID3D11DeviceContext* pContext,
    const DirectX::XMFLOAT3& posOffset,
    const UINT baseVertex,
    const UINT numVertices)
{
    // bind the geometry shader
    pContext->GSSetShader(gs_.GetShader(), nullptr, 0);

    // setup the particles position offset
    cbvsParticlesOffset_.data.posW = posOffset;
    cbvsParticlesOffset_.ApplyChanges(pContext);

    // draw particles set
    pContext->Draw(numVertices, baseVertex);

    // unbind the geometry shader
    pContext->GSSetShader(nullptr, nullptr, 0);
}

//---------------------------------------------------------
// Desc:    initialization helper
//---------------------------------------------------------
void ParticleShader::InitializeShaders(
    ID3D11Device* pDevice,
    const char* vsFilePath,
    const char* psFilePath,
    const char* gsFilePath)
{
    bool result = false;

    const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
    {
        // per vertex data
        {"POSITION",      0, DXGI_FORMAT_R32G32B32_FLOAT, 0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TRANSLUCENCY",  0, DXGI_FORMAT_R32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR",         0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"SIZE",          0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    const UINT layoutElemNum = sizeof(inputLayoutDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC);

    // initialize: VS, PS, sampler state
    result = vs_.Initialize(pDevice, vsFilePath, inputLayoutDesc, layoutElemNum);
    CAssert::True(result, "can't initialize the vertex shader");

    result = gs_.Initialize(pDevice, gsFilePath);
    CAssert::True(result, "can't initialize the geometry shader");

    result = ps_.Initialize(pDevice, psFilePath);
    CAssert::True(result, "can't initialize the pixel shader");

    result = samplerState_.Initialize(pDevice);
    CAssert::True(result, "can't initialize the sampler state");

    HRESULT hr = cbvsParticlesOffset_.Initialize(pDevice);
    CAssert::NotFailed(hr, "can't init a const buffer for particles offset");


    // setup the const buffers with initial data
    ID3D11DeviceContext* pContext = nullptr;
    pDevice->GetImmediateContext(&pContext);

    cbvsParticlesOffset_.data.posW = { 0,0,0 };
    cbvsParticlesOffset_.ApplyChanges(pContext);
}

} // namespace Render
