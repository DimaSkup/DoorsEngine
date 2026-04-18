// *********************************************************************************
// Filename:     VertexBuffer.h
// Description:  a functional wrapper for vertex buffers
//
//
// Revising:     12.12.22
// *********************************************************************************
#pragma once

#include <types.h>
#include <log.h>
#include <mem_helpers.h>

#include <Render/d3dclass.h>

#include <d3d11.h>
#include <memory>   // for using std::construct_at
#include <utility>  // for using std::exchange


namespace Core
{

template <typename T>
class VertexBuffer
{
public:
    VertexBuffer()
    {
    }

    // ------------------------------------------

    ~VertexBuffer() 
    {
        Shutdown();
    }

    // ------------------------------------------

    VertexBuffer(const T* vertices, const int numVertices, const bool isDynamic)
    {
        if (!vertices || numVertices <= 0)
        {
            LogErr(LOG, "invalid input data");
            return;
        }
       
        Init(vertices, numVertices, isDynamic);
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
    VertexBuffer(const VertexBuffer& obj)            = delete;
    VertexBuffer& operator=(const VertexBuffer& obj) = delete;

    // ------------------------------------------

    bool Init(const T* vertices, const int numVertices, const bool isDynamic = false);

    // init an empty buffer (is dynamic by default) of size == numVertices 
    bool InitEmpty(const int numVertices);

    // ------------------------------------------

    bool UpdateDynamic(T* vertices, const size count);
    void CopyBuffer(const VertexBuffer& buffer);

    // ------------------------------------------

    inline void Shutdown(void)
    {
        SafeRelease(&pBuffer_);
        stride_ = 0;
        vertexCount_ = 0;
    }

    // ------------------------------------------

    inline UINT        GetStride()          const { return stride_; }
    inline const UINT* GetStrideAddr()      const { return &stride_; }

    inline int         GetVertexCount()     const { return vertexCount_; }
    inline D3D11_USAGE GetUsage()           const { return usageType_; }

    inline ID3D11Buffer* Get()              const { return pBuffer_; };     
    inline ID3D11Buffer* const* GetAddrOf() const { return &(pBuffer_); }
    
private:
    HRESULT InitHelper(const D3D11_BUFFER_DESC& buffDesc, const T* pVertices);

private:
    ID3D11Buffer* pBuffer_ = nullptr;          // a pointer to the vertex buffer
    uint32_t vertexCount_  = 0;                // max num of vertices for this buffer
    uint8_t stride_        = 0;                // size of a single vertex
    uint8_t usageType_     = D3D11_USAGE::D3D11_USAGE_DEFAULT;
};


// =================================================================================
//                          PUBLIC MODIFICATION API
// =================================================================================

//---------------------------------------------------------
// initialize a vertex buffer with input vertices data
//---------------------------------------------------------
template <typename T>
bool VertexBuffer<T>::Init(
    const T* vertices,
    const int numVertices,
    const bool isDynamic)
{
    // check input args
    if (!vertices || numVertices <= 0)
    {
        LogErr(LOG, "invalid input data");
        return false;
    }

        
    D3D11_BUFFER_DESC desc;
    ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

    // accordingly to the isDynamic flag we setup the vertex buffer description
    if (isDynamic)
    {
        desc.Usage          = D3D11_USAGE_DYNAMIC;           // specify how the buffer will be used
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;        // specify how the CPU will access the buffer
    }
    else
    {
        //desc.Usage        = D3D11_USAGE_DEFAULT;           // specify how the buffer will be used
        desc.Usage          = D3D11_USAGE_IMMUTABLE;
        desc.CPUAccessFlags = 0;                             // specify how the CPU will access the buffer
    }
    
    desc.ByteWidth           = sizeof(T) * numVertices;      // the size, in bytes, of the vertex buffer we are going to create
    desc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
    desc.StructureByteStride = 0;                            // the size, in bytes, of a single element stored in the structured buffer. A structure buffer is a buffer that stores elements of equal size
    desc.MiscFlags           = 0;


    // create a buffer using the description and initial vertices data
    HRESULT hr = InitHelper(desc, vertices);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't create VB");
        return false;
    }

    // setup the number of vertices, stride size, the usage type, etc.
    stride_      = sizeof(T);
    vertexCount_ = numVertices;
    usageType_   = desc.Usage;

    return true;
}

//---------------------------------------------------------
// init an empty buffer (which is DYNAMIC by default) of size == numVertices
//---------------------------------------------------------
template <typename T>
bool VertexBuffer<T>::InitEmpty(const int numVertices)
{
    if (numVertices <= 0)
    {
        LogErr(LOG, "number of vertices must be > 0");
        return false;
    }

    D3D11_BUFFER_DESC desc;
    ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

    desc.Usage               = D3D11_USAGE_DYNAMIC;           // specify how the buffer will be used
    desc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;        // specify how the CPU will access the buffer
    desc.ByteWidth           = sizeof(T) * numVertices;       // the size, in bytes, of the vertex buffer we are going to create
    desc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
    desc.StructureByteStride = 0;                             // the size, in bytes, of a single element stored in the structured buffer. A structure buffer is a buffer that stores elements of equal size
    desc.MiscFlags           = 0;


    // if the vertex buffer has already been initialized before
    SafeRelease(&pBuffer_);

    // try to create a vertex buffer
    HRESULT hr = Render::GetD3dDevice()->CreateBuffer(&desc, nullptr, &pBuffer_);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't create VB");
        return false;
    }

    // setup the number of vertices, stride size, the usage type, etc.
    stride_      = sizeof(T);
    vertexCount_ = numVertices;
    usageType_   = desc.Usage;

    return true;
}

//---------------------------------------------------------
// update this DYNAMIC vertex buffer with new vertices
//---------------------------------------------------------
template <typename T>
bool VertexBuffer<T>::UpdateDynamic(T* vertices, const size count)
{
    if (!vertices || count <= 0)
    {
        LogErr(LOG, "invalid input data");
        return false;
    }

    // check input args
    if (usageType_ != D3D11_USAGE_DYNAMIC)
    {
        const char* usageTypeNames[3] = { "default", "immutable", "dynamic" };
        assert(usageType_ < 3);

        LogErr(LOG, "you try to update VB which isn't dynamic"
                    "(your usage type: %s (%d))", usageTypeNames[usageType_], (int)usageType_);
        return false;
    }

    if ((uint32)count > vertexCount_)
    {
        LogErr(LOG, "VB overflow (buf limit: %u, input num vertices: %zu)", vertexCount_, count);
        return false;
    }

    // map the buffer
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ID3D11DeviceContext* pCtx = Render::GetD3dContext();

    const HRESULT hr = pCtx->Map(pBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(hr))
    {
        LogErr(LOG, "failed to map the VB");
        return false;
    }

    // copy new data into the buffer
    CopyMemory(mappedResource.pData, vertices, stride_ * count);

    pCtx->Unmap(pBuffer_, 0);

    return true;
} 

//---------------------------------------------------------
// copy data from the inBuffer into the current one
// and creates a new vertex buffer using this data;
//---------------------------------------------------------
template <typename T>
void VertexBuffer<T>::CopyBuffer(const VertexBuffer& buf)
{
    ID3D11Buffer* pBuffer       = buf.Get();
    const UINT stride           = buf.GetStride();
    const size vertexCount      = buf.GetVertexCount();
    const D3D11_USAGE usageType = buf.GetUsage();

    // check input params
    if (!pBuffer || (vertexCount < 0) || (stride == 0))
    {
        LogErr(LOG, "invalid input buffer");
        return;
    }

    ID3D11Device*            pDevice = Render::GetD3dDevice();
    ID3D11DeviceContext*     pCtx    = Render::GetD3dContext();
    HRESULT                  hr = S_OK;
    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    D3D11_BUFFER_DESC        dstBufferDesc;
    ID3D11Buffer*            pStagingBuffer = nullptr;
    T*                       vertices       = nullptr;   // vertices for a dst buffer             


    // ------------  CREATE A STAGING BUFFER AND COPY DATA INTO IT  ------------

    // setup the staging buffer description
    D3D11_BUFFER_DESC stagingBufferDesc;
    ZeroMemory(&stagingBufferDesc, sizeof(D3D11_BUFFER_DESC));

    stagingBufferDesc.Usage          = D3D11_USAGE_STAGING;
    stagingBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    stagingBufferDesc.ByteWidth      = stride * vertexCount;

    // create a staging buffer for reading data from the anotherBuffer
    hr = pDevice->CreateBuffer(&stagingBufferDesc, nullptr, &pStagingBuffer);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't create a staging buffer");
        return;
    }
        
    // copy the entire contents of the source resource to the destination 
    // resource using the GPU (from the origin buffer into the statingBuffer)
    pCtx->CopyResource(pStagingBuffer, pBuffer);

    // map the staging buffer
    hr = pCtx->Map(pStagingBuffer, 0, D3D11_MAP_READ, 0, &mappedSubresource);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't map the staging buffer");
        pCtx->Unmap(pStagingBuffer, 0);
        SafeRelease(&pStagingBuffer);
    }

    pCtx->Unmap(pStagingBuffer, 0);
    SafeRelease(&pStagingBuffer);


    // ---------------  CREATE A DESTINATION VERTEX BUFFER  ---------------

    // allocate memory for vertices of the dst buffer and fill it with data
    vertices = NEW T[vertexCount];
    if (!vertices)
    {
        LogErr(LOG, "can't alloc mem for vertices in VB");
        return;
    }

    CopyMemory(vertices, mappedSubresource.pData, stride * vertexCount);
    pBuffer->GetDesc(&dstBufferDesc);

    // create and initialize a new buffer with data
    InitHelper(dstBufferDesc, vertices);

    SafeDeleteArr(vertices);
    
    // set params of this vertex buffer (DON'T COPY HERE A PTR TO THE ORIGIN BUFFER)
    stride_      = stride;
    vertexCount_ = vertexCount;
    usageType_   = usageType;
}

//---------------------------------------------------------
// initialize a vertex buffer of ANY type (usage)
//---------------------------------------------------------
template <typename T>
HRESULT VertexBuffer<T>::InitHelper(const D3D11_BUFFER_DESC& desc, const T* vertices)
{
    D3D11_SUBRESOURCE_DATA vbData = {};

    // fill in initial data 
    vbData.pSysMem          = vertices;    // a pointer to a system memory array which contains the data to initialize the vertex buffer
    vbData.SysMemPitch      = 0;           // not used for vertex buffers
    vbData.SysMemSlicePitch = 0;           // not used for vertex buffers

    // if the vertex buffer has already been initialized before
    SafeRelease(&pBuffer_);

    // try to create a vertex buffer
    return Render::GetD3dDevice()->CreateBuffer(&desc, &vbData, &pBuffer_);
}

} // namespace 
