// =================================================================================
// Filename:     LightShader.cpp
// Created:      09.04.23
// =================================================================================
#include "../Common/pch.h"
#include "../Common/InputLayouts.h"
#include "LightShader.h"


namespace Render
{

LightShader::LightShader()
{
    strcpy(className_, __func__);
}

LightShader::~LightShader()
{
}

//---------------------------------------------------------
// Desc:    load CSO / compile HLSL shaders and init the shader class object
//---------------------------------------------------------
bool LightShader::Initialize(
    ID3D11Device* pDevice,
    const char* vsFilePath,
    const char* psFilePath)
{
    bool result = false;
    const InputLayoutLight inputLayout;

    // init vertex shader
    result = vs_.Initialize(pDevice, vsFilePath, inputLayout.desc, inputLayout.numElems);
    if (!result)
    {
        LogErr(LOG, "can't initialize the vertex shader");
        return false;
    }

    // init pixel shader
    result = ps_.Initialize(pDevice, psFilePath);
    if (!result)
    {
        LogErr(LOG, "can't initialize the pixel shader");
        return false;
    }

    LogDbg(LOG, "is initialized");
    return true;
}

//---------------------------------------------------------
// Desc:   render instances starting from startInstanceLocation
//         in the instances buffer
//---------------------------------------------------------
void LightShader::Render(
    ID3D11DeviceContext* pContext,
    ID3D11Buffer* pInstancedBuffer,
    const InstanceBatch& instances,
    const UINT instancesBuffElemSize,
    const UINT startInstanceLocation)
{
    // bind input layout, shaders, samplers
    pContext->IASetInputLayout(vs_.GetInputLayout());
    pContext->VSSetShader(vs_.GetShader(), nullptr, 0);
    pContext->PSSetShader(ps_.GetShader(), nullptr, 0);


    // bind vertex/index buffers
    ID3D11Buffer* const vbs[2] = { instances.pVB, pInstancedBuffer };
    const UINT stride[2]       = { instances.vertexStride, instancesBuffElemSize };
    const UINT offset[2]       = { 0,0 };

    pContext->IASetVertexBuffers(0, 2, vbs, stride, offset);
    pContext->IASetIndexBuffer(instances.pIB, DXGI_FORMAT_R32_UINT, 0);

    const Subset& subset = instances.subset;

    pContext->DrawIndexedInstanced(
        subset.indexCount,
        instances.numInstances,
        subset.indexStart,
        subset.vertexStart,
        startInstanceLocation);
}

//---------------------------------------------------------
// Desc:   recompile hlsl shaders from file and reinit shader class object
//---------------------------------------------------------
void LightShader::ShaderHotReload(
    ID3D11Device* pDevice,
    const char* vsFilePath,
    const char* psFilePath)
{
    bool result = false;
    const InputLayoutLight inputLayout;

    result = vs_.CompileShaderFromFile(
        pDevice,
        vsFilePath,
        "VS", "vs_5_0",
        inputLayout.desc,
        inputLayout.numElems);
    if (!result)
    {
        LogErr(LOG, "can't hot reload the vertex shader");
        return;
    }

    result = ps_.CompileShaderFromFile(pDevice, psFilePath, "PS", "ps_5_0");
    if (!result)
    {
        LogErr(LOG, "can't hot reload the vertex shader");
        return;
    }

}

} // namespace Render
