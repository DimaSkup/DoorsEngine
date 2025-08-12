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


// ====================================================================================
//                           PUBLIC MODIFICATION API
// ====================================================================================
bool TextureShader::Initialize(
    ID3D11Device* pDevice,
    const char* vsPath,          
    const char* psPath)          
{
    try
    {
        CAssert::True(!StrHelper::IsEmpty(vsPath), "input path to vertex shader is empty");
        CAssert::True(!StrHelper::IsEmpty(psPath), "input path to pixel shader is empty");

        InitializeShaders(pDevice, vsPath, psPath);
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


// ====================================================================================
//                             PUBLIC RENDERING API
// ====================================================================================
void TextureShader::Render(
    ID3D11DeviceContext* pContext,
    ID3D11Buffer* pInstancedBuffer,
    const InstanceBatch* instances,
    const int numModels,
    const UINT instancedBuffElemSize)
{
    assert(0 && "FIXME");
}


// ====================================================================================
//                         PRIVATE MODIFICATION API
// ====================================================================================
void TextureShader::InitializeShaders(
    ID3D11Device* pDevice,
    const char* vsFilePath,
    const char* psFilePath)
{
    // initialized the vertex shader, pixel shader, input layout, 
    // sampler state, and different constant buffers

    bool result = false;

    const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
    {
        // per vertex data
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},

        // per instance data
        {"WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1},

        {"WORLD_INV_TRANSPOSE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"WORLD_INV_TRANSPOSE", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 80, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"WORLD_INV_TRANSPOSE", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 96, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"WORLD_INV_TRANSPOSE", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 112, D3D11_INPUT_PER_INSTANCE_DATA, 1},

        {"TEX_TRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 128, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"TEX_TRANSFORM", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 144, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"TEX_TRANSFORM", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 160, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"TEX_TRANSFORM", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 176, D3D11_INPUT_PER_INSTANCE_DATA, 1},

        {"MATERIAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"MATERIAL", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"MATERIAL", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"MATERIAL", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
    };

    const UINT layoutElemNum = sizeof(inputLayoutDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC);

    // initialize: VS, PS, sampler state
    result = vs_.Initialize(pDevice, vsFilePath, inputLayoutDesc, layoutElemNum);
    CAssert::True(result, "can't initialize the vertex shader");

    result = ps_.Initialize(pDevice, psFilePath);
    CAssert::True(result, "can't initialize the pixel shader");
}

} // namespace Render
