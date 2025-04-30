// =================================================================================
// Filename:     LightShader.cpp
// Description:  this class is needed for rendering 3D models, 
//               its texture, simple PARRALEL light on it using HLSL shaders.
// Created:      09.04.23
// =================================================================================
#include "LightShader.h"
#include "../Common/Log.h"
#include "../Common/Types.h"


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
    catch (LIB_Exception& e)
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
                0U,
                NUM_TEXTURE_TYPES,
                texSRVs + (subsetIdx * NUM_TEXTURE_TYPES));

            //const UINT numTexturesToBind = 22U * numUniqueTexturesArraysPerSubset;
            //pContext->PSSetShaderResources(0U, numTexturesToBind, texSRVs + (subsetIdx * 44U));

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
    //
    // helps to initialize the HLSL shaders, layout, sampler state
    //

    const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
    {
        // per vertex data
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},

        // per instance data
        {"WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1},

        {"WORLD_INV_TRANSPOSE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"WORLD_INV_TRANSPOSE", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 80, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"WORLD_INV_TRANSPOSE", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 96, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"WORLD_INV_TRANSPOSE", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 112, D3D11_INPUT_PER_INSTANCE_DATA, 1},

        {"TEX_TRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 128, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"TEX_TRANSFORM", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 144, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"TEX_TRANSFORM", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 160, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"TEX_TRANSFORM", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 176, D3D11_INPUT_PER_INSTANCE_DATA, 1},

        {"MATERIAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"MATERIAL", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"MATERIAL", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"MATERIAL", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
    };

    const UINT layoutElemNum = sizeof(inputLayoutDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC);

    // initialize: VS, PS, sampler state
    bool result = false;
    result = vs_.Initialize(pDevice, vsFilePath, inputLayoutDesc, layoutElemNum);
    Assert::True(result, "can't initialize the vertex shader");

    result = ps_.Initialize(pDevice, psFilePath);
    Assert::True(result, "can't initialize the pixel shader");

    result = samplerState_.Initialize(pDevice);
    Assert::True(result, "can't initialize the sampler state");
}

} // namespace Render
