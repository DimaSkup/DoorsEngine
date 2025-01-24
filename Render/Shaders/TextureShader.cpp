// ====================================================================================
// Filename: TextureShader.cpp
// ====================================================================================
#include "TextureShader.h"
#include "shaderclass.h"

#include "../Common/Log.h"
#include "../Common/Types.h"
#include "../Common/MathHelper.h"
#include "../Common/Assert.h"


namespace Render
{

TextureShader::TextureShader() : className_{__func__}
{
}

TextureShader::~TextureShader() 
{
}


// ====================================================================================
// 
//                           PUBLIC MODIFICATION API
// 
// ====================================================================================

bool TextureShader::Initialize(
	ID3D11Device* pDevice, 
	ID3D11DeviceContext* pContext)
{
	// initialize the shader class: hlsl for rendering textured objects;
	// also create an instanced buffer;

	try
	{
		const std::string vsFilename = ShaderClass::pathToShadersDir_ + "textureVS.cso";
		const std::string psFilename = ShaderClass::pathToShadersDir_ + "texturePS.cso";

		InitializeShaders(pDevice, pContext, vsFilename, psFilename);
	}
	catch (LIB_Exception& e)
	{
		Log::Error(e, true);
		Log::Error("can't initialize the texture shader class");
		return false;
	}

	Log::Debug("is initialized");

	return true;
}


// ====================================================================================
// 
//                             PUBLIC RENDERING API
// 
// ====================================================================================


void TextureShader::Render(
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
	pContext->PSSetSamplers(0, 1, samplerState_.GetAddressOf());
		
	
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

// ====================================================================================
// 
//                         PRIVATE MODIFICATION API
// 
// ====================================================================================

void TextureShader::InitializeShaders(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext,
	const std::string& vsFilename,
	const std::string& psFilename)
{
	// initialized the vertex shader, pixel shader, input layout, 
	// sampler state, and different constant buffers

	bool result = false;

	const UINT layoutElemNum = 10;
	const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"TEXTRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"TEXTRANSFORM", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 80, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"TEXTRANSFORM", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 96, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"TEXTRANSFORM", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 112, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	};


	// check yourself
	Assert::True(layoutElemNum == ARRAYSIZE(inputLayoutDesc), "layout elems num != layout desc array size");

	// initialize: VS, PS, sampler state
	result = vs_.Initialize(pDevice, vsFilename, inputLayoutDesc, layoutElemNum);
	Assert::True(result, "can't initialize the vertex shader");

	result = ps_.Initialize(pDevice, psFilename);
	Assert::True(result, "can't initialize the pixel shader");

	result = samplerState_.Initialize(pDevice);
	Assert::True(result, "can't initialize the sampler state");
}

} // namespace Render