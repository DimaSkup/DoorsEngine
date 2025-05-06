// ***********************************************************************************
// Filename: FontShader.cpp
// Revising: 23.07.22
// ***********************************************************************************
#include "FontShader.h"
#include "../Common/Log.h"

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
    catch (LIB_Exception & e)
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
    ID3D11Buffer* const* indexBuffers,            // array of text indices buffers
    const uint32_t* indexCounts,                  // array of index counts in each index buffer
    const size numSentences,
    const uint32_t fontVertexSize,
    SRV* const* ppFontTexSRV)
{
    // renders text onto the screen
    try
    {
        Assert::True(vertexBuffers,    "input ptr to vertex buffers arr == nullptr");
        Assert::True(indexBuffers,     "input ptr to index buffers arr == nullptr");
        Assert::True(indexCounts,      "input ptr to index counts arr == nullptr");
        Assert::True(numSentences > 0, "input number of sentences must be > 0");

        // bind vertex and pixel shaders
        pContext->VSSetShader(vs_.GetShader(), nullptr, 0U);
        pContext->PSSetShader(ps_.GetShader(), nullptr, 0U);

        // set the primitive topology for all the sentences and the input layout
        pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        pContext->IASetInputLayout(vs_.GetInputLayout());

        // set the sampler state and textures
        pContext->PSSetSamplers(0, 1, samplerState_.GetAddressOf());
        pContext->PSSetShaderResources(0, 1, ppFontTexSRV);

        const UINT stride = fontVertexSize;
        const UINT offset = 0;

        for (index idx = 0; idx < numSentences; ++idx)
        {
            // bind vb/ib
            pContext->IASetVertexBuffers(0, 1, &vertexBuffers[idx], &stride, &offset);
            pContext->IASetIndexBuffer(indexBuffers[idx], DXGI_FORMAT_R32_UINT, 0);

            // render the fonts on the screen
            pContext->DrawIndexed(indexCounts[idx], 0, 0);
        }
    }
    catch (LIB_Exception& e)
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
    Assert::True(result, "can't initialize the vertex shader");

    // initialize the pixel shader
    result = ps_.Initialize(pDevice, psFilePath);
    Assert::True(result, "can't initialize the pixel shader");

    // initialize the sampler state
    result = samplerState_.Initialize(pDevice);
    Assert::True(result, "can't initialize the sampler state");


    // ---------------------------  CONSTANT BUFFERS  ---------------------------------

    // initialize the matrix buffer
    hr = matrixBuffer_.Initialize(pDevice);
    Assert::NotFailed(hr, "can't initialize the matrix buffer");
    
    // initialize the pixel buffer
    hr = pixelBuffer_.Initialize(pDevice);
    Assert::NotFailed(hr, "can't initialize the pixel buffer");
    

    // ---------------- SET DEFAULT PARAMS FOR CONST BUFFERS --------------------------

    ID3D11DeviceContext* pContext = nullptr;
    pDevice->GetImmediateContext(&pContext);
    SetFontColor(pContext, { 1, 1, 1 });  // set white colour by default
    SetWorldViewOrtho(pContext, WVO);
}

}
