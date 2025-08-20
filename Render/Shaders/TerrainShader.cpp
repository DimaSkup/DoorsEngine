// =================================================================================
// Filename:    TerrainShader.cpp
// Created:     30.05.2025 by DimaSkup
// =================================================================================
#include "../Common/pch.h"
#include "TerrainShader.h"

namespace Render
{


TerrainShader::TerrainShader()
{
    strcpy(className_, __func__);
}

TerrainShader::~TerrainShader()
{
}

//---------------------------------------------------------
// Decs:   Load CSO / compile HLSL shaders and init this shader class instance
// Args:   - vsPath:  a path to compiled (.cso) vertex shader file
//         - psPath:  a path to compiled (.cso) pixel shader file
//---------------------------------------------------------
bool TerrainShader::Initialize(
    ID3D11Device* pDevice,
    const char* vsPath,
    const char* psPath)
{
    try
    {
        CAssert::True(!StrHelper::IsEmpty(vsPath), "input path to vertex shader is empty");
        CAssert::True(!StrHelper::IsEmpty(psPath), "input path to pixel shader is empty");

        HRESULT hr = S_OK;
        bool result = false;

        // initialize: VS, PS, sampler state, constant buffers
        const InputLayoutTerrain layout;

        result = vs_.LoadPrecompiled(pDevice, vsPath, layout.desc, layout.numElems);
        CAssert::True(result, "can't initialize the vertex shader");

        result = ps_.LoadPrecompiled(pDevice, psPath);
        CAssert::True(result, "can't initialize the pixel shader");

        // cbps - const buffer for pixel shader
        hr = cbpsMaterialData_.Initialize(pDevice);
        CAssert::NotFailed(hr, "can't initialize a constant buffer of materials");


        LogDbg(LOG, "is initialized");
        return true;
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        LogErr(LOG, "can't initialize the terrain shader class");
        return false;
    }
}
//---------------------------------------------------------
// Desc:   execute some preparation before terrain's patches rendering
//---------------------------------------------------------
void TerrainShader::Prepare(
    ID3D11DeviceContext* pContext,
    const TerrainInstance& instance)
{
    // bind vertex and pixel shaders
    pContext->VSSetShader(vs_.GetShader(), nullptr, 0U);
    pContext->IASetInputLayout(vs_.GetInputLayout());
    pContext->PSSetShader(ps_.GetShader(), nullptr, 0U);

    // bind vb/ib
    const UINT stride = instance.vertexStride;
    const UINT offset = 0;

    pContext->IASetVertexBuffers(0, 1, &instance.pVB, &stride, &offset);
    pContext->IASetIndexBuffer(instance.pIB, DXGI_FORMAT_R32_UINT, 0U);

    // setup the material
    cbpsMaterialData_.data.ambient  = instance.matColors.ambient;
    cbpsMaterialData_.data.diffuse  = instance.matColors.diffuse;
    cbpsMaterialData_.data.specular = instance.matColors.specular;
    cbpsMaterialData_.data.reflect  = instance.matColors.reflect;
    cbpsMaterialData_.ApplyChanges(pContext);

    pContext->PSSetConstantBuffers(5, 1, cbpsMaterialData_.GetAddressOf());
}

// --------------------------------------------------------
// render geometry of a single terrain patch
// --------------------------------------------------------
void TerrainShader::RenderPatch(
    ID3D11DeviceContext* pContext,
    const TerrainInstance& instance)
{
    pContext->DrawIndexed(instance.indexCount, instance.baseIndex, instance.baseVertex);
}

// --------------------------------------------------------
// Desc:   reload of shaders without reloading of the engine :)
// Args:   - pDevice: pointer to the DirectX device
//         - vsFilename: path to the vertex shader hlsl-file
//         - psFilename: path to the pixel shader hlsl-file
// --------------------------------------------------------
void TerrainShader::ShaderHotReload(
    ID3D11Device* pDevice,
    const char* vsFilename,
    const char* psFilename)
{
    bool result = false;
    const InputLayoutTerrain layout;

    result = vs_.CompileFromFile(
        pDevice,
        vsFilename,
        "VS", "vs_5_0",
        layout.desc,
        layout.numElems);
    CAssert::True(result, "can't hot reload the vertex shader");

    result = ps_.CompileFromFile(pDevice, psFilename, "PS", "ps_5_0");
    CAssert::True(result, "can't hot reload the vertex shader");
}

} // namespace
