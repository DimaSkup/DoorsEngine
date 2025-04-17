// =================================================================================
// Filename:     SkyDomeShader.h
// Description:  this shader is used to render sky dome model (sky)
// 
// Created:      21.12.24
// =================================================================================
#pragma once

#include "VertexShader.h"
#include "PixelShader.h"
#include "SamplerState.h"        // for using the ID3D11SamplerState 
#include "ConstantBuffer.h"

#include "../Common/ConstBufferTypes.h"
#include "../Common/RenderTypes.h"

#include <d3d11.h>


namespace Render
{

class SkyDomeShader
{
public:
	SkyDomeShader();
	~SkyDomeShader();

	// restrict a copying of this class instance
	SkyDomeShader(const SkyDomeShader& obj) = delete;
	SkyDomeShader& operator=(const SkyDomeShader& obj) = delete;

	// ----------------------------------------------------

    bool Initialize(
        ID3D11Device* pDevice,
        const char* vsFilePath,
        const char* psFilePath);

	void Render(
		ID3D11DeviceContext* pContext,
		const SkyInstance& sky,                  // we will render only one instance: sky model
		const DirectX::XMMATRIX& worldViewProj);     

	//
	// setters
	//
	void SetSkyGradient(
		ID3D11DeviceContext* pContext,
		const DirectX::XMFLOAT3& colorCenter,
		const DirectX::XMFLOAT3& colorApex);

	void SetSkyColorCenter(ID3D11DeviceContext* pContext, const DirectX::XMFLOAT3& color);
	void SetSkyColorApex  (ID3D11DeviceContext* pContext, const DirectX::XMFLOAT3& color);


	//
	// inline getters
	//
	inline const char* GetShaderName()                       const { return className_; }
	inline ID3D11Buffer* const GetConstBufferVSPerFrame()    const { return cbvsPerFrame_.Get(); }
	inline ID3D11Buffer* const GetConstBufferPSRareChanged() const { return cbpsRareChanged_.Get(); }

	inline const DirectX::XMFLOAT3& GetColorCenter() const 
	{ 
		return cbpsRareChanged_.data.colorCenter_; 
	}

	inline const DirectX::XMFLOAT3& GetColorApex() const 
	{ 
		return cbpsRareChanged_.data.colorApex_;
	}

private:
	void InitializeShaders(
		ID3D11Device* pDevice,
        const char* vsFilePath,
        const char* psFilePath);

private:
	VertexShader vs_;
	PixelShader  ps_;
	SamplerState samplerState_;                                          // a sampler for texturing

	ConstantBuffer<BuffTypes::cbvsPerFrame_SkyDome>    cbvsPerFrame_;    // cbvs: const buffer for vertex shader
	ConstantBuffer<BuffTypes::cbpsRareChanged_SkyDome> cbpsRareChanged_; // cbps: const buffer for pixel shader

	char className_[32]{"SkyDomeShader"};
};


} // namespace Render
