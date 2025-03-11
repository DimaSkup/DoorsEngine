// =================================================================================
// Filename:     LightShaderClass.h
// Description:  this class is needed for rendering textured models 
//               with simple DIFFUSE light on it using HLSL shaders.
// Created:      09.04.23
// =================================================================================
#pragma once

#include "VertexShader.h"
#include "PixelShader.h"
#include "SamplerState.h"        // for using the ID3D11SamplerState 

#include "../Common/MemHelpers.h"
#include "../Common/MaterialLightTypes.h"
#include "../Common/RenderTypes.h"

#include <d3d11.h>
#include <DirectXMath.h>

namespace Render
{


class LightShaderClass final
{
public:
	LightShaderClass();
	~LightShaderClass();

	// restrict a copying of this class instance
	LightShaderClass(const LightShaderClass& obj) = delete;
	LightShaderClass& operator=(const LightShaderClass& obj) = delete;

	// ----------------------------------------------------

	bool Initialize(ID3D11Device* pDevice);

	void Render(
		ID3D11DeviceContext* pContext,
		ID3D11Buffer* pInstancedBuffer,
		const Instance* instances,
		const int numInstances,
		const UINT instancesBuffElemSize);

	inline const std::string& GetShaderName() const { return className_; }

private:
	void InitializeShaders(
		ID3D11Device* pDevice,
		const std::string& vsFilePath,
		const std::string& psFilePath);

private:
	VertexShader vs_;
	PixelShader  ps_;
	SamplerState samplerState_;                     // a sampler for texturing

	const std::string className_{ "light_shader" };
};


} // namespace Render