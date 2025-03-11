// ====================================================================================
// Filename: BillboardShader.cpp
// ====================================================================================
#include "BillboardShader.h"
#include "shaderclass.h"

#include "../Common/Log.h"
#include "../Common/Types.h"
#include "../Common/Assert.h"


namespace Render
{

BillboardShader::BillboardShader() : className_{ __func__ }
{
}

BillboardShader::~BillboardShader()
{
	Shutdown();
}


// ====================================================================================
//                           PUBLIC MODIFICATION API
// ====================================================================================

bool BillboardShader::Initialize(ID3D11Device* pDevice,	const std::string& pathToShadersDir)
{
	// initialize the shader class: hlsl for rendering textured objects;
	// also create an instanced buffer;

	try
	{
		InitializeShaders(
			pDevice,
			pathToShadersDir + "billboardVS.cso",
			pathToShadersDir + "billboardGS.cso",
			pathToShadersDir + "billboardPS.cso");
	}
	catch (LIB_Exception& e)
	{
		Shutdown();
		Log::Error(e, true);
		Log::Error("can't initialize the texture shader class");
		return false;
	}

	Log::Debug("is initialized");

	return true;
}

///////////////////////////////////////////////////////////

void BillboardShader::UpdateInstancedBuffer(
	ID3D11DeviceContext* pContext,
	const Material* materials,
	const DirectX::XMFLOAT3* positions,   // positions in world space
	const DirectX::XMFLOAT2* sizes,       // sizes of billboards
	const int count)                      // the number of billboards to render
{
	//
	// fill in the instanced buffer with data (call it before Render() method for multiple instances)
	//

	try
	{
		Assert::True(materials && positions && sizes && (count > 0) && (count < numMaxInstances_), "input data is invalid");

		// map the instanced buffer to write into it
		D3D11_MAPPED_SUBRESOURCE mappedData;
		HRESULT hr = pContext->Map(pInstancedBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
		Assert::NotFailed(hr, "can't map the instanced buffer");

		BuffTypes::InstancedDataBillboards* dataView = (BuffTypes::InstancedDataBillboards*)mappedData.pData;

		// write data into the subresource
		for (int i = 0; i < count; ++i)
			dataView[i].material = materials[i];

		for (int i = 0; i < count; ++i)
			dataView[i].posW = positions[i];

		for (int i = 0; i < count; ++i)
			dataView[i].size = sizes[i];


		numCurrentInstances_ = count;

		pContext->Unmap(pInstancedBuffer_, 0);
	}
	catch (LIB_Exception& e)
	{
		Log::Error(e);
		Log::Error("can't update instanced buffer for billboards rendering");
	}
}


// ====================================================================================
//                             PUBLIC RENDERING API
// ====================================================================================
void BillboardShader::Render(
	ID3D11DeviceContext* pContext,
	ID3D11Buffer* pBillboardVB,
	ID3D11Buffer* pBillboardIB,
	SRV* const* ppTextureArrSRV,
	const UINT stride,            // vertex stride
	const DirectX::XMFLOAT3 position)
{
	//
	// render a single billboard
	//

	// bind input layout, shaders, samplers
	pContext->IASetInputLayout(vs_.GetInputLayout());
	pContext->VSSetShader(vs_.GetShader(), nullptr, 0);
	pContext->GSSetShader(gs_.GetShader(), nullptr, 0);
	pContext->PSSetShader(ps_.GetShader(), nullptr, 0);
	pContext->PSSetSamplers(0, 1, samplerState_.GetAddressOf());
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT offset = 0;

	// bind vertex/index buffer and textures 2D array as well
	pContext->IASetVertexBuffers(0, 1, &pBillboardVB, &stride, &offset);
	pContext->IASetIndexBuffer(pBillboardIB, DXGI_FORMAT_R32_UINT, 0);
	pContext->PSSetShaderResources(0, 1, ppTextureArrSRV);

	// draw a billboard
	pContext->DrawIndexed(4, 0, 0);

	// unbind the geometry shader
	pContext->GSSetShader(nullptr, nullptr, 0);
}

///////////////////////////////////////////////////////////

void BillboardShader::Render(ID3D11DeviceContext* pContext, const Instance& instance)
{
	// bind input layout, shaders, samplers
	pContext->IASetInputLayout(vs_.GetInputLayout());
	pContext->VSSetShader(vs_.GetShader(), nullptr, 0);
	pContext->GSSetShader(gs_.GetShader(), nullptr, 0);
	pContext->PSSetShader(ps_.GetShader(), nullptr, 0);
	pContext->PSSetSamplers(0, 1, samplerState_.GetAddressOf());
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// bind vertex/index buffer and textures 2D array as well
	const UINT instanceBufElemSize = sizeof(BuffTypes::InstancedDataBillboards);
	ID3D11Buffer* const vbs[2]     = { instance.pVB, pInstancedBuffer_ };
	const UINT stride[2]           = { instance.vertexStride, instanceBufElemSize };
	const UINT offset[2]           = { 0,0 };

	pContext->IASetVertexBuffers(0, 2, vbs, stride, offset);
	pContext->IASetIndexBuffer(instance.pIB, DXGI_FORMAT_R32_UINT, 0);
	pContext->PSSetShaderResources(0, 1, instance.texSRVs.data());

	// draw billboard instances
	pContext->DrawIndexedInstanced(4, numCurrentInstances_,	0, 0, 0);

	// unbind the geometry shader
	pContext->GSSetShader(nullptr, nullptr, 0);
}


// ====================================================================================
//                         PRIVATE MODIFICATION API
// ====================================================================================
void BillboardShader::Shutdown()
{
	SafeRelease(&pInstancedBuffer_);
	instancedData_.clear();
}

///////////////////////////////////////////////////////////

void BillboardShader::InitializeShaders(
	ID3D11Device* pDevice,
	const std::string& vsFilename,
	const std::string& gsFilename,
	const std::string& psFilename)
{
	// initialized the vertex shader, pixel shader, input layout, 
	// sampler state, and different constant buffers

	bool result = false;

	const UINT layoutElemNum = 8;
	const D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
	{
		// per vertex data
		{"POSITION_L", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"SIZE_L",     0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},

		// per instance data
		{"MATERIAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1,                            0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"MATERIAL", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"MATERIAL", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"MATERIAL", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},

		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"SIZE",     0, DXGI_FORMAT_R32G32_FLOAT,    1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	};


	// check yourself
	Assert::True(layoutElemNum == ARRAYSIZE(inputLayoutDesc), "layout elems num != layout desc array size");

	// initialize: VS, PS, sampler state
	result = vs_.Initialize(pDevice, vsFilename, inputLayoutDesc, layoutElemNum);
	Assert::True(result, "can't initialize the vertex shader");

	result = gs_.Initialize(pDevice, gsFilename);
	Assert::True(result, "can't initialize the geometry shader");

	result = ps_.Initialize(pDevice, psFilename);
	Assert::True(result, "can't initialize the pixel shader");

	result = samplerState_.Initialize(pDevice);
	Assert::True(result, "can't initialize the sampler state");

	// --------------------------------------------

	// create instances buffer
	D3D11_BUFFER_DESC desc;

	// setup buffer's description
	desc.Usage               = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth           = static_cast<UINT>(sizeof(BuffTypes::InstancedDataBillboards) * numMaxInstances_);
	desc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags           = 0;
	desc.StructureByteStride = 0;

	HRESULT hr = pDevice->CreateBuffer(&desc, nullptr, &pInstancedBuffer_);
	Assert::NotFailed(hr, "can't create an instanced buffer");
}

} // namespace Render