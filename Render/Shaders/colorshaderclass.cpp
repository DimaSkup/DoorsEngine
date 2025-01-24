/////////////////////////////////////////////////////////////////////
// Filename: colorshaderclass.cpp
// Revising: 06.04.22
/////////////////////////////////////////////////////////////////////
#include "colorshaderclass.h"
#include "shaderclass.h"

#include "../Common/Log.h"
#include "../Common/MathHelper.h"
#include "../Common/MemHelpers.h"


namespace Render
{


ColorShaderClass::ColorShaderClass() 
	: className_ { __func__ }
{
}

ColorShaderClass::~ColorShaderClass()
{
}


// ------------------------------------------------------------------------------ //
//
//                         PUBLIC FUNCTIONS
//
// ------------------------------------------------------------------------------ //

bool ColorShaderClass::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// THIS FUNCTION initializes the ColorShaderClass; 
	// creates a vertex and pixel shader, input layout, and const buffer 

	try
	{
		const std::string vsFilename = ShaderClass::pathToShadersDir_ + "colorVS.cso";
		const std::string psFilename = ShaderClass::pathToShadersDir_ + "colorPS.cso";

		InitializeShaders(pDevice, pContext, vsFilename, psFilename);
	}
	catch (LIB_Exception& e)
	{
		Log::Error(e, true);
		Log::Error("can't initialize the color shader class");
		return false;
	}

	Log::Debug("is initialized");

	return true;
}

///////////////////////////////////////////////////////////

void ColorShaderClass::Render(
	ID3D11DeviceContext* pContext,
	ID3D11Buffer* pInstancedBuffer,
	const Instance* instances,
	const int numModels,
	const UINT instancedBuffElemSize)
{
	int startInstanceLocation = 0;

	pContext->IASetInputLayout(vs_.GetInputLayout());
	pContext->VSSetShader(vs_.GetShader(), nullptr, 0);
	pContext->PSSetShader(ps_.GetShader(), nullptr, 0);


	// go through each instance and render it
	for (int i = 0; i < numModels; ++i)
	{
		const Instance& instance = instances[i];

		const UINT stride[2] = { instance.vertexStride, instancedBuffElemSize };
		const UINT offset[2] = { 0,0 };

		// prepare input assembler (IA) stage before the rendering process
		ID3D11Buffer* const vbs[2] = { instance.pVB, pInstancedBuffer };

		pContext->IASetVertexBuffers(0, 2, vbs, stride, offset);
		pContext->IASetIndexBuffer(instance.pIB, DXGI_FORMAT_R32_UINT, 0);

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



// ------------------------------------------------------------------------------ //
//
//                         PRIVATE FUNCTIONS
//
// ------------------------------------------------------------------------------ //

void ColorShaderClass::InitializeShaders( 
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext,
	const std::string& vsFilename,
	const std::string& psFilename)
{
	// Initializes the shaders, input vertex layout and constant matrix buffer.
	// This function is called from the Initialize() function

	HRESULT hr = S_OK;
	bool result = false;

	// sum of the structures sizes of:
	// position (float3) + 
	// texture (float2) + 
	// normal (float3) +
	// tangent (float3) + 
	// binormal (float3);
	// (look at the the VERTEX structure)
	
	const UINT colorOffset = (4 * sizeof(DirectX::XMFLOAT3)) + sizeof(DirectX::XMFLOAT2);

	const UINT layoutElemNum = 18;
	const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
	{
		// per vertex data
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, colorOffset, D3D11_INPUT_PER_VERTEX_DATA, 0},
		//{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		//{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		//{"TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		//{"BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},

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
	result = vs_.Initialize(pDevice, vsFilename, inputLayoutDesc, layoutElemNum);
	Assert::True(result, "can't initialize the vertex shader");

	result = ps_.Initialize(pDevice, psFilename);
	Assert::True(result, "can't initialize the pixel shader");
}

} // namespace Render