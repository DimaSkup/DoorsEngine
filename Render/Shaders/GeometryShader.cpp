// =================================================================================
// Filename: GeometryShader.cpp
// Created:  01.03.25  by DimaSkup
// =================================================================================
#include "GeometryShader.h"

#include "../Common/MemHelpers.h"
#include "../Common/Assert.h"
#include "../Common/Log.h"

#include "Helpers/CSOLoader.h"

namespace Render
{

GeometryShader::~GeometryShader()
{
	Shutdown();
}

///////////////////////////////////////////////////////////

bool GeometryShader::Initialize(ID3D11Device* pDevice, const std::string& shaderPath)
{
	// THIS FUNC compiles/load an HLSL/CSO shader by shaderPath;
	// compiles this shader into buffer, and then creates a geometry shader object

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

		HRESULT hr = pDevice->CreateGeometryShader(
			pShaderBuffer_,                 //pShaderBuffer_->GetBufferPointer(),
			len,                            //pShaderBuffer_->GetBufferSize(),
			nullptr,
			&pShader_);

		Assert::NotFailed(hr, "Failed to create a geometry shader obj: " + shaderPath);
	}
	catch (LIB_Exception& e)
	{
		Log::Error(e, true);
		Shutdown();

		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////

void GeometryShader::Shutdown()
{
	// Shutting down of the class object, releasing of the memory, etc.

	Log::Debug("Shutdown");

	SafeDeleteArr(pShaderBuffer_);
	SafeRelease(&pShader_);
}

}; // namespace Render