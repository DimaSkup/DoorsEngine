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
// Decs:   Load CSO / compile HLSL shaders and init this shader class instance
// Args:   - vsPath:  a path to compiled (.cso) vertex shader file
//         - psPath:  a path to compiled (.cso) pixel shader file
//---------------------------------------------------------
bool ColorShader::Initialize(
    ID3D11Device* pDevice,
    const char* vsPath,
    const char* psPath)
{
    try
    {
        CAssert::True(!StrHelper::IsEmpty(vsPath), "path to vertex shader is empty");
        CAssert::True(!StrHelper::IsEmpty(psPath), "path to pixel shader is empty");

        bool result = false;
        const InputLayoutColor layout;

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
        LogErr(LOG, "can't initialize the color shader class");
        return false;
    }
}

// --------------------------------------------------------
// --------------------------------------------------------
void ColorShader::Render(
    ID3D11DeviceContext* pContext,
    ID3D11Buffer* pInstancedBuffer,
    const InstanceBatch* instances,
    const int numModels,
    const UINT instancedBuffElemSize)
{
    assert(0 && "FIXME");
}

} // namespace Render
