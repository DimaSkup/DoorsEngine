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

//---------------------------------------------------------
// Decs:  Load CSO / compile HLSL shaders and init this shader class instance
//---------------------------------------------------------
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

//---------------------------------------------------------
//---------------------------------------------------------
void DebugShader::Render(
    ID3D11DeviceContext* pContext,
    ID3D11Buffer* pInstancedBuffer,
    const InstanceBatch* instances,
    const int numModels,
    const UINT instancedBuffElemSize)
{
    assert(0 && "FIXME");
}

//---------------------------------------------------------
// Desc:  setup the debug state 
//        (for example: render normals as color, or show only diffuse map)
//---------------------------------------------------------
void DebugShader::SetDebugType(ID3D11DeviceContext* pContext, const eDebugState state)
{
 
    cbpsRareChangedDebug_.data.debugType = state;
    cbpsRareChangedDebug_.ApplyChanges(pContext);
}

} // namespace
