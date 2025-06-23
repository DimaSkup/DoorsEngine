// =================================================================================
// Filename:     LightShader.cpp
// Description:  this class is needed for rendering 3D models, 
//               its texture, simple PARRALEL light on it using HLSL shaders.
// Created:      09.04.23
// =================================================================================
#include "../Common/pch.h"
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

///////////////////////////////////////////////////////////

bool LightShader::Initialize(
    ID3D11Device* pDevice,
    const char* vsFilePath,
    const char* psFilePath)
{
    try
    {
        InitializeShaders(pDevice, vsFilePath, psFilePath);
        LogDbg("is initialized");
        return true;
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        LogErr("can't initialize the light shader class");
        return false;
    }
}

///////////////////////////////////////////////////////////

void LightShader::Render(
    ID3D11DeviceContext* pContext,
    ID3D11Buffer* pInstancedBuffer,
    const Instance* instances,
    const int numUniqueGeometry,       // aka the number of unique models
    const UINT instancesBuffElemSize)
{
    // bind input layout, shaders, samplers
    pContext->IASetInputLayout(vs_.GetInputLayout());
    pContext->VSSetShader(vs_.GetShader(), nullptr, 0);
    pContext->PSSetShader(ps_.GetShader(), nullptr, 0);
    pContext->PSSetSamplers(0, 1, samplerState_.GetAddressOf());


    // go through each instance and render it
    for (int i = 0, startInstanceLocation = 0; i < numUniqueGeometry; ++i)
    {
        const Instance& instance = instances[i];

        // bind vertex/index buffers
        ID3D11Buffer* const vbs[2] = { instance.pVB, pInstancedBuffer };
        const UINT stride[2] = { instance.vertexStride, instancesBuffElemSize };
        const UINT offset[2] = { 0,0 };

        pContext->IASetVertexBuffers(0, 2, vbs, stride, offset);
        pContext->IASetIndexBuffer(instance.pIB, DXGI_FORMAT_R32_UINT, 0);

        // textures arr
        ID3D11ShaderResourceView* const* texSRVs = instance.texSRVs.data();
        const int numSubsets = (int)std::ssize(instance.subsets);

        // go through each subset (mesh) of this model and render it
        for (int subsetIdx = 0; subsetIdx < numSubsets; ++subsetIdx)
        {
            // update textures for the current subset
            pContext->PSSetShaderResources(
                1U,
                NUM_TEXTURE_TYPES,
                texSRVs + (subsetIdx * NUM_TEXTURE_TYPES));  // texture_buffer_begin + offset

            const Subset& subset = instance.subsets[subsetIdx];

            pContext->DrawIndexedInstanced(
                subset.indexCount,
                instance.numInstances,
                subset.indexStart,
                subset.vertexStart,
                startInstanceLocation + (subsetIdx * instance.numInstances));
        }

        startInstanceLocation += numSubsets * instance.numInstances;
    }
}


// =================================================================================
//                              private methods                                       
// =================================================================================
void LightShader::InitializeShaders(
    ID3D11Device* pDevice,
    const char* vsFilePath,
    const char* psFilePath)
{
    // helps to initialize the HLSL shaders, layout, sampler state

    const InputLayoutLight inputLayout;

    // initialize: VS, PS, sampler state
    bool result = false;
    result = vs_.Initialize(pDevice, vsFilePath, inputLayout.desc, inputLayout.numElem);
    CAssert::True(result, "can't initialize the vertex shader");

    result = ps_.Initialize(pDevice, psFilePath);
    CAssert::True(result, "can't initialize the pixel shader");

    result = samplerState_.Initialize(pDevice);
    CAssert::True(result, "can't initialize the sampler state");
}

///////////////////////////////////////////////////////////

void LightShader::ShaderHotReload(
    ID3D11Device* pDevice,
    const char* vsFilePath,
    const char* psFilePath)
{
    // recompile hlsl shaders from file and reinit shader class object

    bool result = false;
    const InputLayoutLight inputLayout;

    result = vs_.CompileShaderFromFile(
        pDevice,
        vsFilePath,
        "VS", "vs_5_0",
        inputLayout.desc,
        inputLayout.numElem);
    CAssert::True(result, "can't hot reload the vertex shader");

    result = ps_.CompileShaderFromFile(pDevice, psFilePath, "PS", "ps_5_0");
    CAssert::True(result, "can't hot reload the vertex shader");
}

} // namespace Render
