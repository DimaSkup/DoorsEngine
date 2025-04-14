////////////////////////////////////////////////////////////////////
// Filename: VertexShader.cpp
// Revising: 05.11.22
////////////////////////////////////////////////////////////////////
#include "VertexShader.h"

#include "../Common/MemHelpers.h"
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
	const char* shaderPath,
	const D3D11_INPUT_ELEMENT_DESC* layoutDesc,
	const UINT layoutElemNum)
{
	// THIS FUNC compiles/load an HLSL/CSO shader by shaderPath;
	// compiles this shader into buffer, and then creates
	// a vertex shader object and an input layout using this buffer;

    if ((shaderPath == nullptr) || (shaderPath[0] == '\0'))
    {
        LogErr("input path to vertex shader file is empty!");
        return false;
    }

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
	const bool result = LoadCSO(shaderPath, &pShaderBuffer_, len);
    if (!result)
    {
        sprintf(g_String, "Failed to load .CSO-file of vertex shader: %s", shaderPath);
        LogErr(g_String);
        Shutdown();
        return false;
    }

	// --------------------------------------------

	HRESULT hr = pDevice->CreateVertexShader(
		pShaderBuffer_,                 //pShaderBuffer_->GetBufferPointer(),
		len,                            //pShaderBuffer_->GetBufferSize(),
		nullptr,
		&pShader_);

    if (FAILED(hr))
    {
        sprintf(g_String, "Failed to create a vertex shader obj: %s", shaderPath);
        LogErr(g_String);
        Shutdown();
        return false;
    }

	// --------------------------------------------
		
	hr = pDevice->CreateInputLayout(
		layoutDesc, 
		layoutElemNum,
		pShaderBuffer_,                 //pShaderBuffer_->GetBufferPointer(),
		len,                            //pShaderBuffer_->GetBufferSize(),
		&pInputLayout_);
    if (FAILED(hr))
    {
        sprintf(g_String, "Failed to create the input layout for vertex shader: %s", shaderPath);
        LogErr(g_String);
        Shutdown();
        return false;
    }

	return true;  
}

///////////////////////////////////////////////////////////

void VertexShader::Shutdown()
{
	// Shutting down of the class object, releasing of the memory, etc.

	LogDbg("Shutdown");

	SafeRelease(&pInputLayout_);
	SafeDeleteArr(pShaderBuffer_);
	SafeRelease(&pShader_);
}

}; // namespace Render
