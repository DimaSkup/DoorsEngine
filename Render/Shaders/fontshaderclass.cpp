// ***********************************************************************************
// Filename: fontshaderclass.cpp
// Revising: 23.07.22
// ***********************************************************************************
#include "fontshaderclass.h"
#include "shaderclass.h"

#include "../Common/Log.h"
#include "../Common/Types.h"


namespace Render
{

FontShaderClass::FontShaderClass() : className_(__func__)
{
}

FontShaderClass::~FontShaderClass() 
{
}


// =================================================================================
//                             PUBLIC MODIFICATION API
// =================================================================================

bool FontShaderClass::Initialize(
	ID3D11Device* pDevice, 
	ID3D11DeviceContext* pContext,
	const DirectX::XMMATRIX& WVO)            // world * base_view * ortho
{
	// Initialize() initializes the vertex and pixel shaders, input layout,
	// sampler state, matrix and pixel buffers

	try
	{
		const std::string vsFilename = ShaderClass::pathToShadersDir_ + "fontVS.cso";
		const std::string psFilename = ShaderClass::pathToShadersDir_ + "fontPS.cso";

		// create shader objects, buffers, etc.
		InitializeShaders(pDevice, pContext, vsFilename, psFilename, WVO);
	}
	catch (LIB_Exception & e)
	{
		Log::Error(e, true);
		Log::Error("can't initialize the font shader class");
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////

void FontShaderClass::SetWorldViewOrtho(
	ID3D11DeviceContext* pContext,
	const DirectX::XMMATRIX& WVO)
{
	// prepare matrices for using in the vertex shader
	// (the WVO matrix must be already transposed)
	matrixBuffer_.data.worldViewProj = WVO;
	matrixBuffer_.ApplyChanges(pContext);
}

///////////////////////////////////////////////////////////

void FontShaderClass::SetFontColor(
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

void FontShaderClass::Render(
	ID3D11DeviceContext* pContext,
	const std::vector<ID3D11Buffer*>& textVBs,    // array of text vertex buffers
	const std::vector<ID3D11Buffer*>& textIBs,    // array of text indices buffers
	const std::vector<uint32_t>& indexCounts,
	const uint32_t fontVertexSize,
	SRV* const* ppFontTexSRV)
{
	// renders fonts on the screen using HLSL shaders

	try
	{
		// bind vertex and pixel shaders for rendering
		pContext->VSSetShader(vs_.GetShader(), nullptr, 0U);
		pContext->PSSetShader(ps_.GetShader(), nullptr, 0U);

		// set the primitive topology for all the sentences and the input layout
		pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pContext->IASetInputLayout(vs_.GetInputLayout());

		// set the sampler state for the pixel shader
		pContext->PSSetSamplers(0, 1, samplerState_.GetAddressOf());
		pContext->PSSetShaderResources(0, 1, ppFontTexSRV);


		const size numTextStrs = std::ssize(textVBs);
		Assert::True(numTextStrs == std::ssize(textIBs), "the number of vertex buffers must be equal to the number of index buffers");

		const UINT stride = fontVertexSize;
		const UINT offset = 0;

		for (index idx = 0; idx < numTextStrs; ++idx)
		{
			// bind vb/ib
			pContext->IASetVertexBuffers(0, 1, &textVBs[idx], &stride, &offset);
			pContext->IASetIndexBuffer(textIBs[idx], DXGI_FORMAT_R32_UINT, 0);

			// render the fonts on the screen
			pContext->DrawIndexed(indexCounts[idx], 0, 0);
		}
	}
	catch (LIB_Exception& e)
	{
		Log::Error(e, true);
		throw LIB_Exception("can't render using the shader");
	}
}


// =================================================================================
//                              PRIVATE METHODS
// =================================================================================

void FontShaderClass::InitializeShaders(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext,
	const std::string& vsFilename,
	const std::string& psFilename,
	const DirectX::XMMATRIX& WVO)        // world * base_view * ortho
{
	// InitializeShaders() helps to initialize the vertex and pixel shaders,
	// input layout, sampler state, matrix and pixel buffers

	bool result = false;
	HRESULT hr = S_OK;
	const UINT layoutElemNum = 2;

	// a description of the vertex input layout
	D3D11_INPUT_ELEMENT_DESC layoutDesc[layoutElemNum]; 

	// --------------------------  INPUT LAYOUT DESC  ---------------------------------

	// setup description of the vertex input layout 
	layoutDesc[0].SemanticName = "POSITION";
	layoutDesc[0].SemanticIndex = 0;
	layoutDesc[0].Format = DXGI_FORMAT_R32G32_FLOAT;
	layoutDesc[0].InputSlot = 0;
	layoutDesc[0].AlignedByteOffset = 0;
	layoutDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	layoutDesc[0].InstanceDataStepRate = 0;

	layoutDesc[1].SemanticName = "TEXCOORD";
	layoutDesc[1].SemanticIndex = 0;
	layoutDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	layoutDesc[1].InputSlot = 0;
	layoutDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	layoutDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	layoutDesc[1].InstanceDataStepRate = 0;


	// -----------------------  SHADERS / SAMPLER STATE  ------------------------------

	// initialize the vertex shader
	result = vs_.Initialize(pDevice, vsFilename, layoutDesc, layoutElemNum);
	Assert::True(result, "can't initialize the vertex shader");

	// initialize the pixel shader
	result = ps_.Initialize(pDevice, psFilename);
	Assert::True(result, "can't initialize the pixel shader");

	// initialize the sampler state
	result = samplerState_.Initialize(pDevice);
	Assert::True(result, "can't initialize the sampler state");



	// ---------------------------  CONSTANT BUFFERS  ---------------------------------

	// initialize the matrix buffer
	hr = matrixBuffer_.Initialize(pDevice, pContext);
	Assert::NotFailed(hr, "can't initialize the matrix buffer");
	
	// initialize the pixel buffer
	hr = pixelBuffer_.Initialize(pDevice, pContext);
	Assert::NotFailed(hr, "can't initialize the pixel buffer");
	

	// ---------------- SET DEFAULT PARAMS FOR CONST BUFFERS --------------------------

	SetFontColor(pContext, { 1, 1, 1 });  // set white colour by default
	SetWorldViewOrtho(pContext, WVO);
}

}