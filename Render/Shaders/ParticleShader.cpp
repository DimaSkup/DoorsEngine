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

    // bind vertex buffer
    UINT offset = 0;
    pContext->IASetVertexBuffers(0, 1, &pVB, &stride, &offset);
}

//---------------------------------------------------------
// Desc:   render an array of points as billboards
// Args:   - pContext:         DirectX11 device context
//         - baseVertex:       start rendering from this vertex idx
//         - numVertices:      how many vertices will we render as billboards
//---------------------------------------------------------
void ParticleShader::Render(
    ID3D11DeviceContext* pContext,
    const UINT baseVertex,
    const UINT numVertices)
{
    // bind the geometry shader
    pContext->GSSetShader(gs_.GetShader(), nullptr, 0);

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
    const InputLayoutParticleShader inputLayout;

    // initialize: VS, PS, sampler state
    result = vs_.Initialize(pDevice, vsFilePath, inputLayout.desc, inputLayout.numElems);
    CAssert::True(result, "can't initialize the vertex shader");

    result = gs_.Initialize(pDevice, gsFilePath);
    CAssert::True(result, "can't initialize the geometry shader");

    result = ps_.Initialize(pDevice, psFilePath);
    CAssert::True(result, "can't initialize the pixel shader");
}

//---------------------------------------------------------
// Desc:   recompile hlsl shaders from file and reinit shader class object
// Args:   - pDevice:      a ptr to the DirectX11 device
//         - vsFilePath:   a path to the vertex shader
//         - gsFilePath:   a path to the geometry shader
//         - psFilePath:   a path to the pixel shader
//---------------------------------------------------------
void ParticleShader::ShaderHotReload(
    ID3D11Device* pDevice,
    const char* vsFilePath,
    const char* gsFilePath,
    const char* psFilePath)
{
    bool result = false;
    const InputLayoutParticleShader inputLayout;

    result = vs_.CompileShaderFromFile(
        pDevice,
        vsFilePath,
        "VS", "vs_5_0",
        inputLayout.desc,
        inputLayout.numElems);
    CAssert::True(result, "can't hot reload the vertex shader");

    result = gs_.CompileShaderFromFile(pDevice, gsFilePath, "GS", "gs_5_0");
    CAssert::True(result, "can't hot reload the geometry shader");

    result = ps_.CompileShaderFromFile(pDevice, psFilePath, "PS", "ps_5_0");
    CAssert::True(result, "can't hot reload the vertex shader");
}

} // namespace Render
