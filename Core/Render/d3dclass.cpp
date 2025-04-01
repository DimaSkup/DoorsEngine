// ================================================================================
// Filename: d3dclass.cpp
// Revising: 01.01.23
// ================================================================================
#include "d3dclass.h"

#include <CoreCommon/MemHelpers.h>
#include <CoreCommon/Assert.h>
#include <CoreCommon/Log.h>
#include <CoreCommon/StringHelper.h>


// encourage the driver to select the discrete video adapter by default
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}


namespace Core
{

D3DClass* D3DClass::pInstance_ = nullptr;


D3DClass::D3DClass()
{
    if (D3DClass::pInstance_ == nullptr)
    {
        D3DClass::pInstance_ = this;
    }
    else
    {
        throw EngineException("you can't create more than only one instance of this class");
    }

    Log::Debug();
}

D3DClass::~D3DClass()
{
}



// ================================================================================
//                            public methods
// ================================================================================

bool D3DClass::Initialize(
    HWND hwnd, 
    const bool vsyncEnabled,
    const bool fullScreen,
    const bool enable4xMSAA,
    const float screenNear, 
    const float screenDepth)
{
    try
    {
        Log::Debug();

        // check if we have any available IDXGI adapter
        Assert::True(adaptersReader_.GetNumAdapters() > 1, "can't find any IDXGI adapter");

        // set adapter idx (if there is any discrete graphics adapter we use this discrete adapter as primary)
        displayAdapterIndex_ = 0;

        // get default window size of the full screen mode
        fullScreenWndWidth_ = GetSystemMetrics(SM_CXSCREEN);
        fullScreenWndHeight_ = GetSystemMetrics(SM_CYSCREEN);

        // compute the window dimensions for the current mode (windowed/fullscreen)
        if (fullScreen)
        {
            wndWidth_  = fullScreenWndWidth_;
            wndHeight_ = fullScreenWndHeight_;
        }
        // windowed mode
        else
        {
            RECT clientRect;

            //Retrieves the client area of the window, which tells us its renderable width and height
            bool result = GetClientRect(hwnd, &clientRect);
            Assert::True(result, "can't get the client rectangle params");
        
            wndWidth_ = clientRect.right - clientRect.left;
            wndHeight_ = clientRect.bottom - clientRect.top;

            windowedModeWidth_ = wndWidth_;
            windowedModeHeight_ = wndHeight_;
        }


        // check params
        assert((wndWidth_ > 0) && (wndHeight_ > 0));
        assert((screenNear >= 0.0f) && (screenDepth > screenNear));

        vsyncEnabled_ = vsyncEnabled;        // define if VSYNC is enabled or not
        fullScreen_   = fullScreen;          // define if window is full screen or not
        enable4xMsaa_ =  enable4xMSAA;        // use 4X MSAA?
        screenNear_   = screenNear;
        screenDepth_  = screenDepth;


        // initialize all the main parts of DirectX
        InitializeDirectX(hwnd, wndWidth_, wndHeight_, screenNear, screenDepth);

        // init blending states, rasterizer states, depth stencil states in a separate way
        renderStates_.InitAll(pDevice_, enable4xMSAA);
        InitializeDepthStencil(wndWidth_, wndHeight_);

        Log::Print("is initialized successfully");
    }
    catch (EngineException& e)
    {
        Log::Error(e, true);
        Shutdown();
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////

void D3DClass::Shutdown()
{
    // reset the screen state and release the allocated memory

    // set a windowed mode as active
    if (pSwapChain_)
        pSwapChain_->SetFullscreenState(FALSE, nullptr);


    // release all the depth stencil stuff
    SafeRelease(&pDepthStencilView_);
    SafeRelease(&pDepthStencilBuffer_);
    SafeRelease(&pRenderTargetView_);
    SafeRelease(&pBackBuffer_);
    SafeRelease(&pContext_);
    SafeRelease(&pDevice_);
    SafeRelease(&pSwapChain_);
}

///////////////////////////////////////////////////////////

void* D3DClass::operator new(size_t i)
{
    // memory allocation

    if (void* ptr = _aligned_malloc(i, 16))
    {
        return ptr;
    }
    
    Log::Error("can't allocate memory for the D3DClass object");
    throw std::bad_alloc{};
}

void D3DClass::operator delete(void* p)
{
    _aligned_free(p);
}

///////////////////////////////////////////////////////////

void D3DClass::BeginScene()
{
    // before rendering of each frame we need to reset buffers

    const FLOAT bgColor[4]{ 0, 1, 1, 0 };
    
    // clear the render target view with particular color
    pContext_->ClearRenderTargetView(pRenderTargetView_, bgColor);

    // clear the depth stencil view with 1.0f values
    pContext_->ClearDepthStencilView(pDepthStencilView_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

///////////////////////////////////////////////////////////

void D3DClass::EndScene()
{
    // after all the rendering into the back buffer 
    // we need to present it on the screen

    // if vertical synchronization is enabled the first param will be set to 1
    // or in another case it will be set to 0 (no vsync)
    pSwapChain_->Present((UINT)vsyncEnabled_, 0);
}

///////////////////////////////////////////////////////////

void D3DClass::GetDeviceAndDeviceContext(
    ID3D11Device*& pDevice,
    ID3D11DeviceContext*& pContext)
{
    pDevice  = pDevice_;
    pContext = pContext_;
}

///////////////////////////////////////////////////////////

void D3DClass::GetVideoCardInfo(
    char* cardName, 
    const int maxCardNameSize,
    int& memorySize)
{
    // get data about the video card

    // get adapter's data
    AdapterData* data = adaptersReader_.GetAdapterDataByIdx(displayAdapterIndex_);
    DXGI_ADAPTER_DESC adapterDesc = data->description_;

    // store the dedicated video card memory in megabytes and store its name
    const UINT bytesInMegabyte = 1024 * 1024;

    std::string name = StringHelper::ToString(adapterDesc.Description);
    memcpy(cardName, name.data(), name.length());
    memorySize = static_cast<int>(adapterDesc.DedicatedVideoMemory / bytesInMegabyte);
    
    Log::Debug("Video card name: " + name);
    Log::Debug("Video memory :   " + std::to_string(memorySize) + " MB");
}

///////////////////////////////////////////////////////////

void D3DClass::TurnOnRSfor2Drendering()
{
    // we call this function to set up a raster state 
    // for proper rendering of 2D elements / UI;
    // NOTE: we store a hash of the previous RS so later we can set it back

    prevRasterStateHash_ = renderStates_.GetCurrentRSHash();
    renderStates_.SetRS(pContext_, { FILL_SOLID, CULL_BACK });
}



// ================================================================================= 
//                             PRIVATE METHODS
// =================================================================================
void D3DClass::InitializeDirectX(
    HWND hwnd,
    const UINT wndWidth,
    const UINT wndHeight,
    const float nearZ,
    const float farZ)
{
    try
    {
        Log::Debug();

        Assert::True((wndWidth & wndHeight), "wrong window dimensions");
        Assert::True(nearZ > 0, "near wnd plane must be > 0");
        Assert::True(farZ > nearZ, "far wnd plane must be > near plane");

        // create the Direct3D 11 device and context
        InitializeDevice();

        // initialize all the main parts of DirectX
        InitializeSwapChain(hwnd, wndWidth, wndHeight);
        InitializeRenderTargetView();
        InitializeViewport(wndWidth, wndHeight);
        InitializeMatrices(wndWidth, wndHeight, nearZ, farZ);
    }
    catch (EngineException & e)
    {
        Log::Error(e, true);
        throw EngineException("can't initialize DirectX stuff");
    }
}

///////////////////////////////////////////////////////////

void D3DClass::InitializeDevice()
{
    // THIS FUNCTION creates the Direct3D 11 device and context;
    // also it check the quality level support for 4X MSAA.

    D3D_FEATURE_LEVEL featureLevel;
    UINT createDeviceFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    IDXGIAdapter* pAdapter = adaptersReader_.GetDXGIAdapterByIdx(displayAdapterIndex_);

    HRESULT hr = D3D11CreateDevice(
        pAdapter,
        D3D_DRIVER_TYPE_UNKNOWN,
        0,                                          // no software device
        createDeviceFlags,
        0, 0,                                       // default feature level array
        D3D11_SDK_VERSION,
        &pDevice_,
        &featureLevel,
        &pContext_);

    Assert::NotFailed(hr, "D3D11CreateDevice failed");
    Assert::True(featureLevel == D3D_FEATURE_LEVEL_11_0, "Direct3D Feature Level 11 unsupported");

    // now that we have a created device, we can check the quality level support for 4X MSAA.
    hr = pDevice_->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4xMsaaQuality_);
    Assert::NotFailed(hr, "the quality level number must be > 0");
}

///////////////////////////////////////////////////////////

bool D3DClass::ResizeSwapChain(HWND hwnd, SIZE newSize)
{
    // handle window resizing

    try
    {
        // maybe we got an WM_SIZE event after CreateWindowEx and currently our
        // swap chain isn't created yet so just go out
        if (pSwapChain_ == nullptr)
            return true;

        HRESULT hr = S_OK;
        ID3D11DeviceContext* pContext = GetDeviceContext();

        wndWidth_ = newSize.cx;
        wndHeight_ = newSize.cy;

        //printf("wnd x: %d\n", wndWidth_);
        //printf("wnd y: %d\n", wndHeight_);


        // 1. Clear render targets from device context
        //    crear the previous window size specific context
        ID3D11RenderTargetView* nullViews[] = { nullptr };
        pContext->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);

        // 2. Release rendering target
        SafeRelease(&pDepthStencilBuffer_);
        SafeRelease(&pDepthStencilView_);
        SafeRelease(&pRenderTargetView_);
        pContext->Flush();

        // 3. Resize buffer:
        //    Preserve the existing buffer count and format.
        //    Automatically choose the width and height to match the client rect for HWNDs.
        hr = pSwapChain_->ResizeBuffers(0, 0, 0,
            DXGI_FORMAT_UNKNOWN,                  
            DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
        Assert::NotFailed(hr, "can't resize swap chain buffers");

        // 4. recreate the render target view, depth stencil buffer/view, and viewport
        InitializeRenderTargetView();
        InitializeDepthStencil(wndWidth_, wndHeight_);
        InitializeViewport(wndWidth_, wndHeight_);
    }
    catch (EngineException& e)
    {
        Log::Error(e, true);
        Shutdown();
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////

void D3DClass::InitializeSwapChain(HWND hwnd, const int width, const int height)
{
    HRESULT hr = S_OK;
    DXGI_SWAP_CHAIN_DESC sd { 0 };

    // Setup the swap chain description
    sd.BufferDesc.Width            = width;                                 // desired back buffer width
    sd.BufferDesc.Height           = height;                                // desired back buffer height
    sd.BufferDesc.Format           = backBufferFormat_;                     // use a simple 32-bit surface 
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;// DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	// a rasterizer method to render an image on a surface
    sd.BufferDesc.Scaling          = DXGI_MODE_SCALING_UNSPECIFIED;         // how to scale an image to fit it to the screen resolution

    sd.BufferCount                 = 1;                                     // we have only one back buffer
    sd.BufferUsage                 = DXGI_USAGE_RENDER_TARGET_OUTPUT;       // use the back buffer as the render target output
    sd.OutputWindow                = hwnd;                                  // set the current window
    sd.Windowed                    = !fullScreen_;                          // specity true to run in windowed mode or false for full-screen mode
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;              // discard the content of the back buffer after presenting
    sd.Flags                       = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;


    sd.BufferDesc.RefreshRate.Numerator = (vsyncEnabled_) ? 60 : 0;
    sd.BufferDesc.RefreshRate.Denominator = 1;

    // Use 4X MSAA?
    // Note: m4xMsaaQuality_ is returned via CheckMultisampleQualitylevels()
    sd.SampleDesc.Count   = (enable4xMsaa_) ? 4 : 1;
    sd.SampleDesc.Quality = (enable4xMsaa_) ? m4xMsaaQuality_ - 1 : 0;

    // for creation of the swap chain we have to use the IDXGIFactory instance that was
    // used to create the device
    IDXGIDevice* pDxgiDevice = nullptr;
    hr = pDevice_->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDxgiDevice);
    Assert::NotFailed(hr, "can't get the interface of DXGI Device");

    IDXGIAdapter* pDxgiAdapter = nullptr;
    hr = pDxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&pDxgiAdapter);
    Assert::NotFailed(hr, "can't get the interface of DXGI Adapter");

    // finally go the IDXGIFactory interface
    IDXGIFactory* pDxgiFactory = nullptr;
    hr = pDxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pDxgiFactory);
    Assert::NotFailed(hr, "can't get the interface of DXGI Factory");
        

    // Create the swap chain
    pDxgiFactory->CreateSwapChain(pDevice_, &sd, &pSwapChain_);
    Assert::NotFailed(hr, "can't create the swap chain");
    Assert::NotNullptr(pSwapChain_, "something went wrong during creation of the swap chain because pSwapChain == NULLPTR");

    // release our acquired COM interfaces (because we are done with them)
    SafeRelease(&pDxgiDevice);
    SafeRelease(&pDxgiAdapter);
    SafeRelease(&pDxgiFactory);

}

///////////////////////////////////////////////////////////

void D3DClass::InitializeRenderTargetView()
{
    // create and set up the render target view to the back buffer;
    try
    {
        HRESULT hr = S_OK;
        
        // obtain a ptr to the swap chain's back buffer which we will use as a render target
        ID3D11Texture2D* pBackBuffer = nullptr;
        hr = pSwapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (VOID**)&pBackBuffer);
        Assert::NotFailed(hr, "can't get a buffer from the swap chain");

        // create a render target view 
        if (pBackBuffer)
        {
            hr = pDevice_->CreateRenderTargetView(pBackBuffer, nullptr, &pRenderTargetView_);
            Assert::NotFailed(hr, "can't create a render target view");
            SafeRelease(&pBackBuffer);
        }
    }
    catch (EngineException & e)
    {
        Log::Error(e, true);
        throw EngineException("can't initialize the render target view");
    }
}

///////////////////////////////////////////////////////////

void D3DClass::InitializeDepthStencil(const UINT width, const UINT height)
{
    // creates the depth stencil buffer, depth stencil view
    // and bind them to the current device context

    try
    {
        InitializeDepthStencilTextureBuffer(width, height);
        InitializeDepthStencilView();

        // Set the depth stencil state.
        pContext_->OMSetDepthStencilState(renderStates_.GetDSS(DEPTH_ENABLED), 1);

        // bind together the render target view and the depth stencil view to the output merger stage
        pContext_->OMSetRenderTargets(1, &pRenderTargetView_, pDepthStencilView_);
    }
    catch (EngineException & e)
    {
        Log::Error(e, true);
        throw EngineException("can't initialize some of the depth/stencil elements");
    }
}

///////////////////////////////////////////////////////////

void D3DClass::InitializeDepthStencilTextureBuffer(const UINT width, const UINT height)
{
    // create a depth/stencil buffer texture

    D3D11_TEXTURE2D_DESC desc;

    // describe our Depth/Stencil Buffer
    desc.Width          = width;
    desc.Height         = height;
    desc.Format         = DXGI_FORMAT_D24_UNORM_S8_UINT;	// 24 bits for the depth and 8 bits for the stencil
    desc.MipLevels      = 1;
    desc.ArraySize      = 1;

    // Use 4X MSAA?
    // Note: m4xMsaaQuality_ is returned via CheckMultisampleQualitylevels()
    desc.SampleDesc.Count   = (enable4xMsaa_) ? 4 : 1;
    desc.SampleDesc.Quality = (enable4xMsaa_) ? m4xMsaaQuality_ - 1 : 0;
    
    desc.Usage          = D3D11_USAGE_DEFAULT;
    desc.BindFlags      = D3D11_BIND_DEPTH_STENCIL;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags      = 0;

    // Create the depth/stencil buffer
    HRESULT hr = pDevice_->CreateTexture2D(&desc, nullptr, &pDepthStencilBuffer_);
    Assert::NotFailed(hr, "can't create the depth stencil buffer");
} 

///////////////////////////////////////////////////////////

void D3DClass::InitializeDepthStencilView()
{
#if 0
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));

    // Setup the depth stencil view description
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;
#endif

    // Create a depth stencil view
    const HRESULT hr = pDevice_->CreateDepthStencilView(
        pDepthStencilBuffer_,
        nullptr,                  // &depthStencilViewDesc, -- because we specified the type of our depth/stencil buffer, we specify null for this parameter
        &pDepthStencilView_);

    Assert::NotFailed(hr, "can't create a depth stencil view");
}

///////////////////////////////////////////////////////////

void D3DClass::InitializeViewport(const UINT width, const UINT height)
{
    // Setup the viewport
    SetupViewportParams((float)width, (float)height, 1, 0, 0, 0);
    
    // bind the viewport
    pContext_->RSSetViewports(1, &viewport_);
}

///////////////////////////////////////////////////////////

void D3DClass::SetupViewportParams(
    const float width,
    const float height,
    const float maxDepth,
    const float minDepth,
    const float topLeftX,
    const float topLeftY)
{
    // Setup the viewport with specific values
    viewport_ = { topLeftX, topLeftY, width, height, minDepth, maxDepth };

    // bind the viewport
    pContext_->RSSetViewports(1, &viewport_);
}

///////////////////////////////////////////////////////////

void D3DClass::InitializeMatrices(
    const UINT width, 
    const UINT height,
    const float nearZ, 
    const float farZ)
{
    // THIS FUNCTION initializes world and ortho matrices to it's default values
 
    worldMatrix_ = DirectX::XMMatrixIdentity();

    // Initialize the orthographic matrix for 2D rendering
    orthoMatrix_ = DirectX::XMMatrixOrthographicLH(
        (float)width,
        (float)height,
        nearZ,
        farZ);
}

///////////////////////////////////////////////////////////

bool D3DClass::ToggleFullscreen(HWND hwnd, bool isFullscreen)
{
    try
    {
        Log::Debug("ToggleFullscreen(): " + (isFullscreen) ? "true" : "false");

        HRESULT hr = S_OK;

        fullScreen_ = isFullscreen;

        // turn on the fullscreen mode
        if (isFullscreen)
        {
            // store the current window size in windowed mode 
            // so we can restore it when go back from fullscreen to windowed mode
            windowedModeWidth_ = wndWidth_;
            windowedModeHeight_ = wndHeight_;

            //printf("windowed x: %d\n", windowedModeWidth_);
            //printf("windowed y: %d\n", windowedModeHeight_);

            //RECT desctopArea;
            DXGI_MODE_DESC mode;
            ZeroMemory(&mode, sizeof(DXGI_MODE_DESC));

            // currently we get the maximum monitor dimensions for the fullscreen
            mode.Width  = GetSystemMetrics(SM_CXSCREEN);
            mode.Height = GetSystemMetrics(SM_CYSCREEN);

            
            hr = pSwapChain_->ResizeTarget(&mode);
            Assert::NotFailed(hr, "can't resize a target during switching of the fullscreen mode");

            hr = pSwapChain_->SetFullscreenState(TRUE, nullptr);
            Assert::NotFailed(hr, "can't set full screen state");

            // another one for good luck
            hr = pSwapChain_->ResizeTarget(&mode);
            Assert::NotFailed(hr, "can't resize a target during switching of the fullscreen mode");

            // resize the swapchain and related parts to the chosen solution
            ResizeSwapChain(hwnd, { (LONG)mode.Width, (LONG)mode.Height });
        }

        // switch to WINDOWED mode
        else
        {
            DXGI_MODE_DESC mode;
            ZeroMemory(&mode, sizeof(DXGI_MODE_DESC));

            //printf("restore windowed x: %d\n", windowedModeWidth_);
            //printf("restore windowed y: %d\n", windowedModeHeight_);

            // reset the last size of the window in the windowed mode
            mode.Width  = windowedModeWidth_;
            mode.Height = windowedModeHeight_;

            hr = pSwapChain_->SetFullscreenState(FALSE, nullptr);
            Assert::NotFailed(hr, "can't switch to WINDOWED mode");

            hr = pSwapChain_->ResizeTarget(&mode);
            Assert::NotFailed(hr, "can't resize a target during switching of the fullscreen mode");

            // resize the swapchain and related parts to the chosen solution
            ResizeSwapChain(hwnd, { (LONG)mode.Width, (LONG)mode.Height });
        }

        return true;
    }
    catch (EngineException& e)
    {
        Log::Error(e);
        Log::Error("error during switching WINDOWED/FULL_SCREEN mode");
        return false;
    }
}

} // namespace Core
