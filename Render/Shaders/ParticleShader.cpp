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
// Decs:   Load CSO / compile HLSL shaders and init this shader class instance
// Args:   - vsPath:  a path to compiled (.cso) vertex shader file
//         - gsPath:  a path to compiled (.cso) geometry shader file
//         - psPath:  a path to compiled (.cso) pixel shader file
//---------------------------------------------------------
bool ParticleShader::Initialize(
    ID3D11Device* pDevice,
    const char* vsPath,
    const char* psPath,
    const char* gsPath)
{
    try
    {
        CAssert::True(!StrHelper::IsEmpty(vsPath), "path to vertex shader is empty");
        CAssert::True(!StrHelper::IsEmpty(gsPath), "path to geometry shader is empty");
        CAssert::True(!StrHelper::IsEmpty(psPath), "path to pixel shader is empty");

        bool result = false;
        const InputLayoutParticleShader layout;

        // init vertex, geometry, pixel shaders
        result = vs_.LoadPrecompiled(pDevice, vsPath, layout.desc, layout.numElems);
        CAssert::True(result, "can't initialize the vertex shader");

        result = gs_.LoadPrecompiled(pDevice, gsPath);
        CAssert::True(result, "can't initialize the geometry shader");

        result = ps_.LoadPrecompiled(pDevice, psPath);
        CAssert::True(result, "can't initialize the pixel shader");

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
// Desc:   recompile hlsl shaders from file and reinit shader class object
// Args:   - pDevice:      a ptr to the DirectX11 device
//         - vsPath:   a path to the vertex shader
//         - gsPath:   a path to the geometry shader
//         - psPath:   a path to the pixel shader
//---------------------------------------------------------
void ParticleShader::ShaderHotReload(
    ID3D11Device* pDevice,
    const char* vsPath,
    const char* gsPath,
    const char* psPath)
{
    bool result = false;
    const InputLayoutParticleShader layout;

    result = vs_.CompileFromFile(pDevice, vsPath, "VS", "vs_5_0", layout.desc, layout.numElems);
    CAssert::True(result, "can't hot reload the vertex shader");

    result = gs_.CompileFromFile(pDevice, gsPath, "GS", "gs_5_0");
    CAssert::True(result, "can't hot reload the geometry shader");

    result = ps_.CompileFromFile(pDevice, psPath, "PS", "ps_5_0");
    CAssert::True(result, "can't hot reload the pixel shader");
}

} // namespace Render
