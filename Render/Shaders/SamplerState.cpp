// *********************************************************************************
// Filename:     SamplerState.cpp
// Description:  implementation of the SamplerState class 
// 
// *********************************************************************************
#include "../Common/pch.h"
#include "SamplerState.h"

namespace Render
{

SamplerState::SamplerState()
{
}

SamplerState::~SamplerState()
{
    SafeRelease(&pSamplerState_);
}

//---------------------------------------------------------
// Desc:  initialize a sampler state object
// NOTE:  if we pass a ptr to the sampler desc structure we will use it for creation;
//        in another case we will use default params
// Args:  - pSamplerDesc:  a description for the sampler state
//---------------------------------------------------------
bool SamplerState::Initialize(ID3D11Device* pDevice, D3D11_SAMPLER_DESC* pSamplerDesc)
{

    HRESULT hr = S_OK;

    // if we didn't pass any description as input parameter so use the default one
    if (pSamplerDesc == nullptr)
    {
        D3D11_SAMPLER_DESC samplerDesc {};

        samplerDesc.Filter =  D3D11_FILTER_ANISOTROPIC;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.BorderColor[0] = 0.0f;
        samplerDesc.BorderColor[1] = 0.0f;
        samplerDesc.BorderColor[2] = 0.0f;
        samplerDesc.BorderColor[3] = 0.0f;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NEVER;
        samplerDesc.MinLOD = 0.0f;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
        samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
        samplerDesc.MipLODBias = 0.0f;

        hr = pDevice->CreateSamplerState(&samplerDesc, &pSamplerState_);
    }

    // we passed into this function some specific sampler state description so use it
    else
    {
        hr = pDevice->CreateSamplerState(pSamplerDesc, &pSamplerState_);
    }

    // check if we managed to create a sampler state
    if (FAILED(hr))
    {
        LogErr(LOG, "can't create a sampler state");
        return false;
    }

    return true;
}

} // namespace
