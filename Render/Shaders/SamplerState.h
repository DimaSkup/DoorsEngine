// *********************************************************************************
// Filename:     SamplerState.h
// Description:  functional for work with ID3D11SamplerState
// 
// *********************************************************************************
#pragma once

#include <d3d11.h>

namespace Render
{

class SamplerState
{
public:
	SamplerState();
	~SamplerState();

	bool Initialize(ID3D11Device* pDevice, D3D11_SAMPLER_DESC* pSamplerDesc = nullptr);

	inline ID3D11SamplerState* GetSampler() { return pSamplerState_; };
	inline ID3D11SamplerState* const* GetAddressOf() { return &pSamplerState_; };

private:
	ID3D11SamplerState* pSamplerState_ = nullptr;
};


} // namespace Render