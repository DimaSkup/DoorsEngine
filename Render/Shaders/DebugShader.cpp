// ********************************************************************************
// Filename:   DebugShader.cpp
// Created:    24.11.24
// ********************************************************************************
#include "../Common/pch.h"
#include "../Common/InputLayouts.h"
#include "DebugShader.h"
#pragma warning (disable : 4996)


namespace Render
{

DebugShader::DebugShader()
{
    strcpy(className_, __func__);
}

DebugShader::~DebugShader()
{
}

// *********************************************************************************
//                             PUBLIC METHODS                                       
// *********************************************************************************
bool DebugShader::Initialize(
    ID3D11Device* pDevice,
    const char* vsFilePath,
    const char* psFilePath)
{
    try
    {
        HRESULT hr = S_OK;
        bool result = false;
        InputLayoutDebugShader inputLayout;

        // init vertex shader
        result = vs_.Initialize(pDevice, vsFilePath, inputLayout.desc, inputLayout.numElems);
        CAssert::True(result, "can't initialize the vertex shader");

        // init pixel shader
        result = ps_.Initialize(pDevice, psFilePath);
        CAssert::True(result, "can't initialize the pixel shader");

        // rare changed const buffer (for debug)
        hr = cbpsRareChangedDebug_.Initialize(pDevice);
        CAssert::NotFailed(hr, "can't init a const buffer for debug (PS)");

        // setup const buffers with initial params
        ID3D11DeviceContext* pContext = nullptr;
        pDevice->GetImmediateContext(&pContext);

        // load rare changed data into GPU
        cbpsRareChangedDebug_.ApplyChanges(pContext);
        LogDbg(LOG, "is initialized");
        return true;
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        LogErr(LOG, "can't initialize the debug vector shader");
        return false;
    }
}

// --------------------------------------------------------
// Desc:   reload of shaders without reloading of the engine :)
// Args:   - pDevice: pointer to the DirectX device
//         - vsFilename: path to the vertex shader hlsl-file
//         - psFilename: path to the pixel shader hlsl-file
// --------------------------------------------------------
void DebugShader::ShaderHotReload(
    ID3D11Device* pDevice,
    const char* vsFilename,
    const char* psFilename)
{
    bool result = false;
    const InputLayoutDebugShader layout;

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

///////////////////////////////////////////////////////////

void DebugShader::Render(
    ID3D11DeviceContext* pContext,
    ID3D11Buffer* pInstancedBuffer,
    const Instance* instances,
    const int numModels,
    const UINT instancedBuffElemSize)
{
    // bind input layout, shaders
    pContext->IASetInputLayout(vs_.GetInputLayout());
    pContext->VSSetShader(vs_.GetShader(), nullptr, 0);
    pContext->PSSetShader(ps_.GetShader(), nullptr, 0);

    // ---------------------------------------------
    
    // go through each instance and render it
    for (int i = 0, startInstanceLocation = 0; i < numModels; ++i)
    {
        const Instance& instance = instances[i];

        // prepare input assembler (IA) stage before the rendering process
        ID3D11Buffer* const vbs[2] = { instance.pVB, pInstancedBuffer };
        const UINT stride[2] = { instance.vertexStride, instancedBuffElemSize };
        const UINT offset[2] = { 0,0 };

        pContext->IASetVertexBuffers(0, 2, vbs, stride, offset);
        pContext->IASetIndexBuffer(instance.pIB, DXGI_FORMAT_R32_UINT, 0);

        // textures arr
        SRV* const* texIDs = instance.texSRVs.data();

        // go through each subset (mesh) of this model and render it
        for (int subsetIdx = 0; subsetIdx < (int)std::ssize(instance.subsets); ++subsetIdx)
        {
            // update textures for the current subset
            pContext->PSSetShaderResources(
                0U,
                NUM_TEXTURE_TYPES,
                texIDs + (subsetIdx * NUM_TEXTURE_TYPES));

            const Subset& subset = instance.subsets[subsetIdx];

            pContext->DrawIndexedInstanced(
                subset.indexCount,
                instance.numInstances,
                subset.indexStart,
                subset.vertexStart,
                startInstanceLocation + subsetIdx);
        }

        startInstanceLocation += (int)std::ssize(instance.subsets) * instance.numInstances;
    }
}

///////////////////////////////////////////////////////////

void DebugShader::SetDebugType(ID3D11DeviceContext* pContext, const eDebugState state)
{
    // setup the debug state 
    // (for instance: render normals as color, or show only diffuse map)
    cbpsRareChangedDebug_.data.debugType = state;
    cbpsRareChangedDebug_.ApplyChanges(pContext);
}

}  // namespace Render
