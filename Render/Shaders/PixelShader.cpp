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
	const char* shaderPath,
	const char* funcName)
{
	// initializing of a pixel shader object

    if ((shaderPath == nullptr) || (shaderPath[0] == '\0'))
    {
        LogErr("input path to pixel shader file is empty!");
        return false;
    }
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
	const bool result = LoadCSO(shaderPath, &pShaderBuffer_, len);
    if (!result)
    {
        sprintf(g_String, "Failed to load .CSO-file of pixel shader: %s", shaderPath);
        LogErr(g_String);
        Shutdown();
        return false;
    }

	// --------------------------------------------

	HRESULT hr = pDevice->CreatePixelShader(
		pShaderBuffer_,
		len,
		nullptr,
		&pShader_);

    if (FAILED(hr))
    {
        sprintf(g_String, "Failed to create a pixel shader obj: %s", shaderPath);
        LogErr(g_String);
        Shutdown();
        return false;
    }

	return true;
}

///////////////////////////////////////////////////////////

void PixelShader::Shutdown()
{
    LogDbg("shutdown");
	SafeRelease(&pShader_);
	SafeDeleteArr(pShaderBuffer_);
}

} // namespace 
