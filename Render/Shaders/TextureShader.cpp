// ====================================================================================
// Filename: TextureShader.cpp
// ====================================================================================
#include "../Common/pch.h"
#include "TextureShader.h"


namespace Render
{

TextureShader::TextureShader()
{
    strcpy(className_, __func__);
}

TextureShader::~TextureShader() 
{
}

//---------------------------------------------------------
// Decs:   Load CSO / compile HLSL shaders and init this shader class instance
// Args:   - vsPath:  a path to compiled (.cso) vertex shader file
//         - psPath:  a path to compiled (.cso) pixel shader file
//---------------------------------------------------------
bool TextureShader::Initialize(
    ID3D11Device* pDevice,
    const char* vsPath,          
    const char* psPath)          
{
    try
    {
        CAssert::True(!StrHelper::IsEmpty(vsPath), "input path to vertex shader is empty");
        CAssert::True(!StrHelper::IsEmpty(psPath), "input path to pixel shader is empty");

        bool result = false;
        const InputLayoutTextureShader layout;

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
        LogErr(LOG, "can't initialize the texture shader class");
        return false;
    }
}

//---------------------------------------------------------
// Desc:   just render stuff onto the screen
//---------------------------------------------------------
void TextureShader::Render(
    ID3D11DeviceContext* pContext,
    ID3D11Buffer* pInstancedBuffer,
    const InstanceBatch* instances,
    const int numModels,
    const UINT instancedBuffElemSize)
{
    assert(0 && "FIXME");
}

} // namespace Render
