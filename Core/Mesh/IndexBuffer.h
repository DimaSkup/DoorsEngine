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
		Initialize(pDevice, indices, numIndices);
	}

	// ---------------------------------------------

	inline ~IndexBuffer()
	{
        Shutdown();
	}

	// ---------------------------------------------

	inline IndexBuffer(IndexBuffer&& rhs) noexcept :
		pBuffer_(std::exchange(rhs.pBuffer_, nullptr)),
		indexCount_(std::exchange(rhs.indexCount_, 0)) 
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
		const T* pIndices,
		const int numIndices);

	void CopyBuffer(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pDeviceContext,
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
	void InitializeHelper(
		ID3D11Device* pDevice,
		const D3D11_BUFFER_DESC& buffDesc,
		const T* pIndices);

private:
	ID3D11Buffer* pBuffer_ = nullptr;
	UINT          indexCount_ = 0;
};



// =================================================================================
//                              PUBLIC METHODS
// =================================================================================

template <typename T>
bool IndexBuffer<T>::Initialize(
	ID3D11Device* pDevice,
	const T* pIndices,
	const int numIndices)
{
	// initialize this index buffer with indices data

    try
    {
        CAssert::True((pIndices != nullptr) && (numIndices > 0), "wrong input data");

        // initialize the number of indices
        indexCount_ = (UINT)numIndices;

        // setup the index buffer description
        D3D11_BUFFER_DESC desc;
        desc.ByteWidth           = sizeof(T) * indexCount_;
        desc.Usage               = D3D11_USAGE_IMMUTABLE;   // D3D11_USAGE_DEFAULT;
        desc.BindFlags           = D3D11_BIND_INDEX_BUFFER;
        desc.CPUAccessFlags      = 0;
        desc.MiscFlags           = 0;
        desc.StructureByteStride = 0;

        // create and initialize a buffer with data
        InitializeHelper(pDevice, desc, pIndices);

        return true;
    }
    catch (EngineException& e)
    {
        LogErr(e);
        Shutdown();
        return false;
    }
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

	HRESULT hr = S_OK;
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	D3D11_BUFFER_DESC dstBufferDesc;
	ID3D11Buffer* pStagingBuffer = nullptr;
	T* indicesArr = nullptr;                 // will be filled with indices for a destination buffer

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
void IndexBuffer<T>::InitializeHelper(
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
	CAssert::NotFailed(hr, "can't create an index buffer");
}

} // namespace Core
