// *********************************************************************************
// Filename:     ConstantBuffer.h
// Description:  this class is needed for easier using of 
//               constant buffers for HLSL shaders;
// *********************************************************************************
#pragma once

#include <Log.h>
#include <d3d11.h>


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


    HRESULT Init(ID3D11Device* pDevice); 
    void ApplyChanges(ID3D11DeviceContext* pContext); 

    inline ID3D11Buffer*        Get()       const { return pBuffer_; }
    inline ID3D11Buffer* const* GetAddrOf() const { return &pBuffer_; }

    // here is placed data for a HLSL constant buffer
    T data;   

private:
    ID3D11Buffer* pBuffer_ = nullptr;
};


//---------------------------------------------------------
// Desc:  setup description for const buffer and init it
//---------------------------------------------------------
template<class T>
HRESULT ConstantBuffer<T>::Init(ID3D11Device* pDevice)
{
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
        LogErr(LOG, "can't create a constant buffer");

    return hr;
}

//---------------------------------------------------------
// Desc:  update the constant buffer data (transfer data to GPU)
//---------------------------------------------------------
template<class T>
void ConstantBuffer<T>::ApplyChanges(ID3D11DeviceContext* pContext)
{
    if (!pBuffer_)
    {
        LogErr(LOG, "ptr to buffer == nullptr");
        return;
    }

    D3D11_MAPPED_SUBRESOURCE mappedResource;

    HRESULT hr = pContext->Map(pBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(hr))
    {
        LogErr(LOG, "failed to Map the constant buffer");
        return;
    }
    
    CopyMemory(mappedResource.pData, &data, sizeof(T));
    pContext->Unmap(pBuffer_, 0);
}


} // namespace Render
