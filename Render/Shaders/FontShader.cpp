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

// =================================================================================
//                             PUBLIC MODIFICATION API
// =================================================================================

bool FontShader::Initialize(
    ID3D11Device* pDevice, 
    const DirectX::XMMATRIX& WVO,            // world * base_view * ortho
    const char* vsFilePath,
    const char* psFilePath)
{
    // Initialize() initializes the vertex and pixel shaders, input layout,
    // sampler state, matrix and pixel buffers
    try
    {
        InitializeShaders(pDevice, WVO, vsFilePath, psFilePath);
        LogDbg("is initialized");
        return true;
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        LogErr("can't initialize the font shader class");
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

        // set the sampler state and textures
        pContext->PSSetSamplers(0, 1, samplerState_.GetAddressOf());
        pContext->PSSetShaderResources(0, 1, ppFontTexSRV);

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


// =================================================================================
//                              PRIVATE METHODS
// =================================================================================
void FontShader::InitializeShaders(
    ID3D11Device* pDevice,
    const DirectX::XMMATRIX& WVO,        // world * base_view * ortho
    const char* vsFilePath,
    const char* psFilePath)
{
    // InitializeShaders() helps to initialize the vertex and pixel shaders,
    // input layout, sampler state, matrix and pixel buffers

    bool result = false;
    HRESULT hr = S_OK;

    // --------------------------  INPUT LAYOUT DESC  ---------------------------------

    const D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    const UINT layoutElemNum = sizeof(layoutDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC);


    // -----------------------  SHADERS / SAMPLER STATE  ------------------------------

    // initialize the vertex shader
    result = vs_.Initialize(pDevice, vsFilePath, layoutDesc, layoutElemNum);
    CAssert::True(result, "can't initialize the vertex shader");

    // initialize the pixel shader
    result = ps_.Initialize(pDevice, psFilePath);
    CAssert::True(result, "can't initialize the pixel shader");

    // initialize the sampler state
    result = samplerState_.Initialize(pDevice);
    CAssert::True(result, "can't initialize the sampler state");


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
}

}
