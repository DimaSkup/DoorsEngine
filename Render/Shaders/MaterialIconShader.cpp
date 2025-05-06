// =================================================================================
// Filename:     MaterialIconShader.cpp
//
// Created       30.04.2025 by DimaSkup
// =================================================================================
#include "MaterialIconShader.h"
#include "../Common/Log.h"
#include "../Common/FileSystem.h"
#include "../Common/Assert.h"
#include "../Common/Types.h"

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
    catch (LIB_Exception& e)
    {
        LogErr(e, true);
        sprintf(g_String, "can't initialize the %s", className_);
        LogErr(g_String);
        return false;
    }
}

///////////////////////////////////////////////////////////

void MaterialIconShader::Render(
    ID3D11DeviceContext* pContext,
    ID3D11Buffer* vertexBuffer,
    ID3D11Buffer* indexBuffer,
    const int indexCount,
    ID3D11ShaderResourceView* const* ppTextures,
    const int vertexSize,
    const Render::Material& mat)
{
    // render a model (preferably sphere) with a single material
    try
    {
        Assert::True(vertexBuffer,   "input ptr to vertex buffer == nullptr");
        Assert::True(indexBuffer,    "input ptr to index buffer == nullptr");
        Assert::True(indexCount > 0, "input number of indices must be > 0");
        Assert::True(ppTextures,     "input ptr to arr of textures == nullptr");
        Assert::True(vertexSize > 0, "input size of vertex must be > 0");

        // bind vertex and pixel shaders
        pContext->VSSetShader(vs_.GetShader(), nullptr, 0U);
        pContext->PSSetShader(ps_.GetShader(), nullptr, 0U);

        // set the primitive topology and input layout
        pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        pContext->IASetInputLayout(vs_.GetInputLayout());

        // set the sampler state and textures
        pContext->PSSetSamplers(0, 1, samplerState_.GetAddressOf());
        pContext->PSSetShaderResources(0, NUM_TEXTURE_TYPES, ppTextures);

        const UINT stride = vertexSize;
        const UINT offset = 0;

        // bind vb/ib
        pContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
        pContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0U);

        // setup the material
        cbpsMaterialData_.data.ambient  = mat.ambient_;
        cbpsMaterialData_.data.diffuse  = mat.diffuse_;
        cbpsMaterialData_.data.specular = mat.specular_;
        cbpsMaterialData_.data.reflect  = mat.reflect_;
        cbpsMaterialData_.ApplyChanges(pContext);
        pContext->PSSetConstantBuffers(5, 1, cbpsMaterialData_.GetAddressOf());

        // render geometry
        pContext->DrawIndexed(indexCount, 0U, 0U);
    }
    catch (LIB_Exception& e)
    {
        LogErr(e, true);
        LogErr("can't render using the shader");
        return;
    }
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

    const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
    {
        // per vertex data
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},

        // per instance data
        //{"MATERIAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        //{"MATERIAL", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        //{"MATERIAL", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        //{"MATERIAL", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
    };

    const UINT layoutElemNum = sizeof(inputLayoutDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC);

    // initialize: VS, PS, texture sampler
    bool result = false;
    result = vs_.Initialize(pDevice, vsFilePath, inputLayoutDesc, layoutElemNum);
    Assert::True(result, "can't initialize the vertex shader");

    result = ps_.Initialize(pDevice, psFilePath);
    Assert::True(result, "can't initialize the pixel shader");

    result = samplerState_.Initialize(pDevice);
    Assert::True(result, "can't initialize the sampler state");

    HRESULT hr = cbpsMaterialData_.Initialize(pDevice);
    Assert::NotFailed(hr, "can't initialize the const buffer (for material in PS)");

    // apply the default params for the constant buffer
    ID3D11DeviceContext* pContext = nullptr;
    pDevice->GetImmediateContext(&pContext);
    cbpsMaterialData_.ApplyChanges(pContext);
}

} // namespace Render
