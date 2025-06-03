// *********************************************************************************
// Filename:     ConstantBuffer.h
// Description:  this class is needed for easier using of 
//               constant buffers for HLSL shaders;
// *********************************************************************************
#pragma once

#include <Log.h>
#include <d3d11.h>
#include <DirectXMath.h>


namespace Render
{

template<class T>
class ConstantBuffer
{
public:
	ConstantBuffer() {};
	~ConstantBuffer() { SafeRelease(&pBuffer_); };

	// restrict a copying of this class instance
	ConstantBuffer(const ConstantBuffer<T>& rhs) = delete;
	ConstantBuffer& operator=(const ConstantBuffer& obj) = delete;

	// initialize a constant buffer
	HRESULT Initialize(ID3D11Device* pDevice); 

	void ApplyChanges(ID3D11DeviceContext* pContext);  // update the constant buffer data

	inline ID3D11Buffer* Get() const { return pBuffer_; }
	inline ID3D11Buffer* const* GetAddressOf() const { return &pBuffer_; }

	T data;                                                  // here is placed data for a HLSL constant buffer

private:
	ID3D11Buffer* pBuffer_ = nullptr;
};

// *********************************************************************************


template<class T>
HRESULT ConstantBuffer<T>::Initialize(ID3D11Device* pDevice)
{
	// initialize a constant buffer

	D3D11_BUFFER_DESC desc;
	HRESULT hr = S_OK;

	// if the constant buffer has already been initialized before
	SafeRelease(&pBuffer_);

	desc.ByteWidth           = static_cast<UINT>(sizeof(T) + (16 - (sizeof(T) % 16)));
	desc.Usage               = D3D11_USAGE_DYNAMIC;
	desc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags           = 0;
	desc.StructureByteStride = 0;

	hr = pDevice->CreateBuffer(&desc, 0, &pBuffer_);

    if (FAILED(hr))
        LogErr("can't create a constant buffer");

    return hr;
}

///////////////////////////////////////////////////////////

template<class T>
void ConstantBuffer<T>::ApplyChanges(ID3D11DeviceContext* pContext)
{
	// update the constant buffer data

    if (!pBuffer_)
    {
        LogErr("ptr to buffer == nullptr");
        return;
    }

	D3D11_MAPPED_SUBRESOURCE mappedResource;

	HRESULT hr = pContext->Map(pBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(hr))
    {
        LogErr("failed to Map the constant buffer");
        return;
    }
	
	CopyMemory(mappedResource.pData, &data, sizeof(T));
	pContext->Unmap(pBuffer_, 0);
}


} // namespace Render
