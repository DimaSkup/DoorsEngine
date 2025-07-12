// =================================================================================
// Filename:     OutlineShader.cpp
// Description:  implementation of a wrapper for the outline shader
// 
// Created:      07.02.25  by DimaSkup
// =================================================================================
#include "../Common/pch.h"
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
        InitializeShaders(pDevice, vsFilePath, psFilePath);
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
	const Instance* instances,
	const int numInstances,
	const UINT instancesBuffElemSize)
{
	// bind input layout and shaders
	pContext->IASetInputLayout(vs_.GetInputLayout());
	pContext->VSSetShader(vs_.GetShader(), nullptr, 0);
	pContext->PSSetShader(ps_.GetShader(), nullptr, 0);

	// go through each instance and render it
	for (int i = 0, startInstanceLocation = 0; i < numInstances; ++i)
	{
		const Instance& instance = instances[i];

		// bind buffers
		ID3D11Buffer* const vbs[2] = { instance.pVB, pInstancedBuffer };
		const UINT stride[2] = { instance.vertexStride, instancesBuffElemSize };
		const UINT offset[2] = { 0,0 };

		pContext->IASetVertexBuffers(0, 2, vbs, stride, offset);
		pContext->IASetIndexBuffer(instance.pIB, DXGI_FORMAT_R32_UINT, 0);

		// go through each subset (mesh) of this model and render it
		for (int subsetIdx = 0; subsetIdx < (int)std::ssize(instance.subsets); ++subsetIdx)
		{
			const Subset& subset = instance.subsets[subsetIdx];

			pContext->DrawIndexedInstanced(
				subset.indexCount,
				instance.numInstances,
				subset.indexStart,
				subset.vertexStart,
				startInstanceLocation + subsetIdx);
		}

		startInstanceLocation += (int)std::ssize(instance.subsets) * instance.numInstances;
	}
}

// =================================================================================
//                              private methods                                       
// =================================================================================
void OutlineShader::InitializeShaders(
	ID3D11Device* pDevice,
    const char* vsFilePath,
    const char* psFilePath)
{
	//
	// helps to initialize the HLSL shaders, layout, etc.
	//

	bool result = false;

	const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
	{
		// per vertex data
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},

		// per instance data
		{"WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1},

		{"WORLD_INV_TRANSPOSE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"WORLD_INV_TRANSPOSE", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 80, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"WORLD_INV_TRANSPOSE", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 96, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"WORLD_INV_TRANSPOSE", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 112, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	};

    const UINT layoutElemNum = sizeof(inputLayoutDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC);

	// initialize: VS, PS
	result = vs_.Initialize(pDevice, vsFilePath, inputLayoutDesc, layoutElemNum);
	CAssert::True(result, "can't initialize the vertex shader");

	result = ps_.Initialize(pDevice, psFilePath);
	CAssert::True(result, "can't initialize the pixel shader");
}

} // namespace Render
