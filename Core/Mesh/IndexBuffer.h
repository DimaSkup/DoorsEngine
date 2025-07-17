// =================================================================================
// Filename:     IndexBuffer.h
// Description:  this class is needed for easier using of 
//               indices buffers for models;
// 
// Created:      
// =================================================================================
#pragma once

#include <MemHelpers.h>
#include <log.h>
#include <CAssert.h>

#include <d3d11.h>
#include <memory>   // for using std::construct_at
#include <utility>  // for using std::exchange


namespace Core
{

template <typename T>
class IndexBuffer
{
public:
    IndexBuffer() {};

    // ---------------------------------------------

    inline IndexBuffer(ID3D11Device* pDevice, const T* indices, const int numIndices)
    {
        CAssert::True((indices != nullptr) && (numIndices > 0), "wrong input data");

        constexpr bool isDynamic = false;
        Initialize(pDevice, indices, numIndices, isDynamic);
    }

    // ---------------------------------------------

    inline ~IndexBuffer()
    {
        Shutdown();
    }

    // ---------------------------------------------

    inline IndexBuffer(IndexBuffer&& rhs) noexcept :
        pBuffer_(std::exchange(rhs.pBuffer_, nullptr)),
        indexCount_(std::exchange(rhs.indexCount_, 0)),
        usageType_(rhs.usageType_)
    {
    }

    // ---------------------------------------------

    inline IndexBuffer& operator=(IndexBuffer&& rhs) noexcept
    {
        // move assignment
        if (this != &rhs)
        {
            this->Shutdown();                    // lifetime of *this ends
            std::construct_at(this, std::move(rhs));
        }

        return *this;
    }

    // ---------------------------------------------

    // restrict a copying of this class instance 
    //(you have to execute copyting using the CopyBuffer() function)
    IndexBuffer& operator=(const IndexBuffer& obj) = delete;
    IndexBuffer(const IndexBuffer& rhs) = delete;

    // ---------------------------------------------

    // Public modification API 
    bool Initialize(
        ID3D11Device* pDevice,
        const T* indices,
        const int numIndices,
        const bool isDynamic);

    bool Update(
        ID3D11DeviceContext* pContext,
        T* indices,
        const size count);

    void CopyBuffer(
        ID3D11Device* pDevice,
        ID3D11DeviceContext* pContext,
        const IndexBuffer& srcBuffer);

    inline void Shutdown()
    {
        SafeRelease(&pBuffer_);
        indexCount_ = 0;
    }

    // Public query API  
    inline ID3D11Buffer*        Get()           const { return pBuffer_; }
    inline ID3D11Buffer* const* GetAddressOf()  const { return &pBuffer_; }
    inline UINT                 GetIndexCount() const { return indexCount_; }

private:
    bool InitializeHelper(
        ID3D11Device* pDevice,
        const D3D11_BUFFER_DESC& buffDesc,
        const T* pIndices);

private:
    ID3D11Buffer* pBuffer_    = nullptr;
    UINT          indexCount_ = 0;
    D3D11_USAGE   usageType_  = D3D11_USAGE_DEFAULT;
};



//---------------------------------------------------------
// Desc:    initialize index buffer with input indices data
// Args:    - pDevice:      a ptr to DirectX11 device
//          - indices:      an array of indices
//          - numIndices:   the number of indices in input array
//          - isDynamic:    a flag to define if we want to create a dynamic buffer
//---------------------------------------------------------
template <typename T>
bool IndexBuffer<T>::Initialize(
    ID3D11Device* pDevice,
    const T* indices,
    const int numIndices,
    const bool isDynamic)
{
    // check input args
    if (!indices)
    {
        LogErr(LOG, "can't init index buffer: input ptr to indices arr == nullptr");
        return false;
    }

    if (numIndices <= 0)
    {
        LogErr(LOG, "can't init index buffer: input number of indices <= 0");
        return false;
    }


    indexCount_ = (UINT)numIndices;
    usageType_  = (isDynamic) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;

    // setup the index buffer description
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth           = sizeof(T) * indexCount_;
    desc.Usage               = usageType_;
    desc.BindFlags           = D3D11_BIND_INDEX_BUFFER;
    desc.CPUAccessFlags      = (isDynamic) ? D3D11_CPU_ACCESS_WRITE : 0;
    desc.MiscFlags           = 0;
    desc.StructureByteStride = 0;

    // create and initialize a buffer with data
    if (!InitializeHelper(pDevice, desc, indices))
    {
        LogErr(LOG, "something went wrong during creation of the index buffer");
        Shutdown();
        return false;
    }

    return true;
}

//---------------------------------------------------------
// Desc:   update buffer's content (only for dynamic usage type)
// Args:   - pContext:  a ptr to DirectX11 device context
//         - indices:   array of indices
//         - count:     how many indices will we write
// Ret:    true if we successfully updated the buffer
//---------------------------------------------------------
template <typename T>
bool IndexBuffer<T>::Update(
    ID3D11DeviceContext* pContext,
    T* indices,
    const size count)
{
    // check some params
    if (usageType_ != D3D11_USAGE_DYNAMIC)
    {
        LogErr(LOG, "can't update index buffer: not dynamic usage type");
        return false;
    }

    if (!indices || (count > indexCount_))
    {
        LogErr(LOG, "can't update index buffer: invalid input args");
        return false;
    }

    // map the buffer
    D3D11_MAPPED_SUBRESOURCE mappedResource;

    const HRESULT hr = pContext->Map(pBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(hr))
    {
        LogErr(LOG, "failed to map the index buffer");
        return false;
    }

    // copy new data into the buffer
    CopyMemory(mappedResource.pData, indices, sizeof(T) * count);

    pContext->Unmap(pBuffer_, 0);

    return true;
}

///////////////////////////////////////////////////////////

template <typename T>
void IndexBuffer<T>::CopyBuffer(
    ID3D11Device* pDevice,
    ID3D11DeviceContext* pDeviceContext,
    const IndexBuffer& srcBuffer)
{
    // this function copies data from the inOriginBuffer into the current one
    // and creates a new index buffer using this data;

    // copy the main data from the origin buffer
    ID3D11Buffer* pOrigBuffer = srcBuffer.Get();
    const UINT origIndexCount = srcBuffer.GetIndexCount();

    // check input params
    CAssert::NotZero(origIndexCount, "there is no indices in the inOriginBuffer");

    HRESULT                  hr = S_OK;
    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    D3D11_BUFFER_DESC        dstBufferDesc;
    ID3D11Buffer*            pStagingBuffer = nullptr;
    T*                       indicesArr     = nullptr;   // will be filled with indices for a destination buffer

    try
    {
        /////////////////  CREATE A STAGING BUFFER AND COPY DATA INTO IT  /////////////////

        // setup the staging buffer description
        D3D11_BUFFER_DESC stagingBufferDesc;
        ZeroMemory(&stagingBufferDesc, sizeof(D3D11_BUFFER_DESC));

        stagingBufferDesc.Usage = D3D11_USAGE_STAGING;
        stagingBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        stagingBufferDesc.ByteWidth = sizeof(T) * origIndexCount;

        // create a staging buffer for reading data from the anotherBuffer
        hr = pDevice->CreateBuffer(&stagingBufferDesc, nullptr, &pStagingBuffer);
        CAssert::NotFailed(hr, "can't create a staging buffer");

        // copy the entire contents of the source resource to the destination 
        // resource using the GPU (from the anotherBuffer into the statingBuffer)
        pDeviceContext->CopyResource(pStagingBuffer, pOrigBuffer);

        // map the staging buffer
        hr = pDeviceContext->Map(pStagingBuffer, 0, D3D11_MAP_READ, 0, &mappedSubresource);
        CAssert::NotFailed(hr, "can't map the staging buffer");

        // in the end we unmap the staging buffer and release it
        pDeviceContext->Unmap(pStagingBuffer, 0);
        SafeRelease(&pStagingBuffer);


        /////////////////////  CREATE A DESTINATION INDEX BUFFER  //////////////////////

        // get the description of the anotherBuffer
        pOrigBuffer->GetDesc(&dstBufferDesc);

        // allocate memory for indices of the destination buffer and fill it with data
        indicesArr = new T[origIndexCount]{ 0 };
        CopyMemory(indicesArr, mappedSubresource.pData, sizeof(T) * origIndexCount);

        // create and initialize a buffer with data
        InitializeHelper(pDevice, dstBufferDesc, indicesArr);

        // update the data of this buffer
        indexCount_ = origIndexCount;

        SafeDeleteArr(indicesArr);
    }
    catch (std::bad_alloc& e)
    {
        SafeDeleteArr(indicesArr);
        LogErr(e.what());
        throw EngineException("can't allocate memory for indices of buffer");
    }
    catch (EngineException& e)
    {
        SafeDeleteArr(indicesArr);
        LogErr(e);
        throw EngineException("can't copy an index buffer");
    }
}



// =================================================================================
//                             PRIVATE METHODS
// =================================================================================

template <typename T>
bool IndexBuffer<T>::InitializeHelper(
    ID3D11Device* pDevice,
    const D3D11_BUFFER_DESC& buffDesc,
    const T* pIndices)
{
    // this function helps to initialize an INDEX buffer

    D3D11_SUBRESOURCE_DATA indexBufferData;
    ZeroMemory(&indexBufferData, sizeof(D3D11_SUBRESOURCE_DATA));

    // if we already have some data by the buffer pointer we need first of all to release it
    SafeRelease(&pBuffer_);

    // fill in initial indices data 
    indexBufferData.pSysMem = pIndices;

    // create an index buffer
    const HRESULT hr = pDevice->CreateBuffer(&buffDesc, &indexBufferData, &pBuffer_);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't create an index buffer");
        return false;
    }

    return true;
}

} // namespace Core
