// ================================================================================
// Filename: d3dclass.cpp
// Revising: 01.01.23
// ================================================================================
#include "../Common/pch.h"
#include "d3dclass.h"


#pragma warning (disable : 4996)


// encourage the driver to select the discrete video adapter by default
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

namespace Render
{

D3DClass* D3DClass::pInstance_ = nullptr;

// Global pointers of DX11 device and device context
ID3D11Device*        g_pDevice;
ID3D11DeviceContext* g_pContext;


//---------------------------------------------------------
// Desc:   default constructor and destructor
//---------------------------------------------------------
D3DClass::D3DClass()
{
    if (D3DClass::pInstance_ == nullptr)
    {
        D3DClass::pInstance_ = this;
    }
    else
    {
        LogErr(LOG, "you can't create more than only one instance of the D3DClass");
        exit(0);
    }

    LogDbg(LOG, "D3DClass is init");
}

D3DClass::~D3DClass()
{
    Shutdown();
}

//---------------------------------------------------------
// Desc:   initialize all the DirectX11 stuff, some common params, depth-stencil, etc.
// Ret:    true if we managed to initialize
//---------------------------------------------------------
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
        LogDbg(LOG, "D3DClass start of initialization");

        // check if we have any available IDXGI adapter
        if (adaptersReader_.GetNumAdapters() <= 1)
        {
            LogDbg(LOG, "can't find any IDXGI adapter");
            exit(0);
        }

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
            if (!result)
            {
                LogErr(LOG, "can't get the client rectangle params");
                exit(0);
            }
        
            wndWidth_  = clientRect.right - clientRect.left;
            wndHeight_ = clientRect.bottom - clientRect.top;

            windowedModeWidth_ = wndWidth_;
            windowedModeHeight_ = wndHeight_;
        }


        // check params
        assert((wndWidth_ > 0) && (wndHeight_ > 0));
        assert((screenNear >= 0.0f) && (screenDepth > screenNear));

        vsyncEnabled_ = vsyncEnabled;        // define if VSYNC is enabled or not
        fullScreen_   = fullScreen;          // define if window is full screen or not
        enable4xMsaa_ = enable4xMSAA;        // use 4X MSAA?
        nearZ_        = screenNear;
        farZ_         = screenDepth;


        // initialize all the main parts of DirectX
        InitializeDirectX(hwnd, wndWidth_, wndHeight_, screenNear, screenDepth);

        // init blending states, rasterizer states, depth stencil states in a separate way
        renderStates_.InitAll(pDevice_, enable4xMSAA);
        InitializeDepthStencil(wndWidth_, wndHeight_);

        LogMsg(LOG, "is initialized successfully");
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        Shutdown();
        return false;
    }

    return true;
}

//---------------------------------------------------------
// reset the screen state and release the allocated memory
//---------------------------------------------------------
void D3DClass::Shutdown()
{
    // set a windowed mode as active
    if (pSwapChain_)
        pSwapChain_->SetFullscreenState(FALSE, nullptr);

    // release all the depth stencil stuff
    SafeRelease(&pDepthStencilView_);
    SafeRelease(&pDepthStencilBuffer_);
    SafeRelease(&pRenderTargetView_);
    //SafeRelease(&pBackBuffer_);
    SafeRelease(&pContext_);
    SafeRelease(&pDevice_);
    SafeRelease(&pSwapChain_);
}

//---------------------------------------------------------
// Desc:  before rendering of each frame we need to reset buffers
//---------------------------------------------------------
void D3DClass::BeginScene()
{
    const FLOAT bgColor[4]{ 0, 1, 1, 0 };
    
    // clear the render target view with particular color
    pContext_->ClearRenderTargetView(pRenderTargetView_, bgColor);

    // clear the depth stencil view with 1.0f values
    pContext_->ClearDepthStencilView(pDepthStencilView_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

//---------------------------------------------------------
// Desc:  present the back buffer content onto the screen
//---------------------------------------------------------
void D3DClass::EndScene()
{
    // if vertical synchronization is enabled the first param will be set to 1
    // or in another case it will be set to 0 (no vsync)
    pSwapChain_->Present((UINT)vsyncEnabled_, 0);
}

//---------------------------------------------------------
// Desc:   get a pointer to the DirectX11 device and device context
//---------------------------------------------------------
void D3DClass::GetDeviceAndContext(ID3D11Device*& pDevice, ID3D11DeviceContext*& pContext)
{
    pDevice  = pDevice_;
    pContext = pContext_;
}

//---------------------------------------------------------
// Desc:   get some info about our video card
// Out:    - outCardName:  the name of used GPU
//         - maxCardNameSize: 
//---------------------------------------------------------
void D3DClass::GetVideoCardInfo(
    char* outCardName, 
    const int maxCardNameSize,
    int& outMemorySize)
{
    // get data about the video card

    // get adapter's data
    AdapterData* data = adaptersReader_.GetAdapterDataByIdx(displayAdapterIndex_);
    DXGI_ADAPTER_DESC adapterDesc = data->description_;

    // store the dedicated video card memory in megabytes and store its name
    constexpr UINT bytesInMegabyte = 1024 * 1024;

    StrHelper::ToStr(adapterDesc.Description, outCardName);
    outMemorySize = (int)(adapterDesc.DedicatedVideoMemory / bytesInMegabyte);

    // print info about GPU into the console and log file
    SetConsoleColor(GREEN);
    LogMsg("Video card info:");
    LogMsg("Video card name: %s", outCardName);
    LogMsg("Video memory: %d MB", outMemorySize);
    SetConsoleColor(RESET);
}

//---------------------------------------------------------
// Desc:   we call this function to set up a raster state for proper
//         rendering of 2D elements / UI;
//         NOTE: we store a hash of the previous RS so later we can set it back
//---------------------------------------------------------
void D3DClass::TurnOnRSfor2Drendering()
{
    prevRasterStateHash_ = renderStates_.GetCurrentRSHash();
    renderStates_.SetRS(pContext_, { R_FILL_SOLID, R_CULL_BACK, R_FRONT_CLOCKWISE });
}

//---------------------------------------------------------
// Desc:  initialize DirectX11 device, context, swap chain, render target, viewport
//---------------------------------------------------------
void D3DClass::InitializeDirectX(
    HWND hwnd,
    UINT wndWidth,
    UINT wndHeight,
    float nearZ,
    float farZ)
{
    try
    {
        LogMsg(LOG, "Start initialization of DirectX stuff");

        if ((wndWidth == 0) || (wndHeight == 0))
        {
            LogErr(LOG, "wrong window dimensions; reset to default (800x600)");
            wndWidth  = 800;
            wndHeight = 600;
        }

        if (nearZ <= 0)
        {
            LogErr(LOG, "near plane can't be <= 0 (your value: %f); reset to default (1.0f)", nearZ);
            nearZ = 1.0f;
        }

        if (farZ <= nearZ)
        {
            LogErr(LOG, "far plane can't be <= near plane (far: %f; near: %f); reset farZ to default (500.0f)", farZ, nearZ);
            farZ = 500.0f;
        }

        // create the Direct3D 11 device and context
        InitializeDevice();

        // initialize all the main parts of DirectX
        InitializeSwapChain(hwnd, wndWidth, wndHeight);
        InitializeRenderTargetView();
        InitializeViewport(wndWidth, wndHeight);
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        throw EngineException("can't initialize DirectX stuff");
    }
}

//---------------------------------------------------------
// Desc:  initialize DirectX11 device and device context;
//        also it check the quality level support for 4X MSAA.
//---------------------------------------------------------
void D3DClass::InitializeDevice()
{
    D3D_FEATURE_LEVEL actualFeatureLevel;
    UINT createDeviceFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    const D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };

    const UINT numFeatureLevels = ARRAYSIZE(featureLevels);


    IDXGIAdapter* pAdapter = adaptersReader_.GetDXGIAdapterByIdx(displayAdapterIndex_);

    HRESULT hr = D3D11CreateDevice(
        pAdapter,                         // adapter (null for default)
        D3D_DRIVER_TYPE_UNKNOWN,          // driver type
        nullptr,                          // software rasterizer (nullptr for hardware)
        createDeviceFlags,                // flags
        featureLevels,                    // arr of feature levels to try
        numFeatureLevels,                 // number of feature levels in the arr
        D3D11_SDK_VERSION,                // SDK version
        &pDevice_,                        // pointer to the device
        &actualFeatureLevel,              // pointer to the actual feature level created
        &pContext_);                      // pointer to the immediate context

    if (FAILED(hr))
    {
        LogErr(LOG, "failed to create DX11 device");
        exit(0);
    }

    if (actualFeatureLevel < D3D_FEATURE_LEVEL_10_0)
    {
        LogErr(LOG, "the engine supports Direct3D Feature Level >= 10_0 (your level is %u; look at D3D_FEATURE_LEVEL enum)", (UINT)actualFeatureLevel);
        exit(0);
    }

    // now that we have a created device, we can check the quality level support for 4X MSAA.
    hr = pDevice_->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4xMsaaQuality_);
    if (FAILED(hr))
    {
        LogErr(LOG, "the multisample quality level number must be > 0");
        exit(0);
    }

    // also initialize the global pointers to device and device context
    g_pDevice  = pDevice_;
    g_pContext = pContext_;
}

//---------------------------------------------------------
// Desc:   handle the window resizing:
//         resize the swap chain, reinit related stuff, change window's params
//         (is used when we change the window size or switch btw editor and game modes)
// Ret:    true if we managed to do all right
//---------------------------------------------------------
bool D3DClass::ResizeSwapChain(HWND hwnd, SIZE newSize)
{
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

        // 1. Clear render targets from device context
        //    crear the previous window size specific context
        pContext->OMSetRenderTargets(0, 0, 0);

        // 2. Release rendering target
        SafeRelease(&pDepthStencilBuffer_);
        SafeRelease(&pDepthStencilView_);
        SafeRelease(&pRenderTargetView_);
        pContext->Flush();

        // 3. Resize buffer:
        //    Preserve the existing buffer count and format.
        //    Automatically choose the width and height to match the client rect for HWNDs.
        hr = pSwapChain_->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
        if (FAILED(hr))
        {
            LogErr(LOG, "can't resize swap chain's buffers");
            //exit(0);
        }

        UpdateWindow(hwnd);

        // 4. recreate the render target view, depth stencil buffer/view, and viewport
        InitializeRenderTargetView();
        InitializeDepthStencil(wndWidth_, wndHeight_);
        InitializeViewport(wndWidth_, wndHeight_);
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        Shutdown();
        return false;
    }

    return true;
}

//---------------------------------------------------------
// Desc:   initialize the DirectX11 swap chain
// Args:   - hwnd:   window handler
//         - width:  window width
//         - height: window height
//---------------------------------------------------------
void D3DClass::InitializeSwapChain(HWND hwnd, const int width, const int height)
{
    HRESULT hr = S_OK;
    DXGI_SWAP_CHAIN_DESC sd{ 0 };

    // Setup the swap chain description
    sd.BufferDesc.Width             = width;                                 // desired back buffer width
    sd.BufferDesc.Height            = height;                                // desired back buffer height
    sd.BufferDesc.Format            = backBufferFormat_;                     // use a simple 32-bit surface 
    sd.BufferDesc.ScanlineOrdering  = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	 // a rasterizer method to render an image on a surface
    sd.BufferDesc.Scaling           = DXGI_MODE_SCALING_UNSPECIFIED;         // how to scale an image to fit it to the screen resolution

    sd.BufferCount                  = 2;                                     // we have only one back buffer
    sd.BufferUsage                  = DXGI_USAGE_RENDER_TARGET_OUTPUT;       // use the back buffer as the render target output
    sd.OutputWindow                 = hwnd;                                  // set the current window
    sd.Windowed                     = !fullScreen_;                          // specity true to run in windowed mode or false for full-screen mode
    sd.SwapEffect                   = DXGI_SWAP_EFFECT_DISCARD;              // discard the content of the back buffer after presenting
    sd.Flags                        = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;


    sd.BufferDesc.RefreshRate.Numerator = (vsyncEnabled_) ? 60 : 0;
    sd.BufferDesc.RefreshRate.Denominator = 1;

    // Use 4X MSAA?
    // Note: m4xMsaaQuality_ is returned via CheckMultisampleQualitylevels()
    sd.SampleDesc.Count = (enable4xMsaa_) ? 4 : 1;
    sd.SampleDesc.Quality = (enable4xMsaa_) ? m4xMsaaQuality_ - 1 : 0;

    // for creation of the swap chain we have to use the IDXGIFactory instance that was
    // used to create the device
    IDXGIDevice* pDxgiDevice = nullptr;
    hr = pDevice_->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDxgiDevice);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't get the interface of DXGI Device");
        exit(0);
    }

    IDXGIAdapter* pDxgiAdapter = nullptr;
    hr = pDxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&pDxgiAdapter);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't get the interface of DXGI Adapter");
        exit(0);
    }

    // finally go the IDXGIFactory interface
    IDXGIFactory* pDxgiFactory = nullptr;
    hr = pDxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pDxgiFactory);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't get the interface of DXGI Factory");
        exit(0);
    }

    // Create the swap chain
    hr = pDxgiFactory->CreateSwapChain(pDevice_, &sd, &pSwapChain_);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't create the swap chain");
        exit(0);
    }

    if (!pSwapChain_)
    {
        LogErr(LOG, "something went wrong during creation of the swap chain because pSwapChain == NULLPTR");
        exit(0);
    }

    // release our acquired COM interfaces (because we are done with them)
    SafeRelease(&pDxgiDevice);
    SafeRelease(&pDxgiAdapter);
    SafeRelease(&pDxgiFactory);
}

//---------------------------------------------------------
// Desc:   create and set up the render target view to the back buffer
//---------------------------------------------------------
void D3DClass::InitializeRenderTargetView()
{
    HRESULT hr = S_OK;
        
    // obtain a ptr to the swap chain's back buffer which we will use as a render target
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = pSwapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (VOID**)&pBackBuffer);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't get a buffer from the swap chain");
        exit(0);
    }

    // create a render target view 
    if (pBackBuffer)
    {
        hr = pDevice_->CreateRenderTargetView(pBackBuffer, nullptr, &pRenderTargetView_);
        if (FAILED(hr))
        {
            LogErr(LOG, "can't create a render target view");
            SafeRelease(&pBackBuffer);
            exit(0);
        }
    }
    SafeRelease(&pBackBuffer);
}

//---------------------------------------------------------
// Desc:   create the depth stencil buffer, depth stencil view
//         and bind them to the current device context
//---------------------------------------------------------
void D3DClass::InitializeDepthStencil(const UINT width, const UINT height)
{
    try
    {
        InitializeDepthStencilTextureBuffer(width, height);
        InitializeDepthStencilView();

        // Set the depth stencil state.
        pContext_->OMSetDepthStencilState(renderStates_.GetDSS(R_DEPTH_ENABLED), 1);

        // bind together the render target view and the depth stencil view to the output merger stage
        pContext_->OMSetRenderTargets(1, &pRenderTargetView_, pDepthStencilView_);
    }
    catch (EngineException & e)
    {
        LogErr(e, true);
        throw EngineException("can't initialize some of the depth/stencil elements");
    }
}

//---------------------------------------------------------
// Desc:   initialize the DirectX11 depth-stencil buffer texture
//---------------------------------------------------------
void D3DClass::InitializeDepthStencilTextureBuffer(const UINT width, const UINT height)
{
    // describe our Depth/Stencil Buffer
    D3D11_TEXTURE2D_DESC desc;
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
    if (FAILED(hr))
    {
        LogErr(LOG, "can't create the depth stencil buffer");
        exit(0);
    }
} 

//---------------------------------------------------------
// Desc:   initialize the DirectX11 depth-stencil view
//---------------------------------------------------------
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
    HRESULT hr = pDevice_->CreateDepthStencilView(
        pDepthStencilBuffer_,
        nullptr,                  // &depthStencilViewDesc, -- because we specified the type of our depth/stencil buffer, we specify null for this parameter
        &pDepthStencilView_);

    if (FAILED(hr))
    {
        LogErr(LOG, "can't create a depth stencil view");
        exit(0);
    }
}

//---------------------------------------------------------
// Desc:   initialize the DirectX11 viewport
//---------------------------------------------------------
void D3DClass::InitializeViewport(const UINT width, const UINT height)
{
    SetupViewportParams((float)width, (float)height, 1, 0, 0, 0); 
}

//---------------------------------------------------------
// Desc:   setup/reinit the DirectX11 viewport
//---------------------------------------------------------
void D3DClass::SetupViewportParams(
    const float width,
    const float height,
    const float maxDepth,
    const float minDepth,
    const float topLeftX,
    const float topLeftY)
{
    viewport_ = { topLeftX, topLeftY, width, height, minDepth, maxDepth };

    pContext_->RSSetViewports(1, &viewport_); // bind the viewport
}

//---------------------------------------------------------
// Desc:   switch btw windowed/fullscreen modes
//---------------------------------------------------------
bool D3DClass::ToggleFullscreen(HWND hwnd, bool isFullscreen)
{
    try
    {
        LogDbg(LOG, "ToggleFullscreen(): %s", (isFullscreen) ? "true" : "false");

        fullScreen_ = isFullscreen;

        // turn on the fullscreen mode
        if (isFullscreen)
        {
            // store the current window size in windowed mode 
            // so we can restore it when go back from fullscreen to windowed mode
            windowedModeWidth_ = wndWidth_;
            windowedModeHeight_ = wndHeight_;

            HRESULT        hr = S_OK;
            DXGI_MODE_DESC mode;
            ZeroMemory(&mode, sizeof(DXGI_MODE_DESC));

            // currently we get the maximum monitor dimensions for the fullscreen
            mode.Width  = GetSystemMetrics(SM_CXSCREEN);
            mode.Height = GetSystemMetrics(SM_CYSCREEN);

            
            hr = pSwapChain_->ResizeTarget(&mode);
            CAssert::NotFailed(hr, "can't resize a target during switching of the fullscreen mode");

            hr = pSwapChain_->SetFullscreenState(TRUE, nullptr);
            CAssert::NotFailed(hr, "can't set full screen state");

            // another one for good luck
            hr = pSwapChain_->ResizeTarget(&mode);
            CAssert::NotFailed(hr, "can't resize a target during switching of the fullscreen mode");

            // resize the swapchain and related parts to the chosen solution
            ResizeSwapChain(hwnd, { (LONG)mode.Width, (LONG)mode.Height });
        }
        // switch to WINDOWED mode
        else
        {
            DXGI_MODE_DESC mode;
            ZeroMemory(&mode, sizeof(DXGI_MODE_DESC));

            // reset the last size of the window in the windowed mode
            mode.Width  = windowedModeWidth_;
            mode.Height = windowedModeHeight_;

            HRESULT hr = pSwapChain_->SetFullscreenState(FALSE, nullptr);
            CAssert::NotFailed(hr, "can't switch to WINDOWED mode");

            hr = pSwapChain_->ResizeTarget(&mode);
            CAssert::NotFailed(hr, "can't resize a target during switching of the fullscreen mode");

            // resize the swapchain and related parts to the chosen solution
            ResizeSwapChain(hwnd, { (LONG)mode.Width, (LONG)mode.Height });
        }

        return true;
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr("error during switching WINDOWED/FULL_SCREEN mode");
        return false;
    }
}

} // namespace Core
