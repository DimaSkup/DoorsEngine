// =================================================================================
// Filename:     LightShaderClass.cpp
// Description:  this class is needed for rendering 3D models, 
//               its texture, simple PARRALEL light on it using HLSL shaders.
// Created:      09.04.23
// =================================================================================
#include "LightShaderClass.h"
#include "shaderclass.h"

#include "../Common/Assert.h"
#include "../Common/Log.h"

namespace Render
{

LightShaderClass::LightShaderClass() 
	: className_{ __func__ }
{
}

LightShaderClass::~LightShaderClass()
{
}


// =================================================================================
//                             public methods                                       
// =================================================================================

bool LightShaderClass::Initialize(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	// initialize the shader class: for rendering textured + lit objects;
	// also create an instanced buffer;

	try
	{
		const std::string vsFilePath = ShaderClass::pathToShadersDir_ + "LightVS.cso";
		const std::string psFilePath = ShaderClass::pathToShadersDir_ + "LightPS.cso";

		InitializeShaders(pDevice, pContext, vsFilePath, psFilePath);

		Log::Debug("is initialized");
	}
	catch (LIB_Exception& e)
	{
		Log::Error(e, true);
		Log::Error("can't initialize the light shader class");
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////

void LightShaderClass::Render(
	ID3D11DeviceContext* pContext,
	ID3D11Buffer* pInstancedBuffer,
	const Instance* instances,
	const int numModels,
	const UINT instancedBuffElemSize)
{
	// bind input layout, shaders, samplers
	pContext->IASetInputLayout(vs_.GetInputLayout());
	pContext->VSSetShader(vs_.GetShader(), nullptr, 0);
	pContext->PSSetShader(ps_.GetShader(), nullptr, 0);
	pContext->PSSetSamplers(0, 1, samplerState_.GetAddressOf());


	// go through each instance and render it
	for (int i = 0, startInstanceLocation = 0; i < numModels; ++i)
	{
		const Instance& instance = instances[i];

		// prepare input assembler (IA) stage before the rendering process
		ID3D11Buffer* const vbs[2] = { instance.pVB, pInstancedBuffer };
		const UINT stride[2] = { instance.vertexStride, instancedBuffElemSize };
		const UINT offset[2] = { 0,0 };

		pContext->IASetVertexBuffers(0, 2, vbs, stride, offset);
		pContext->IASetIndexBuffer(instance.pIB, DXGI_FORMAT_R32_UINT, 0);

		// textures arr
		SRV* const* texIDs = instance.texSRVs.data();

		// go through each subset (mesh) of this model and render it
		for (int subsetIdx = 0; subsetIdx < (int)std::ssize(instance.subsets); ++subsetIdx)
		{
			// update textures for the current subset
			pContext->PSSetShaderResources(0U, 22U, texIDs + (subsetIdx * 22));
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

void LightShaderClass::InitializeShaders(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext,
	const std::string& vsFilePath,
	const std::string& psFilePath)
{
	//
	// helps to initialize the HLSL shaders, layout, sampler state
	//
	
	bool result = false;

	const UINT layoutElemNum = 21;
	const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
	{
		// per vertex data
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},

		// per instance data
		{"WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
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


	// check yourself
	Assert::True(layoutElemNum == ARRAYSIZE(inputLayoutDesc), "layout elems num != layout desc array size");

	// initialize: VS, PS, sampler state
	result = vs_.Initialize(pDevice, vsFilePath, inputLayoutDesc, layoutElemNum);
	Assert::True(result, "can't initialize the vertex shader");

	result = ps_.Initialize(pDevice, psFilePath);
	Assert::True(result, "can't initialize the pixel shader");

	result = samplerState_.Initialize(pDevice);
	Assert::True(result, "can't initialize the sampler state");
}

} // namespace Render