/////////////////////////////////////////////////////////////////////
// Filename: ColorShader.cpp
// Revising: 06.04.22
/////////////////////////////////////////////////////////////////////
#include "../Common/pch.h"
#include "../Common/InputLayouts.h"
#include "ColorShader.h"


namespace Render
{

ColorShader::ColorShader()
{
    strcpy(className_, __func__);
}

ColorShader::~ColorShader()
{
}

//---------------------------------------------------------
// Desc:   load hlsl shaders, create DX shader objects,
//         and init this shader class instance
//---------------------------------------------------------
bool ColorShader::Initialize(
    ID3D11Device* pDevice,
    const char* vsFilePath,
    const char* psFilePath)
{
    try
    {
        InitializeShaders(pDevice, vsFilePath, psFilePath);
        LogDbg(LOG, "is initialized");
        return true;
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        LogErr(LOG, "can't initialize the color shader class");
        return false;
    }
}

///////////////////////////////////////////////////////////

void ColorShader::Render(
    ID3D11DeviceContext* pContext,
    ID3D11Buffer* pInstancedBuffer,
    const Instance* instances,
    const int numModels,
    const UINT instancedBuffElemSize)
{
    int startInstanceLocation = 0;

    // bind shaders, input layout
    pContext->IASetInputLayout(vs_.GetInputLayout());
    pContext->VSSetShader(vs_.GetShader(), nullptr, 0);
    pContext->PSSetShader(ps_.GetShader(), nullptr, 0);


    // go through each instance and render it
    for (int i = 0; i < numModels; ++i)
    {
        const Instance& instance = instances[i];

        const UINT stride[2] = { instance.vertexStride, instancedBuffElemSize };
        const UINT offset[2] = { 0,0 };

        // prepare input assembler (IA) stage before the rendering process
        ID3D11Buffer* const vbs[2] = { instance.pVB, pInstancedBuffer };

        pContext->IASetVertexBuffers(0, 2, vbs, stride, offset);
        pContext->IASetIndexBuffer(instance.pIB, DXGI_FORMAT_R32_UINT, 0);

        SRV* const* texIDs = instance.texSRVs.data();

        // go through each subset (mesh) of this model and render it
        for (int subsetIdx = 0; subsetIdx < (int)std::ssize(instance.subsets); ++subsetIdx)
        {
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

// ------------------------------------------------------------------------------
// Desc:   a private helper for initialization
// ------------------------------------------------------------------------------
void ColorShader::InitializeShaders( 
    ID3D11Device* pDevice,
    const char* vsFilePath,
    const char* psFilePath)
{
    bool result = false;
    InputLayoutColor inputLayout;
    
    // initialize: VS, PS
    result = vs_.Initialize(pDevice, vsFilePath, inputLayout.desc, inputLayout.numElems);
    CAssert::True(result, "can't initialize the vertex shader");

    result = ps_.Initialize(pDevice, psFilePath);
    CAssert::True(result, "can't initialize the pixel shader");
}

} // namespace Render
