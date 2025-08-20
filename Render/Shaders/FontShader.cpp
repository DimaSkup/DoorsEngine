// ***********************************************************************************
// Filename: FontShader.cpp
// Revising: 23.07.22
// ***********************************************************************************
#include "../Common/pch.h"
#include "FontShader.h"

#pragma warning (disable : 4996)


namespace Render
{

FontShader::FontShader()
{
    strcpy(className_, __func__);
}

FontShader::~FontShader() 
{
}

//---------------------------------------------------------
// Decs:   Load CSO / compile HLSL shaders and init this shader class instance
// Args:   - WVO:     world * base_view * ortho  matrix
//         - vsPath:  a path to compiled (.cso) vertex shader file
//         - psPath:  a path to compiled (.cso) pixel shader file
//---------------------------------------------------------
bool FontShader::Initialize(
    ID3D11Device* pDevice, 
    const DirectX::XMMATRIX& WVO,            
    const char* vsPath,
    const char* psPath)
{
    try
    {
        CAssert::True(!StrHelper::IsEmpty(vsPath), "path to vertex shader is empty");
        CAssert::True(!StrHelper::IsEmpty(psPath), "path to pixel shader is empty");

        const InputLayoutFontShader layout;
        bool result = false;
        HRESULT hr = S_OK;

        // init the vertex shader
        result = vs_.LoadPrecompiled(pDevice, vsPath, layout.desc, layout.numElems);
        CAssert::True(result, "can't initialize the vertex shader");

        // init the pixel shader
        result = ps_.LoadPrecompiled(pDevice, psPath);
        CAssert::True(result, "can't initialize the pixel shader");


        // ---------------------------  CONSTANT BUFFERS  ---------------------------------

        // initialize the matrix buffer
        hr = matrixBuffer_.Initialize(pDevice);
        CAssert::NotFailed(hr, "can't initialize the matrix buffer");

        // initialize the pixel buffer
        hr = pixelBuffer_.Initialize(pDevice);
        CAssert::NotFailed(hr, "can't initialize the pixel buffer");


        // ---------------- SET DEFAULT PARAMS FOR CONST BUFFERS --------------------------

        ID3D11DeviceContext* pContext = nullptr;
        pDevice->GetImmediateContext(&pContext);
        SetFontColor(pContext, { 1, 1, 1 });  // set white colour by default
        SetWorldViewOrtho(pContext, WVO);


        LogDbg(LOG, "is initialized");
        return true;
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        LogErr(LOG, "can't initialize the font shader class");
        return false;
    }
}

///////////////////////////////////////////////////////////

void FontShader::SetWorldViewOrtho(
    ID3D11DeviceContext* pContext,
    const DirectX::XMMATRIX& WVO)
{
    // setup matrix for 2D rendering
    // NOTE: the WVO matrix must be already transposed
    matrixBuffer_.data.worldViewProj = WVO;
    matrixBuffer_.ApplyChanges(pContext);
}

///////////////////////////////////////////////////////////

void FontShader::SetFontColor(
    ID3D11DeviceContext* pContext,
    const DirectX::XMFLOAT3& color)
{
    // load the pixel color data into GPU
    pixelBuffer_.data.pixelColor = color;
    pixelBuffer_.ApplyChanges(pContext);
}


// =================================================================================
//                             PUBLIC RENDERING API
// =================================================================================
void FontShader::Render(
    ID3D11DeviceContext* pContext,
    ID3D11Buffer* const* vertexBuffers,           // array of text vertex buffers
    ID3D11Buffer* pIndexBuffer,                   // index buffer is common for all vertex buffers (we have the same vertices order for each sentence)
    const uint32* indexCounts,                  // array of index counts in each index buffer
    const size numSentences,
    const uint32 fontVertexSize,
    SRV* const* ppFontTexSRV)
{
    // renders text onto the screen
    try
    {
        CAssert::True(vertexBuffers,    "input ptr to vertex buffers arr == nullptr");
        CAssert::True(pIndexBuffer,     "input ptr to index buffer == nullptr");
        CAssert::True(indexCounts,      "input ptr to index counts arr == nullptr");
        CAssert::True(numSentences > 0, "input number of sentences must be > 0");

        // bind vertex/pixel shaders and input layout
        pContext->VSSetShader(vs_.GetShader(), nullptr, 0U);
        pContext->PSSetShader(ps_.GetShader(), nullptr, 0U);
        pContext->IASetInputLayout(vs_.GetInputLayout());

        // set textures
        pContext->PSSetShaderResources(2, 1, ppFontTexSRV);

        const UINT stride = fontVertexSize;
        const UINT offset = 0;

        pContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

        for (index idx = 0; idx < numSentences; ++idx)
        {
            pContext->IASetVertexBuffers(0, 1, &vertexBuffers[idx], &stride, &offset);
            
            // render the fonts on the screen
            pContext->DrawIndexed(indexCounts[idx], 0, 0);
        }
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        LogErr("can't render using the shader");
        return;
    }
}

}
