// =================================================================================
// Filename:     LightShader.h
// Description:  this class is needed for rendering textured models 
//               with simple DIFFUSE light on it using HLSL shaders.
// Created:      09.04.23
// =================================================================================
#pragma once

#include "VertexShader.h"
#include "PixelShader.h"
#include "SamplerState.h"        // for using the ID3D11SamplerState 

#include "../Common/RenderTypes.h"
#include <d3d11.h>


namespace Render
{

class LightShader
{
public:
	LightShader();
	~LightShader();

	// restrict copying of this class instance
	LightShader(const LightShader&) = delete;
	LightShader& operator=(const LightShader&) = delete;

	// ----------------------------------------------------

    bool Initialize(
        ID3D11Device* pDevice,
        const char* vsFilePath,
        const char* psFilePath);

	void Render(
		ID3D11DeviceContext* pContext,
		ID3D11Buffer* pInstancedBuffer,
		const Instance* instances,
		const int numUniqueGeometry,
		const UINT instancesBuffElemSize);

	inline const char* GetShaderName() const { return className_; }

private:
	void InitializeShaders(
		ID3D11Device* pDevice,
		const char* vsFilePath,
		const char* psFilePath);

private:
	VertexShader vs_;
	PixelShader  ps_;
	SamplerState samplerState_;          // a sampler for texturing

	char className_[32]{"LightShader"};
};


} // namespace Render
