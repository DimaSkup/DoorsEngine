// ====================================================================================
// Filename: BillboardShader.cpp
// ====================================================================================
#include "../Common/pch.h"
#include "BillboardShader.h"

#pragma warning (disable : 4996)


namespace Render
{

BillboardShader::BillboardShader()
{
    strcpy(className_, __func__);
}

BillboardShader::~BillboardShader()
{
}

///////////////////////////////////////////////////////////

bool BillboardShader::Initialize(
    ID3D11Device* pDevice,
    const char* vsFilePath,
    const char* psFilePath,
    const char* gsFilePath)
{
    try
    {
        CAssert::True(!StrHelper::IsEmpty(vsFilePath), "input vertex shader filepath is empty");
        CAssert::True(!StrHelper::IsEmpty(psFilePath), "input pixel shader filepath is empty");
        CAssert::True(!StrHelper::IsEmpty(gsFilePath), "input geometry shader filepath is empty");

        InitializeShaders(pDevice, vsFilePath, psFilePath, gsFilePath);
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
// Desc:   render an array of points as billboards
// Args:   - pContext:         DirectX11 device context
//         - pVB:              a pointer to the vertex buffer
//         - ppTextureArrSRV:  2d array of textures shader resources
//         - stride:           size in bytes of a single vertex
//         - numVertices:      how many vertices will we render as billboards
//---------------------------------------------------------
void BillboardShader::Render(
    ID3D11DeviceContext* pContext,
    ID3D11Buffer* pVB,
    SRV* const* ppTextureArrSRV,
    const UINT stride,             
    const UINT numVertices)
{
    pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    // bind input layout, shaders, samplers
    pContext->IASetInputLayout(vs_.GetInputLayout());
    pContext->VSSetShader(vs_.GetShader(), nullptr, 0);
    pContext->GSSetShader(gs_.GetShader(), nullptr, 0);
    pContext->PSSetShader(ps_.GetShader(), nullptr, 0);
    pContext->PSSetSamplers(0, 1, samplerState_.GetAddressOf());

    UINT offset = 0;

    // bind vertex/index buffer and textures 2D array as well
    pContext->IASetVertexBuffers(0, 1, &pVB, &stride, &offset);
    //pContext->PSSetShaderResources(1, 1, ppTextureArrSRV);
    //pContext->PSSetShaderResources(2, 1, ppTextureArrSRV+1);

    // draw a billboard
    pContext->Draw(numVertices, 0);

    // unbind the geometry shader
    pContext->GSSetShader(nullptr, nullptr, 0);
}

///////////////////////////////////////////////////////////

void BillboardShader::InitializeShaders(
    ID3D11Device* pDevice,
    const char* vsFilePath,
    const char* psFilePath,
    const char* gsFilePath)
{
    // initialized the vertex shader, pixel shader, input layout, 
    // sampler state, and different constant buffers

    bool result = false;

    const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
    {
        // per vertex data
        {"POSITION",      0, DXGI_FORMAT_R32G32B32_FLOAT, 0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TRANSLUCENCY",  0, DXGI_FORMAT_R32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR",         0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"SIZE",          0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    const UINT layoutElemNum = sizeof(inputLayoutDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC);

    // initialize: VS, PS, sampler state
    result = vs_.Initialize(pDevice, vsFilePath, inputLayoutDesc, layoutElemNum);
    CAssert::True(result, "can't initialize the vertex shader");

    result = gs_.Initialize(pDevice, gsFilePath);
    CAssert::True(result, "can't initialize the geometry shader");

    result = ps_.Initialize(pDevice, psFilePath);
    CAssert::True(result, "can't initialize the pixel shader");

    result = samplerState_.Initialize(pDevice);
    CAssert::True(result, "can't initialize the sampler state");
}

} // namespace Render
