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
        bool result = false;
        InputLayoutColor inputLayout;

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
