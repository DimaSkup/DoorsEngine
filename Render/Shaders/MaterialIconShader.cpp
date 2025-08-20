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

//---------------------------------------------------------
// Decs:   Load CSO / compile HLSL shaders and init this shader class instance
// Args:   - vsPath:  a path to compiled (.cso) vertex shader file
//         - psPath:  a path to compiled (.cso) pixel shader file
//---------------------------------------------------------
bool MaterialIconShader::Initialize(
    ID3D11Device* pDevice,
    const char* vsPath,
    const char* psPath)
{
    try
    {
        CAssert::True(!StrHelper::IsEmpty(vsPath), "input path to vertex shader is empty");
        CAssert::True(!StrHelper::IsEmpty(psPath), "input path to pixel shader is empty");

        bool result = false;
        const InputLayoutMaterialIcon layout;

        // initialize: vertex/pixel shader, and const buffers
        result = vs_.LoadPrecompiled(pDevice, vsPath, layout.desc, layout.numElems);
        CAssert::True(result, "can't initialize the vertex shader");

        result = ps_.LoadPrecompiled(pDevice, psPath);
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

        return true;
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        LogErr(LOG, "can't initialize the %s", className_);
        return false;
    }
}

//---------------------------------------------------------
// Desc:    bind buffers/shaders/input layout before rendering
// Args:    - pVB:         a ptr to vertex buffer
//          - pIB:         a ptr to index buffer
//          - vertexSize:  stride for a single vertex
//---------------------------------------------------------
void MaterialIconShader::PrepareRendering(
    ID3D11DeviceContext* pContext,
    ID3D11Buffer* pVB,
    ID3D11Buffer* pIB,
    const int vertexSize)
{
    try
    {
        CAssert::True(pVB,            "input ptr to vertex buffer == nullptr");
        CAssert::True(pIB,            "input ptr to index buffer == nullptr");
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
        pContext->IASetVertexBuffers(0, 1, &pVB, &stride, &offset);
        pContext->IASetIndexBuffer(pIB, DXGI_FORMAT_R32_UINT, 0U);

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

//---------------------------------------------------------
// Desc:   render a model (preferably sphere) with a single material to visualize it
// Args:   - indexCount:  how many indices we have for the current model
//         - mat:         color properties of the material (ambient, diffuse, etc.)
//---------------------------------------------------------
void MaterialIconShader::Render(
    ID3D11DeviceContext* pContext,
    const int indexCount,
    ID3D11ShaderResourceView* const* ppTextures,
    const Render::MaterialColors& mat)
{
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

//-----------------------------------------------
//-----------------------------------------------
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

//---------------------------------------------------------
// Desc:   recompile hlsl shaders from file and reinit shader class object
// Args:   - pDevice:  a ptr to the DirectX11 device
//         - vsPath:   a path to the vertex shader
//         - gsPath:   a path to the geometry shader
//         - psPath:   a path to the pixel shader
//---------------------------------------------------------
void MaterialIconShader::ShaderHotReload(
    ID3D11Device* pDevice,
    const char* vsPath,
    const char* psPath)
{
    // recompile hlsl shaders from file and reinit shader class object

    bool result = false;
    const InputLayoutMaterialIcon layout;

    result = vs_.CompileFromFile(pDevice, vsPath, "VS", "vs_5_0", layout.desc, layout.numElems);
    CAssert::True(result, "can't hot reload the vertex shader");

    result = ps_.CompileFromFile(pDevice, psPath, "PS", "ps_5_0");
    CAssert::True(result, "can't hot reload the vertex shader");
}

} // namespace Render
