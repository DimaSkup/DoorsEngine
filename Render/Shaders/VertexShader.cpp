////////////////////////////////////////////////////////////////////
// Filename: VertexShader.cpp
// Revising: 05.11.22
////////////////////////////////////////////////////////////////////
#include "VertexShader.h"

#include "../Common/MemHelpers.h"
#include "../Common/Assert.h"
#include "../Common/Log.h"

#include "Helpers/CSOLoader.h"

namespace Render
{


VertexShader::~VertexShader()
{
	Shutdown();
}

///////////////////////////////////////////////////////////

bool VertexShader::Initialize(
	ID3D11Device* pDevice,
	const std::string& shaderPath,
	const D3D11_INPUT_ELEMENT_DESC* layoutDesc,
	const UINT layoutElemNum)
{
	// THIS FUNC compiles/load an HLSL/CSO shader by shaderPath;
	// compiles this shader into buffer, and then creates
	// a vertex shader object and an input layout using this buffer;

	try
	{
#if 0
		// compile a vertex shader into the buffer
		hr = ShaderClass::CompileShaderFromFile(
			shaderPath.c_str(), 
			funcName.c_str(),
			"vs_5_0",
			&pShaderBuffer_,
			errorMgr);
		Assert::NotFailed(hr, errorMgr);
#endif

		
		std::streampos len = 0;

		// load in shader bytecode
		LoadCSO(shaderPath, &pShaderBuffer_, len);

		// --------------------------------------------

		HRESULT hr = pDevice->CreateVertexShader(
			pShaderBuffer_,                 //pShaderBuffer_->GetBufferPointer(),
			len,                            //pShaderBuffer_->GetBufferSize(),
			nullptr,
			&pShader_);

		// check if we managed to create a vertex shader
		Assert::NotFailed(hr, "Failed to create a vertex shader obj: " + shaderPath);

		// --------------------------------------------
		
		hr = pDevice->CreateInputLayout(
			layoutDesc, 
			layoutElemNum,
			pShaderBuffer_,                 //pShaderBuffer_->GetBufferPointer(),
			len,                            //pShaderBuffer_->GetBufferSize(),
			&pInputLayout_);
		
		Assert::NotFailed(hr, "can't create the input layout for vs: " + shaderPath);


	}
	catch (LIB_Exception & e)
	{
		Log::Error(e, true);
		Shutdown();

		return false;
	}

	return true;  
}

///////////////////////////////////////////////////////////

void VertexShader::Shutdown()
{
	// Shutting down of the class object, releasing of the memory, etc.

	Log::Debug("Shutdown");

	SafeRelease(&pInputLayout_);
	SafeDeleteArr(pShaderBuffer_);
	SafeRelease(&pShader_);
}

}; // namespace Render
