// *********************************************************************************
// Filename:     VertexBuffer.h
// Description:  a functional wrapper for vertex buffers
//
//
// Revising:     12.12.22
// *********************************************************************************
#pragma once

#include <CoreCommon/Types.h>
#include <CoreCommon/log.h>
#include <CoreCommon/Assert.h>
#include <CoreCommon/MemHelpers.h>
//#include "Vertex.h"

#include <d3d11.h>


namespace Core
{

template <typename T>
class VertexBuffer final
{
public:
	VertexBuffer() {}

	~VertexBuffer() 
	{
		SafeRelease(&pBuffer_);
		stride_ = 0;
		vertexCount_ = 0;
	}

	// ------------------------------------------

	VertexBuffer(
		ID3D11Device* pDevice,
		const T* pVertices,
		const int numVertices,
		const bool isDynamic)
	{
		Assert::True((pVertices != nullptr) && (numVertices > 0), "wrong input data");
		Initialize(pDevice, pVertices, numVertices, isDynamic);
	}

	// ------------------------------------------

	VertexBuffer(VertexBuffer&& rhs) noexcept 
		: 
		pBuffer_(std::exchange(rhs.pBuffer_, nullptr)),
		stride_(std::exchange(rhs.stride_, 0)),
		vertexCount_(std::exchange(rhs.vertexCount_, 0)),
		usageType_(rhs.usageType_) {}

	// ------------------------------------------

	VertexBuffer& operator=(VertexBuffer&& rhs) noexcept
	{
		if (this != &rhs)
		{
			this->~VertexBuffer();                    // lifetime of *this ends
			std::construct_at(this, std::move(rhs));
		}
		
		return *this;
	}

	// ------------------------------------------

	// restrict a copying of this class instance 
	// using the asignment operator or copy constructor
	// (for deep copy you have to use the CopyBuffer() method)
	VertexBuffer(const VertexBuffer& obj) = delete;
	VertexBuffer& operator=(const VertexBuffer& obj) = delete;

	// ------------------------------------------

	void Initialize(
		ID3D11Device* pDevice,
		const T* vertices,
		const int numVertices,
		const bool isDynamic = false);

	// ------------------------------------------

	void UpdateDynamic(
		ID3D11DeviceContext* pContext,
		T* vertices,
		const size count);

	// ------------------------------------------

	void CopyBuffer(
		ID3D11Device* pDevice, 
		ID3D11DeviceContext* pContext, 
		const VertexBuffer& buffer);


	// Public query API  
	inline void GetBufferAndStride(ID3D11Buffer* pBuffer, UINT*& pStride)
	{
		pBuffer = pBuffer_;
		pStride = &stride_;
	}

	inline UINT GetStride()                    const { return stride_; }
	inline const UINT* GetAddressOfStride()    const { return &stride_; }

	inline int GetVertexCount()                const { return vertexCount_; }
	inline D3D11_USAGE GetUsage()              const { return usageType_; }

	inline ID3D11Buffer* Get()                 const { return pBuffer_; };     
	inline ID3D11Buffer* const* GetAddressOf() const { return &(pBuffer_); }
	
private:

	void InitializeHelper(
		ID3D11Device* pDevice,
		const D3D11_BUFFER_DESC& buffDesc,
		const T* pVertices);

private:
	ID3D11Buffer* pBuffer_ = nullptr;          // a pointer to the vertex buffer
    uint32_t vertexCount_  = 0;                // max num of vertices for this buffer
    uint8_t stride_        = 0;                // size of a single vertex
	uint8_t usageType_     = D3D11_USAGE::D3D11_USAGE_DEFAULT;
};





// =================================================================================
//                          PUBLIC MODIFICATION API
// =================================================================================

template <typename T>
void VertexBuffer<T>::Initialize(
	ID3D11Device* pDevice,
	const T* pVertices,
	const int numVertices,
	const bool isDynamic)
{
	// initialize a vertex buffer with vertices data

	try
	{
		Assert::True((pVertices != nullptr) && (numVertices > 0), "wrong input data");
		
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

		// accordingly to the isDynamic flag we setup the vertex buffer description
		if (isDynamic)
		{
			desc.Usage = D3D11_USAGE_DYNAMIC;                    // specify how the buffer will be used
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;        // specify how the CPU will access the buffer
		}
		else
		{
			desc.Usage = D3D11_USAGE_IMMUTABLE;
			//desc.Usage = D3D11_USAGE_DEFAULT;                    // specify how the buffer will be used
			desc.CPUAccessFlags = 0;                             // specify how the CPU will access the buffer
		}
	
		desc.ByteWidth = sizeof(T) * numVertices;                // the size, in bytes, of the vertex buffer we are going to create
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.StructureByteStride = 0;                            // the size, in bytes, of a single element stored in the structured buffer. A structure buffer is a buffer that stores elements of equal size
		desc.MiscFlags = 0;

		// create a buffer using the description and initial vertices data
		InitializeHelper(pDevice, desc, pVertices);

		// setup the number of vertices, stride size, the usage type, etc.
		stride_      = sizeof(T);
		vertexCount_ = numVertices;
		usageType_   = desc.Usage;
	}
	catch (EngineException& e)
	{
		SafeRelease(&pBuffer_);
		stride_ = 0;
		vertexCount_ = 0;
		Log::Error(e);
		throw EngineException("can't create a vertex buffer");
	}
}

///////////////////////////////////////////////////////////

template <typename T>
void VertexBuffer<T>::UpdateDynamic(
	ID3D11DeviceContext* pContext,
	T* vertices,
	const size count)
{
	// update this DYNAMIC vertex buffer with new vertices

	try
	{
		Assert::True(usageType_ == D3D11_USAGE_DYNAMIC, "not dynamic usage of the buffer");
		Assert::True(vertices && ((u32)count <= vertexCount_), "wrong input data");

		// map the buffer
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		const HRESULT hr = pContext->Map(pBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		Assert::NotFailed(hr, "failed to map the vertex buffer");

		// copy new data into the buffer
		CopyMemory(mappedResource.pData, vertices, stride_ * count);

		pContext->Unmap(pBuffer_, 0);
	}
	catch (EngineException & e)
	{
		Log::Error(e);
		throw EngineException("can't update the dynamic vertex buffer");
	}
} 

///////////////////////////////////////////////////////////

template <typename T>
void VertexBuffer<T>::CopyBuffer(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext,
	const VertexBuffer& inBuffer)
{
	// this function copies data from the inBuffer into the current one
	// and creates a new vertex buffer using this data;

	Assert::NotNullptr(pDevice, "a ptr to the device == nullptr");
	Assert::NotNullptr(pContext, "a ptr to the device context == nullptr");

	ID3D11Buffer* pBuffer       = inBuffer.Get();
	const UINT stride           = inBuffer.GetStride();
	const size vertexCount      = inBuffer.GetVertexCount();
	const D3D11_USAGE usageType = inBuffer.GetUsage();

	// check input params
	Assert::NotNullptr(pBuffer, "ptr to buffer == nullptr");
	Assert::True((vertexCount > 0) && (stride), "wrong input buffer's data");


	HRESULT hr = S_OK;
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	D3D11_BUFFER_DESC dstBufferDesc;
	ID3D11Buffer* pStagingBuffer = nullptr;
	T* vertices = nullptr;                       // vertices for a dst buffer             

	try
	{
		// ------------  CREATE A STAGING BUFFER AND COPY DATA INTO IT  ------------

		// setup the staging buffer description
		D3D11_BUFFER_DESC stagingBufferDesc;
		ZeroMemory(&stagingBufferDesc, sizeof(D3D11_BUFFER_DESC));

		stagingBufferDesc.Usage          = D3D11_USAGE_STAGING;
		stagingBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		stagingBufferDesc.ByteWidth      = stride * vertexCount;

		// create a staging buffer for reading data from the anotherBuffer
		hr = pDevice->CreateBuffer(&stagingBufferDesc, nullptr, &pStagingBuffer);
		Assert::NotFailed(hr, "can't create a staging buffer");
		
		// copy the entire contents of the source resource to the destination 
		// resource using the GPU (from the origin buffer into the statingBuffer)
		pContext->CopyResource(pStagingBuffer, inBuffer.Get());

		// map the staging buffer
		hr = pContext->Map(pStagingBuffer, 0, D3D11_MAP_READ, 0, &mappedSubresource);
		Assert::NotFailed(hr, "can't map the staging buffer");

		pContext->Unmap(pStagingBuffer, 0);
		SafeRelease(&pStagingBuffer);


		// ---------------  CREATE A DESTINATION Vertex BUFFER  ---------------

		// allocate memory for vertices of the dst buffer and fill it with data
		vertices = new T[vertexCount];
		CopyMemory(vertices, mappedSubresource.pData, stride * vertexCount);

		pBuffer->GetDesc(&dstBufferDesc);

		// create and initialize a new buffer with data
		InitializeHelper(pDevice, dstBufferDesc, vertices);

		SafeDeleteArr(vertices);
	
		// set params of this vertex buffer (DON'T COPY HERE A PTR TO THE ORIGIN BUFFER)
		stride_      = stride;
		vertexCount_ = vertexCount;
		usageType_   = usageType;
	}
	catch (std::bad_alloc & e)
	{
		pContext->Unmap(pStagingBuffer, 0U);
		SafeDeleteArr(vertices);
		Log::Error(e.what());
		throw EngineException("can't allocate memory for vertices of buffer");
	}
	catch (EngineException & e)
	{
		pContext->Unmap(pStagingBuffer, 0U);
		SafeDeleteArr(vertices);
		Log::Error(e);
		throw EngineException("can't copy a vertex buffer");
	}
}


// =================================================================================
//                           PRIVATE FUNCTIONS (HELPERS)
// =================================================================================

template <typename T>
void VertexBuffer<T>::InitializeHelper(
	ID3D11Device* pDevice,
	const D3D11_BUFFER_DESC & buffDesc,
	const T* vertices)
{
	// this function helps to initialize a vertex buffer of ANY type (usage)

	D3D11_SUBRESOURCE_DATA vertexBufferData = {};

	// fill in initial data 
	vertexBufferData.pSysMem          = vertices;    // a pointer to a system memory array which contains the data to initialize the vertex buffer
	vertexBufferData.SysMemPitch      = 0;           // not used for vertex buffers
	vertexBufferData.SysMemSlicePitch = 0;           // not used for vertex buffers

	// if the vertex buffer has already been initialized before
	SafeRelease(&pBuffer_);

	// try to create a vertex buffer
	const HRESULT hr = pDevice->CreateBuffer(&buffDesc, &vertexBufferData, &pBuffer_);
	Assert::NotFailed(hr, "can't create a vertex buffer");
}


} // namespace Core
