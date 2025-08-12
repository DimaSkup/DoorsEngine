// =================================================================================
// Filename:     MaterialIconShader.cpp
//
// Created       30.04.2025 by DimaSkup
// =================================================================================
#include "../Common/pch.h"
#include "MaterialIconShader.h"

namespace Render
{

MaterialIconShader::MaterialIconShader()
{
    strcpy(className_, __func__);
}

MaterialIconShader::~MaterialIconShader()
{
}

///////////////////////////////////////////////////////////

bool CheckShadersPaths(const char* vsFilePath, const char* psFilePath)
{
    if (!FileSys::Exists(vsFilePath))
    {
        sprintf(g_String, "there is no vertex shader file: %s", vsFilePath);
        LogErr(g_String);
        return false;
    }
    if (!FileSys::Exists(psFilePath))
    {
        sprintf(g_String, "there is no pixel shader file: %s", psFilePath);
        LogErr(g_String);
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////

bool MaterialIconShader::Initialize(
    ID3D11Device* pDevice,
    const char* vsFilePath,
    const char* psFilePath)
{
    try
    {
        if (!CheckShadersPaths(vsFilePath, psFilePath))
            return false;

        InitializeHelper(pDevice, vsFilePath, psFilePath);

        return true;
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        sprintf(g_String, "can't initialize the %s", className_);
        LogErr(g_String);
        return false;
    }
}

///////////////////////////////////////////////////////////

void MaterialIconShader::PrepareRendering(
    ID3D11DeviceContext* pContext,
    ID3D11Buffer* vertexBuffer,
    ID3D11Buffer* indexBuffer,
    const int vertexSize)
{
    // bind buffers/shaders/input layout
    try
    {
        CAssert::True(vertexBuffer,   "input ptr to vertex buffer == nullptr");
        CAssert::True(indexBuffer,    "input ptr to index buffer == nullptr");
        CAssert::True(vertexSize > 0, "input size of vertex must be > 0");

        // bind vertex and pixel shaders
        pContext->VSSetShader(vs_.GetShader(), nullptr, 0U);
        pContext->PSSetShader(ps_.GetShader(), nullptr, 0U);

        // set the primitive topology and input layout
        pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        pContext->IASetInputLayout(vs_.GetInputLayout());

        const UINT stride = vertexSize;
        const UINT offset = 0;

        // bind vb/ib
        pContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
        pContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0U);

        // bind the constant buffer for vertex shader
        pContext->VSSetConstantBuffers(10, 1, cbvsWorldViewProj_.GetAddressOf());
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        LogErr("can't render using the shader");
        return;
    }
}

///////////////////////////////////////////////////////////

void MaterialIconShader::Render(
    ID3D11DeviceContext* pContext,
    const int indexCount,
    ID3D11ShaderResourceView* const* ppTextures,
    const Render::MaterialColors& mat)
{
    // render a model (preferably sphere) with a single material
    try
    {
        CAssert::True(indexCount > 0, "input number of indices must be > 0");
        CAssert::True(ppTextures,     "input ptr to arr of textures == nullptr");

        // setup textures
        pContext->PSSetShaderResources(0, NUM_TEXTURE_TYPES, ppTextures);

        // setup the material
        cbpsMaterialData_.data.ambient  = mat.ambient;
        cbpsMaterialData_.data.diffuse  = mat.diffuse;
        cbpsMaterialData_.data.specular = mat.specular;
        cbpsMaterialData_.data.reflect  = mat.reflect;
        cbpsMaterialData_.ApplyChanges(pContext);
        pContext->PSSetConstantBuffers(5, 1, cbpsMaterialData_.GetAddressOf());

        // render geometry
        pContext->DrawIndexed(indexCount, 0U, 0U);
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        LogErr("can't render using the shader");
        return;
    }
}

///////////////////////////////////////////////////////////

void MaterialIconShader::SetMatrix(
    ID3D11DeviceContext* pContext,
    const DirectX::XMMATRIX& world,
    const DirectX::XMMATRIX& view,
    const DirectX::XMMATRIX& proj)
{
    cbvsWorldViewProj_.data.world = world;
    cbvsWorldViewProj_.data.viewProj = DirectX::XMMatrixTranspose(view * proj);
    cbvsWorldViewProj_.ApplyChanges(pContext);
}

// =================================================================================
//                              private methods                                       
// =================================================================================
void MaterialIconShader::InitializeHelper(
    ID3D11Device* pDevice,
    const char* vsFilePath,
    const char* psFilePath)
{
    // helps to init the HLSL shaders, input layout, texture sampler

    const InputLayoutMaterialIcon inputLayout;

    // initialize: VS, PS, texture sampler
    bool result = false;
    result = vs_.Initialize(pDevice, vsFilePath, inputLayout.desc, inputLayout.numElems);
    CAssert::True(result, "can't initialize the vertex shader");

    result = ps_.Initialize(pDevice, psFilePath);
    CAssert::True(result, "can't initialize the pixel shader");

    // initialize: constant buffers
    HRESULT hr = cbvsWorldViewProj_.Initialize(pDevice);
    CAssert::NotFailed(hr, "can't initialize the const buffer (for world/view/proj in VS)");

    hr = cbpsMaterialData_.Initialize(pDevice);
    CAssert::NotFailed(hr, "can't initialize the const buffer (for material in PS)");


    // apply the default params for the constant buffers
    ID3D11DeviceContext* pContext = nullptr;
    pDevice->GetImmediateContext(&pContext);

    cbvsWorldViewProj_.ApplyChanges(pContext);
    cbpsMaterialData_.ApplyChanges(pContext);
}

///////////////////////////////////////////////////////////

void MaterialIconShader::ShaderHotReload(
    ID3D11Device* pDevice,
    const char* vsFilePath,
    const char* psFilePath)
{
    // recompile hlsl shaders from file and reinit shader class object

    bool result = false;
    const InputLayoutMaterialIcon inputLayout;

    result = vs_.CompileShaderFromFile(
        pDevice,
        vsFilePath,
        "VS", "vs_5_0",
        inputLayout.desc,
        inputLayout.numElems);
    CAssert::True(result, "can't hot reload the vertex shader");

    result = ps_.CompileShaderFromFile(pDevice, psFilePath, "PS", "ps_5_0");
    CAssert::True(result, "can't hot reload the vertex shader");
}

} // namespace Render
