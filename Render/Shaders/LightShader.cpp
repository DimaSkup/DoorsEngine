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
// Decs:   Load CSO / compile HLSL shaders and init this shader class instance
// Args:   - vsPath:  a path to compiled (.cso) vertex shader file
//         - psPath:  a path to compiled (.cso) pixel shader file
//---------------------------------------------------------
bool LightShader::Initialize(
    ID3D11Device* pDevice,
    const char* vsPath,
    const char* psPath)
{
    try
    {
        CAssert::True(!StrHelper::IsEmpty(vsPath), "input path to vertex shader is empty");
        CAssert::True(!StrHelper::IsEmpty(psPath), "input path to pixel shader is empty");

        bool result = false;
        const InputLayoutLight inputLayout;

        // init vertex shader
        result = vs_.LoadPrecompiled(pDevice, vsPath, inputLayout.desc, inputLayout.numElems);
        CAssert::True(result, "can't initialize the vertex shader");

        // init pixel shader
        result = ps_.LoadPrecompiled(pDevice, psPath);
        CAssert::True(result, "can't initialize the pixel shader");

        LogDbg(LOG, "is initialized");
        return true;
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        LogErr(LOG, "can't init the light shader class");
        return false;
    }
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
// Args:   - pDevice:      a ptr to the DirectX11 device
//         - vsPath:   a path to the vertex shader
//         - psPath:   a path to the pixel shader
//---------------------------------------------------------
void LightShader::ShaderHotReload(
    ID3D11Device* pDevice,
    const char* vsPath,
    const char* psPath)
{
    bool result = false;
    const InputLayoutLight layout;

    result = vs_.CompileFromFile(pDevice, vsPath, "VS", "vs_5_0", layout.desc, layout.numElems);
    CAssert::True(result, "can't hot reload the vertex shader");

    result = ps_.CompileFromFile(pDevice, psPath, "PS", "ps_5_0");
    CAssert::True(result, "can't hot reload the pixel shader");
}

} // namespace Render
