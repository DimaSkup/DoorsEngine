// =================================================================================
// Filename:     OutlineShader.cpp
// Description:  implementation of a wrapper for the outline shader
// 
// Created:      07.02.25  by DimaSkup
// =================================================================================
#include "../Common/pch.h"
#include "../Common/InputLayouts.h"
#include "OutlineShader.h"


namespace Render
{

OutlineShader::OutlineShader()
{
    strcpy(className_, __func__);
}

OutlineShader::~OutlineShader()
{
}

// =================================================================================
//                             public methods                                       
// =================================================================================
bool OutlineShader::Initialize(
    ID3D11Device* pDevice,
    const char* vsFilePath,
    const char* psFilePath)
{
    try
    {
        bool result = false;
        InputLayoutOutlineShader inputLayout;

        // initialize: VS, PS
        result = vs_.Initialize(pDevice, vsFilePath, inputLayout.desc, inputLayout.numElems);
        CAssert::True(result, "can't initialize the vertex shader");

        result = ps_.Initialize(pDevice, psFilePath);
        CAssert::True(result, "can't initialize the pixel shader");

        LogDbg(LOG, "is initialized");
        return true;
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        LogErr(LOG, "can't initialize the outline shader class");
        return false;
    }
}

///////////////////////////////////////////////////////////

void OutlineShader::Render(
    ID3D11DeviceContext* pContext,
    ID3D11Buffer* pInstancedBuffer,
    const InstanceBatch* instances,
    const int numInstances,
    const UINT instancesBuffElemSize)
{
}

} // namespace Render
