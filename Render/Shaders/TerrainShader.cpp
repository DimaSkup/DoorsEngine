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

// --------------------------------------------------------
// Desc:   Initialize the shader class, load HLSL shaders
//         from files, create shader objects, etc.
// Args:   - pDevice: pointer to the DirectX device
//         - vsFilename: path to the vertex shader hlsl-file
//         - psFilename: path to the pixel shader hlsl-file
// --------------------------------------------------------
bool TerrainShader::Initialize(
    ID3D11Device* pDevice,
    const char* vsFilename,
    const char* psFilename)
{
    try
    {
        CAssert::True(!StrHelper::IsEmpty(vsFilename), "input vertex shader filename is empty");
        CAssert::True(!StrHelper::IsEmpty(psFilename), "input pixel shader filename is empty");

        InitializeShaders(pDevice, vsFilename, psFilename);
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

// --------------------------------------------------------
// Desc:   terrain rendering method
// Args:   - pContext: DirectX device context
//         - instance: a container which holds all the necessary
//                     data for terrain rendering
// --------------------------------------------------------
void TerrainShader::Render(
    ID3D11DeviceContext* pContext,
    const TerrainInstance& instance)
{
    // bind vertex and pixel shaders
    pContext->VSSetShader(vs_.GetShader(), nullptr, 0U);
    pContext->IASetInputLayout(vs_.GetInputLayout());
    pContext->PSSetShader(ps_.GetShader(), nullptr, 0U);

    // set the sampler state and textures
    pContext->PSSetSamplers(1U, 1U, samplerState_.GetAddressOf());
    pContext->PSSetShaderResources(10U, NUM_TEXTURE_TYPES, instance.textures);

    // bind vb/ib
    const UINT stride = instance.vertexStride;
    const UINT offset = 0;

    pContext->IASetVertexBuffers(0, 1, &instance.pVB, &stride, &offset);
    pContext->IASetIndexBuffer(instance.pIB, DXGI_FORMAT_R32_UINT, 0U);

    // setup the material
    cbpsMaterialData_.data.ambient  = instance.material.ambient_;
    cbpsMaterialData_.data.diffuse  = instance.material.diffuse_;
    cbpsMaterialData_.data.specular = instance.material.specular_;
    cbpsMaterialData_.data.reflect  = instance.material.reflect_;
    cbpsMaterialData_.ApplyChanges(pContext);

    pContext->PSSetConstantBuffers(5, 1, cbpsMaterialData_.GetAddressOf());

    // render geometry
    pContext->DrawIndexed(instance.indexCount, 0U, 0U);
    //pContext->Draw(instance.numVertices, 0);
}

void TerrainShader::Prepare(
    ID3D11DeviceContext* pContext,
    const TerrainInstance& instance)
{
    // bind vertex and pixel shaders
    pContext->VSSetShader(vs_.GetShader(), nullptr, 0U);
    pContext->IASetInputLayout(vs_.GetInputLayout());
    pContext->PSSetShader(ps_.GetShader(), nullptr, 0U);

    // set the sampler state and textures
    pContext->PSSetSamplers(1U, 1U, samplerState_.GetAddressOf());
    pContext->PSSetShaderResources(10U, NUM_TEXTURE_TYPES, instance.textures);

    // bind vb/ib
    const UINT stride = instance.vertexStride;
    const UINT offset = 0;

    pContext->IASetVertexBuffers(0, 1, &instance.pVB, &stride, &offset);
    pContext->IASetIndexBuffer(instance.pIB, DXGI_FORMAT_R32_UINT, 0U);

    // setup the material
    cbpsMaterialData_.data.ambient = instance.material.ambient_;
    cbpsMaterialData_.data.diffuse = instance.material.diffuse_;
    cbpsMaterialData_.data.specular = instance.material.specular_;
    cbpsMaterialData_.data.reflect = instance.material.reflect_;
    cbpsMaterialData_.ApplyChanges(pContext);

    pContext->PSSetConstantBuffers(5, 1, cbpsMaterialData_.GetAddressOf());
}

void TerrainShader::RenderPatch(
    ID3D11DeviceContext* pContext,
    const TerrainInstance& instance)
{
    // render geometry
    pContext->DrawIndexed(instance.indexCount, instance.baseIndex, instance.baseVertex);
    //pContext->Draw(instance.numVertices, 0);
}


// --------------------------------------------------------
// Desc:   render terrain's vertices onto the screen
//         (NOTE: we don't use any indices here)
// Args:   - pContext: a ptr to DirectX11 device context
//         - instance: container of necessary data for rendering
// --------------------------------------------------------
void TerrainShader::RenderVertices(
    ID3D11DeviceContext* pContext,
    const TerrainInstance& instance)
{
    // bind vertex and pixel shaders
    pContext->VSSetShader(vs_.GetShader(), nullptr, 0U);
    pContext->IASetInputLayout(vs_.GetInputLayout());
    pContext->PSSetShader(ps_.GetShader(), nullptr, 0U);

    // set the sampler state and textures
    pContext->PSSetSamplers(1U, 1U, samplerState_.GetAddressOf());
    pContext->PSSetShaderResources(1U, NUM_TEXTURE_TYPES, instance.textures);

    // bind vertex buffer
    constexpr UINT offset = 0;

    pContext->IASetVertexBuffers(0, 1, &instance.pVB, &instance.vertexStride, &offset);

    // setup the material
    cbpsMaterialData_.data.ambient  = instance.material.ambient_;
    cbpsMaterialData_.data.diffuse  = instance.material.diffuse_;
    cbpsMaterialData_.data.specular = instance.material.specular_;
    cbpsMaterialData_.data.reflect  = instance.material.reflect_;
    cbpsMaterialData_.ApplyChanges(pContext);

    pContext->PSSetConstantBuffers(5, 1, cbpsMaterialData_.GetAddressOf());

    // render geometry
    pContext->Draw(instance.numVertices, 0);
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

    result = vs_.CompileShaderFromFile(
        pDevice,
        vsFilename,
        "VS", "vs_5_0",
        layout.desc,
        layout.numElems);
    CAssert::True(result, "can't hot reload the vertex shader");

    result = ps_.CompileShaderFromFile(pDevice, psFilename, "PS", "ps_5_0");
    CAssert::True(result, "can't hot reload the vertex shader");
}


// --------------------------------------------------------
// Desc:   a helper for initialization process
// Args:   - pDevice: pointer to the DirectX device
//         - vsFilename: path to the vertex shader hlsl-file
//         - psFilename: path to the pixel shader hlsl-file
// --------------------------------------------------------
void TerrainShader::InitializeShaders(
    ID3D11Device* pDevice,
    const char* vsFilename,
    const char* psFilename)
{
    HRESULT hr = S_OK;
    bool result = false;

    // initialize: VS, PS, sampler state, constant buffers
    const InputLayoutTerrain layout;

    result = vs_.Initialize(pDevice, vsFilename, layout.desc, layout.numElems);
    CAssert::True(result, "can't initialize the vertex shader");

    result = ps_.Initialize(pDevice, psFilename);
    CAssert::True(result, "can't initialize the pixel shader");

    result = samplerState_.Initialize(pDevice);
    CAssert::True(result, "can't initialize the sampler state");

    // cbps - const buffer for pixel shader
    hr = cbpsMaterialData_.Initialize(pDevice);
    CAssert::NotFailed(hr, "can't initialize a constant buffer of materials");
}

} // namespace
