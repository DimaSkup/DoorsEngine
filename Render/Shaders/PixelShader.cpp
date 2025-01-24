////////////////////////////////////////////////////////////////////
// Filename: PixelShader.cpp
// Revising: 05.11.22
////////////////////////////////////////////////////////////////////
#include "PixelShader.h"

#include "../Common/MemHelpers.h"
#include "../Common/Assert.h"
#include "../Common/Log.h"

#include "Helpers/CSOLoader.h"

namespace Render
{

PixelShader::PixelShader()
{
}

PixelShader::~PixelShader()
{
	Shutdown();
}

///////////////////////////////////////////////////////////

bool PixelShader::Initialize(
	ID3D11Device* pDevice,
	const std::string& shaderPath,
	const std::string& funcName)
{
	// initializing of a pixel shader object

	try
	{
#if 0
		std::string errorMgr;

		// loading of the shader code
		hr = ShaderClass::CompileShaderFromFile(
			shaderPath.c_str(),
			funcName.c_str(),
			"ps_5_0",
			&pShaderBuffer_,
			errorMgr);
		Assert::NotFailed(hr, errorMgr);
#endif

		std::streampos len = 0;
		
		// load in shader bytecode
		LoadCSO(shaderPath, &pShaderBuffer_, len);

		// --------------------------------------------

		HRESULT hr = pDevice->CreatePixelShader(
			pShaderBuffer_,
			len,
			nullptr,
			&pShader_);

		Assert::NotFailed(hr, "Failed to create a pixel shader obj: " + shaderPath);
	}
	catch (LIB_Exception& e)
	{
		Shutdown();

		Log::Error(e, true);
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////

void PixelShader::Shutdown()
{
	SafeRelease(&pShader_);
	SafeDeleteArr(pShaderBuffer_);
}


} // namespace Render