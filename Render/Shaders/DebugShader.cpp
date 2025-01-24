// ********************************************************************************
// Filename:   DebugShader.cpp
// Created:    24.11.24
// ********************************************************************************
#include "DebugShader.h"
#include "shaderclass.h"

namespace Render
{

DebugShader::DebugShader() : className_{ __func__ }
{
}

DebugShader::~DebugShader()
{
}

// *********************************************************************************
//
//                             PUBLIC METHODS                                       
//
// *********************************************************************************

bool DebugShader::Initialize(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext)
{
	try
	{
		const std::string vsFilepath = ShaderClass::pathToShadersDir_ + "DebugVS.cso";
		const std::string psFilepath = ShaderClass::pathToShadersDir_ + "DebugPS.cso";

		InitializeShaders(pDevice, pContext, vsFilepath, psFilepath);
	}
	catch (LIB_Exception& e)
	{
		Log::Error(e, true);
		Log::Error("can't initialize the debug vector shader");
		return false;
	}

	Log::Debug("is initialized");

	return true;
}

///////////////////////////////////////////////////////////

void DebugShader::Render(
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

	// ---------------------------------------------
	
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

///////////////////////////////////////////////////////////

void DebugShader::SetDebugType(ID3D11DeviceContext* pContext, const DebugState state)
{
	// setup the debug state 
	// (for instance: render normals as color, or show only diffuse map)
	cbpsRareChangedDebug_.data.debugType = state;
	cbpsRareChangedDebug_.ApplyChanges(pContext);
}



// =================================================================================
//                               private methods                                       
// =================================================================================

void DebugShader::InitializeShaders(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext,
	const std::string& vsFilename,
	const std::string& psFilename)
{
	//
	// helps to initialize the HLSL shaders, layout, sampler state, and buffers
	//

	HRESULT hr = S_OK;
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


	// --------------------- SHADERS / SAMPLER STATE -------------------------- //

	result = vs_.Initialize(pDevice, vsFilename, inputLayoutDesc, layoutElemNum);
	Assert::True(result, "can't initialize the vertex shader");

	// initialize the DEFAULT pixel shader
	result = ps_.Initialize(pDevice, psFilename);
	Assert::True(result, "can't initialize the pixel shader");

	result = samplerState_.Initialize(pDevice);
	Assert::True(result, "can't initialize the sampler state");

	// ------------------------ CONSTANT BUFFERS ------------------------------ 

	// rare changed const buffer (for debug)
	hr = cbpsRareChangedDebug_.Initialize(pDevice, pContext);
	Assert::NotFailed(hr, "can't init a const buffer for debug (PS)");


	// -------------  SETUP CONST BUFFERS WITH DEFAULT PARAMS  ----------------

	// load rare changed data into GPU
	cbpsRareChangedDebug_.ApplyChanges(pContext);
}

}  // namespace Render