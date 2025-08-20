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

//---------------------------------------------------------
// Decs:   Load CSO / compile HLSL shaders and init this shader class instance
// Args:   - vsPath:  a path to compiled (.cso) vertex shader file
//         - psPath:  a path to compiled (.cso) pixel shader file
//---------------------------------------------------------
bool OutlineShader::Initialize(
    ID3D11Device* pDevice,
    const char* vsPath,
    const char* psPath)
{
    try
    {
        CAssert::True(!StrHelper::IsEmpty(vsPath), "input path to vertex shader is empty");
        CAssert::True(!StrHelper::IsEmpty(psPath), "input path to pixel shader is empty");

        bool result = false;
        const InputLayoutOutlineShader layout;

        // initialize vertex, pixel shaders
        result = vs_.LoadPrecompiled(pDevice, vsPath, layout.desc, layout.numElems);
        CAssert::True(result, "can't initialize the vertex shader");

        result = ps_.LoadPrecompiled(pDevice, psPath);
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

//---------------------------------------------------------
//---------------------------------------------------------
void OutlineShader::Render(
    ID3D11DeviceContext* pContext,
    ID3D11Buffer* pInstancedBuffer,
    const InstanceBatch* instances,
    const int numInstances,
    const UINT instancesBuffElemSize)
{
    assert(0 && "FIXME");
}

} // namespace Render
