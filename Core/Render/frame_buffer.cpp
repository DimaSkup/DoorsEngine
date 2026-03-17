// ====================================================================================
// Filename:       FrameBuffer.cpp
// Created:        20.09.23  by DimaSkup
// ====================================================================================
#include <CoreCommon/pch.h>
#include "frame_buffer.h"
#include <Render/CRender.h>

namespace Core
{

FrameBuffer::FrameBuffer()
{
}

FrameBuffer::~FrameBuffer()
{
    Shutdown();
}

///////////////////////////////////////////////////////////

FrameBuffer::FrameBuffer(FrameBuffer&& rhs) noexcept
    :
    pRenderTargetTexture_(std::exchange(rhs.pRenderTargetTexture_, nullptr)),
    pRenderTargetView_   (std::exchange(rhs.pRenderTargetView_, nullptr)),
    pShaderResourceView_ (std::exchange(rhs.pShaderResourceView_, nullptr)),
    pDepthStencilBuffer_ (std::exchange(rhs.pDepthStencilBuffer_, nullptr)),
    pDepthStencilView_   (std::exchange(rhs.pDepthStencilView_, nullptr)),
    viewport_            { rhs.viewport_ },
    projection_          { rhs.projection_ },
    orthoMatrix_         { rhs.orthoMatrix_ }
{
    // move-constructor
}

///////////////////////////////////////////////////////////

FrameBuffer& FrameBuffer::operator=(FrameBuffer&& rhs) noexcept
{
    // move-assignment
    if (this != &rhs)
    {
        Shutdown();                          // lifetime of *this ends
        std::construct_at(this, std::move(rhs));
    }

    return *this;
}

///////////////////////////////////////////////////////////

void FrameBuffer::Shutdown()
{
    SafeRelease(&pDepthStencilView_);
    SafeRelease(&pDepthStencilBuffer_);
    SafeRelease(&pShaderResourceView_);
    SafeRelease(&pRenderTargetView_);
    SafeRelease(&pRenderTargetTexture_);
    isInit_ = false;
}


// ====================================================================================
//                               public methods
// ====================================================================================

bool FrameBuffer::Init(const FrameBufSpec& spec)
{
    // 1. this function will do the setup of the render texture object. The function creates
    //    a render target texture by first setting up the description of the texture and
    //    then creating that texture. 
    // 2. It then uses that texture to setup a render target view
    //    so that the texture can be drawn to as a render target. 
    // 3. The third thing we do is create a shader resource view of that texture so that 
    //    we can supply the rendered data to calling objects
    // 4. This function will also create a projection and ortho matrix for correct 
    //    perspective rendering of the render texture, since the dimensions of the render
    //    texture may vary. Remember to always keep the aspect ratio of this render
    //    to texture the same as the aspect ratio of where the texture will be used, 
    //    or there will be some size distortion.

    try
    {
        CAssert::True((spec.width > 0) && (spec.height > 0), "wrong texture dimensions");
        CAssert::True((spec.screenNear > 0.0f) && (spec.screenNear < spec.screenDepth), "wrong screen near or depth values");

        // store the basic params
        specification_ = spec;

        // prevent memory leakage
        Shutdown();

        CreateRenderTargetTexture();
        CreateRenderTargetView();
        CreateShaderResourceView();
        CreateDepthStencilBuffer();
        CreateDepthStencilView();
        SetupViewportAndMatrices();

        isInit_ = true;
    }
    catch (EngineException & e)
    {
        Shutdown();
        LogErr(LOG, e.what());
        LogErr(LOG, "can't init a frame buffer");
        return false;
    }

    return true;
}

//---------------------------------------------------------
// handle window resizing
//---------------------------------------------------------
void FrameBuffer::ResizeBuffers(const int w, const int h)
{
    try
    {
        specification_.width = w;
        specification_.height = h;

        ID3D11DeviceContext* pCtx = Render::GetD3dContext();

        // 1. Clear render targets from device context
        //    crear the previous window size specific context
        ID3D11RenderTargetView* nullViews[] = { nullptr };
        pCtx->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);

        // 2. Release rendering target
        SafeRelease(&pDepthStencilBuffer_);
        SafeRelease(&pDepthStencilView_);
        SafeRelease(&pShaderResourceView_);
        SafeRelease(&pRenderTargetTexture_);
        SafeRelease(&pRenderTargetView_);
        pCtx->Flush();

        // 4. recreate the render target view, depth stencil buffer/view, and viewport
        CreateRenderTargetTexture();
        CreateRenderTargetView();
        CreateShaderResourceView();
        CreateDepthStencilBuffer();
        CreateDepthStencilView();
        SetupViewportAndMatrices();
    }
    catch (EngineException& e)
    {
        LogErr(LOG, e.what());
        Shutdown();
        throw EngineException("can't resize buffers for the framebuffer");
    }
}

//---------------------------------------------------------
// this func changes where we are currently rendering to. We are usually rendering
// to the back buffer (or another render texture), but when we call this function,
// we will now be rendering to this render texture.
// Note that we also need to set the viewport for this render texture, since
// the dimensions might be different that the back buffer or wherever we were
// rendering to before this function was called
//---------------------------------------------------------
void FrameBuffer::Bind()
{
    ID3D11DeviceContext* pCtx = Render::GetD3dContext();

    // bind the render target view and depth stencil buffer to the output render pipeline
    pCtx->OMSetRenderTargets(1, &pRenderTargetView_, pDepthStencilView_);

    // set the viewport
    pCtx->RSSetViewports(1, &viewport_);
}

//---------------------------------------------------------
// Desc:   clear the prev content of the frame buffer before rendering
//---------------------------------------------------------
void FrameBuffer::ClearBuffers(float r, float g, float b, float a)
{
    ID3D11DeviceContext* pCtx = Render::GetD3dContext();

    float color[4]{ r, g, b, a };

    // clear the back buffer
    if (pRenderTargetView_)
        pCtx->ClearRenderTargetView(pRenderTargetView_, color);

    // clear the depth buffer
    if (pDepthStencilView_)
        pCtx->ClearDepthStencilView(pDepthStencilView_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}


// ====================================================================================
//                               private methods
// ====================================================================================

//---------------------------------------------------------
// Desc:   create a render target texture buffer
//---------------------------------------------------------
void FrameBuffer::CreateRenderTargetTexture()
{
    HRESULT hr = S_OK;
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));

    // setup the render target texture description
    desc.Width              = specification_.width;
    desc.Height             = specification_.height;
    desc.MipLevels          = 1;
    desc.ArraySize          = 1;
    desc.Format             = specification_.format;
    desc.SampleDesc.Count   = 1;
    desc.Usage              = D3D11_USAGE_DEFAULT;
    desc.BindFlags          = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags     = 0;
    desc.MiscFlags          = 0;

    // create the render target texture
    hr = Render::GetD3dDevice()->CreateTexture2D(&desc, nullptr, &pRenderTargetTexture_);
    CAssert::NotFailed(hr, "can't create the render target texture");
}

//---------------------------------------------------------

void FrameBuffer::CreateRenderTargetView()
{
    HRESULT hr = S_OK;
    D3D11_RENDER_TARGET_VIEW_DESC desc;
    ZeroMemory(&desc, sizeof(desc));

    // setup the description of the render target view
    desc.Format             = specification_.format;
    desc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
    desc.Texture2D.MipSlice = 0;

    // create the render target view
    hr = Render::GetD3dDevice()->CreateRenderTargetView(pRenderTargetTexture_, &desc, &pRenderTargetView_);
    CAssert::NotFailed(hr, "can't create the render target view");
}

//---------------------------------------------------------
// create a shader resource view related to the render target
//---------------------------------------------------------
void FrameBuffer::CreateShaderResourceView()
{
    HRESULT hr = S_OK;
    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
    ZeroMemory(&desc, sizeof(desc));

    // setup the description of the shader resource view
    desc.Format                    = specification_.format;
    desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
    desc.Texture2D.MostDetailedMip = 0;
    desc.Texture2D.MipLevels       = 1;

    // create the shader resource view
    hr = Render::GetD3dDevice()->CreateShaderResourceView(pRenderTargetTexture_, &desc, &pShaderResourceView_);
    CAssert::NotFailed(hr, "can't create the shader resource view");
}

//---------------------------------------------------------

void FrameBuffer::CreateDepthStencilBuffer()
{
    HRESULT hr = S_OK;
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));

    // set up the description of the depth buffer
    desc.Width              = specification_.width;
    desc.Height             = specification_.height;
    desc.MipLevels          = 1;
    desc.ArraySize          = 1;
    desc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage              = D3D11_USAGE_DEFAULT;
    desc.BindFlags          = D3D11_BIND_DEPTH_STENCIL;
    desc.CPUAccessFlags     = 0;
    desc.MiscFlags          = 0;

    // create the texture for the depth buffer using the filled out description
    hr = Render::GetD3dDevice()->CreateTexture2D(&desc, nullptr, &pDepthStencilBuffer_);
    CAssert::NotFailed(hr, "can't create the texture for the depth buffer");
}

//---------------------------------------------------------

void FrameBuffer::CreateDepthStencilView()
{
    HRESULT hr = S_OK;
    D3D11_DEPTH_STENCIL_VIEW_DESC desc;
    ZeroMemory(&desc, sizeof(desc));

    // set up the depth stencil view description
    desc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
    desc.ViewDimension      = D3D11_DSV_DIMENSION_TEXTURE2D;
    desc.Texture2D.MipSlice = 0;

    // create the depth stencil view
    hr = Render::GetD3dDevice()->CreateDepthStencilView(pDepthStencilBuffer_, nullptr, &pDepthStencilView_);
    CAssert::NotFailed(hr, "can't create the depth stencil view");
}

//---------------------------------------------------------

void FrameBuffer::SetupViewportAndMatrices()
{
    const float w  = (float)specification_.width;
    const float h  = (float)specification_.height;
    const float zn = specification_.screenNear;
    const float zf = specification_.screenDepth;

    // Setup the viewport for rendering
    viewport_.TopLeftX = 0;
    viewport_.TopLeftY = 0;
    viewport_.Width    = w;
    viewport_.Height   = h;
    viewport_.MinDepth = 0;
    viewport_.MaxDepth = 1;

    // setup the projection matrix
    projection_ = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, w/h, zn, zf);

    // create an orthographic projection matrix for 2D rendering
    orthoMatrix_ = DirectX::XMMatrixOrthographicLH(w, h, zn, zf);
}

} // namespace
