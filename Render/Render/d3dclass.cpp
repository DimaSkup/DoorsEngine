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

D3DClass* D3DClass::pInstance_  = nullptr;

// Global pointers of DX11 device and device context
ID3D11Device*        g_pDevice  = nullptr;
ID3D11DeviceContext* g_pContext = nullptr;


//---------------------------------------------------------
// Desc:   default constructor and destructor
//---------------------------------------------------------
D3DClass::D3DClass()
{
    if (D3DClass::pInstance_ != nullptr)
    {
        LogErr(LOG, "you can't create more than only one instance of the D3DClass");
        exit(0);
    }

    D3DClass::pInstance_ = this;
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
        fullScreenWndWidth_  = GetSystemMetrics(SM_CXSCREEN);
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
        InitDirectX(hwnd, wndWidth_, wndHeight_, screenNear, screenDepth);

        // init blending states, rasterizer states, depth stencil states in a separate way
        renderStates_.InitAll(pDevice_, enable4xMSAA);
        InitDepthStencil(wndWidth_, wndHeight_);

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

    ReleaseTargets();

    // release device, context and swap chain stuff
    //SafeRelease(&pBackBufferSRV_);
    SafeRelease(&pSwapChain_);
    SafeRelease(&pContext_);
    SafeRelease(&pDevice_);
}

//---------------------------------------------------------
// Desc:  before rendering of each frame we need to reset buffers
//---------------------------------------------------------
void D3DClass::BeginScene()
{
    //const FLOAT bgColor[4]{ 0, 1, 1, 0 };
    const FLOAT bgColor[4]{ 0.53f, 0.81f, 0.92f, 1.0f };  // sky blue
    
    // clear the render target view with particular color
    pContext_->ClearRenderTargetView(pSwapChainRTV_, bgColor);

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
    //pSwapChain_->Present(1, 0);
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
void D3DClass::InitDirectX(
    HWND hwnd,
    const UINT wndWidth,
    const UINT wndHeight,
    const float nearZ,
    const float farZ)
{
    LogMsg(LOG, "Start initialization of DirectX stuff");

    try
    {
        // check input args
        if ((wndWidth == 0) || (wndHeight == 0))
        {
            LogErr(LOG, "wrong window dimensions (%u x %u)", wndWidth, wndHeight);
            exit(0);
        }

        if (nearZ <= 0)
        {
            LogErr(LOG, "near plane can't be <= 0 (your value: %f)", nearZ);
            exit(0);
        }

        if (farZ <= nearZ)
        {
            LogErr(LOG, "far plane can't be <= near plane (far: %f; near: %f)", farZ, nearZ);
            exit(0);
        }

        // create the Direct3D 11 device and context
        InitDevice();

        // initialize all the main parts of DirectX
        InitSwapChain(hwnd, wndWidth, wndHeight);
        InitRenderTargetView(wndWidth, wndHeight);
        InitViewport(wndWidth, wndHeight);

        // bind together the render target view and the depth stencil view to the output merger stage
        pContext_->OMSetRenderTargets(1, &pSwapChainRTV_, pDepthStencilView_);
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
void D3DClass::InitDevice()
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
bool D3DClass::ResizeSwapChain(HWND hwnd, const UINT width, const UINT height)
{
    try
    {
        // maybe we got an WM_SIZE event after CreateWindowEx and currently our
        // swap chain isn't created yet so just go out
        if (pSwapChain_ == nullptr)
            return true;

        HRESULT hr = S_OK;
        ID3D11DeviceContext* pContext = GetDeviceContext();

        printf("resize swap chain: %d x %d\n", (int)width, (int)height);

        wndWidth_  = width;
        wndHeight_ = height;

        // if we resize when in windowed mode
        if (!fullScreen_)
        {
            windowedModeWidth_  = width;
            windowedModeHeight_ = height;
        }

        // 1. Clear render targets from device context
        //    crear the previous window size specific context
        pContext->OMSetRenderTargets(0, 0, 0);

        // 2. Release rendering target and all the related stuff
        ReleaseTargets();

        pContext->Flush();

        // 3. Resize buffer:
        //    Preserve the existing buffer count and format.
        //    Automatically choose the width and height to match the client rect for HWNDs.
        hr = pSwapChain_->ResizeBuffers(
            0, 0, 0,
            DXGI_FORMAT_UNKNOWN,
            DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
        if (FAILED(hr))
        {
            LogErr(LOG, "can't resize swap chain's buffers");
        }

        UpdateWindow(hwnd);

        // 4. recreate the render target view, depth stencil buffer/view, and viewport
        InitRenderTargetView(wndWidth_, wndHeight_);
        InitDepthStencil(wndWidth_, wndHeight_);
        InitViewport(wndWidth_, wndHeight_);
   
        // bind together the render target view and the depth stencil view to the output merger stage
        pContext_->OMSetRenderTargets(1, &pSwapChainRTV_, pDepthStencilView_);
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
// Desc:  switch anti-aliasing type (or not use it at all)
// NOTE:  takes as input AntiAliasingType casted to uint8
//---------------------------------------------------------
void D3DClass::SetAntiAliasingType(const uint8 type)
{
    const AntiAliasingType aaType = (AntiAliasingType)type;

    if (aaType >= NUM_AA_TYPES)
    {
        LogErr(LOG, "can't set anti-aliasing type: input type is wrong (%d)", (int)type);
        return;
    }

    if (aaType == currAntiAliasing_)
        return;


    currAntiAliasing_ = aaType;
    ReleaseTargets();

    switch (aaType)
    {
        case AA_TYPE_NO_AA:
        {
            enable4xMsaa_ = false;
            break;
        }
        case AA_TYPE_FXAA:
        {
            enable4xMsaa_ = false;
            break;
        }
        case AA_TYPE_MSAA:
        {
            enable4xMsaa_ = true;

            HRESULT hr = pDevice_->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4xMsaaQuality_);
            if (FAILED(hr))
            {
                LogErr(LOG, "the multisample quality level number must be > 0");
                exit(0);
            }

            break;
        }
    }

    SafeRelease(&pSwapChain_);
    InitSwapChain(hwnd_, wndWidth_, wndHeight_);
    InitRenderTargetView(wndWidth_, wndHeight_);
    InitDepthStencil(wndWidth_, wndHeight_);
    InitViewport(wndWidth_, wndHeight_);

    // bind together the render target view and the depth stencil view to the output merger stage
    pContext_->OMSetRenderTargets(1, &pSwapChainRTV_, pDepthStencilView_);
}

//---------------------------------------------------------
// Desc:   initialize the DirectX11 swap chain
// Args:   - hwnd:   window handler
//         - width:  window width
//         - height: window height
//---------------------------------------------------------
void D3DClass::InitSwapChain(HWND hwnd, const int width, const int height)
{
    HRESULT hr = S_OK;
    DXGI_SWAP_CHAIN_DESC sd = {};

    // store HWND for later using (for instance: when change anti-aliasing type)
    hwnd_ = hwnd;

    // Setup the swap chain description
    sd.BufferDesc.Width             = width;                                 // desired back buffer width
    sd.BufferDesc.Height            = height;                                // desired back buffer height
    sd.BufferDesc.Format            = backBufFormat_;                        // use a simple 32-bit surface 
    sd.BufferDesc.ScanlineOrdering  = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	 // a rasterizer method to render an image on a surface
    sd.BufferDesc.Scaling           = DXGI_MODE_SCALING_UNSPECIFIED;         // how to scale an image to fit it to the screen resolution

    sd.BufferDesc.RefreshRate.Numerator   = (vsyncEnabled_) ? 60 : 0;
    sd.BufferDesc.RefreshRate.Denominator = 1;

    sd.BufferCount                  = 2;                                     // we have only one back buffer
    sd.BufferUsage                  = DXGI_USAGE_RENDER_TARGET_OUTPUT;       // use the back buffer as the render target output
    sd.OutputWindow                 = hwnd;                                  // set the current window
    sd.Windowed                     = !fullScreen_;                          // specity true to run in windowed mode or false for full-screen mode
    sd.SwapEffect                   = DXGI_SWAP_EFFECT_DISCARD;              // discard the content of the back buffer after presenting
    sd.Flags                        = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    // Use 4X MSAA?
    // Note: m4xMsaaQuality_ is returned via CheckMultisampleQualitylevels()
    sd.SampleDesc.Count   = (enable4xMsaa_) ? 4 : 1;
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
        LogErr(LOG, "can't create a swap chain");
        exit(0);
    }

    if (!pSwapChain_)
    {
        LogErr(LOG, "something went wrong during creation of the swap chain because swap chaing ptr == NULL");
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
void D3DClass::InitRenderTargetView(const UINT width, const UINT height)
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
        hr = pDevice_->CreateRenderTargetView(pBackBuffer, nullptr, &pSwapChainRTV_);
        if (FAILED(hr))
        {
            LogErr(LOG, "can't create a render target view");
            Shutdown();
            exit(0);
        }
    }

    SafeRelease(&pBackBuffer);

    //-----------------------------------------------------

    UINT qualityLevels = 0;
    hr = pDevice_->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &qualityLevels);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't get multisample quality levels");
        Shutdown();
        exit(0);
    }

    // create MSAA render target
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width              = width;
    desc.Height             = height;
    desc.MipLevels          = 1;
    desc.ArraySize          = 1;
    desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count   = 4;
    desc.SampleDesc.Quality = qualityLevels - 1;
    desc.Usage              = D3D11_USAGE_DEFAULT;
    desc.BindFlags          = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags     = 0;
    desc.MiscFlags          = 0;

    hr = pDevice_->CreateTexture2D(&desc, nullptr, &pMSAAColorTex_);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't create MSAA color texture");
        Shutdown();
        exit(0);
    }

    D3D11_RENDER_TARGET_VIEW_DESC rtvMsDesc = {};
    rtvMsDesc.Format = desc.Format;
    rtvMsDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;

    hr = pDevice_->CreateRenderTargetView(pMSAAColorTex_, &rtvMsDesc, &pMSAARTV_);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't create MSAA render target");
        Shutdown();
        exit(0);
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvMsDesc = {};
    srvMsDesc.Format = desc.Format;
    srvMsDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
    hr = pDevice_->CreateShaderResourceView(pMSAAColorTex_, &srvMsDesc, &pMSAASRV_);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't create MSAA SRV");
        Shutdown();
        exit(0);
    }

    //-----------------------------------------------------

    // create 2 targets for post process (texture, render target, shader resource view)
    for (int i = 0; i < 2; ++i)
    {
        // create resolved texture (non-MSAA, shader-readable, for post effects)
        desc.Width              = width;
        desc.Height             = height;
        desc.MipLevels          = 1;
        desc.ArraySize          = 1;
        desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage              = D3D11_USAGE_DEFAULT;
        desc.BindFlags          = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags     = 0;
        desc.MiscFlags          = 0;

        hr = pDevice_->CreateTexture2D(&desc, nullptr, &postFxsPassTex_[i]);
        if (FAILED(hr))
        {
            LogErr(LOG, "can't create a texture for post effects (%d)", i);
            Shutdown();
            exit(0);
        }

        // create render target view (RTV)
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format          = desc.Format;
        rtvDesc.ViewDimension   = D3D11_RTV_DIMENSION_TEXTURE2D;

        hr = pDevice_->CreateRenderTargetView(postFxsPassTex_[i], &rtvDesc, &postFxsPassRTV_[i]);
        if (FAILED(hr))
        {
            LogErr(LOG, "can't create render target (RTV) for post effects (%d)", i);
            Shutdown();
            exit(0);
        }

        // create shader resource view (SRV)
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format                    = desc.Format;
        srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels       = 1;

        hr = pDevice_->CreateShaderResourceView(postFxsPassTex_[i], &srvDesc, &postFxsPassSRV_[i]);
        if (FAILED(hr))
        {
            LogErr(LOG, "can't create shader resource view (SRV) for post effects (%d)", i);
            Shutdown();
            exit(0);
        }
    } // for
}

//---------------------------------------------------------
// Desc:   create the depth stencil buffer, depth stencil view
//         and bind them to the current device context
//---------------------------------------------------------
void D3DClass::InitDepthStencil(const UINT width, const UINT height)
{
    try
    {
        const UINT mipLevels = 1;

        InitDepthStencilTexBuf(width, height, mipLevels);
        InitDepthStencilView();
        InitDepthStencilSRV(mipLevels);

        if (IsEnabled4xMSAA())
        {
            InitDepthResolvedTexture(width, height, mipLevels);
        }

        InitDepthPrepassTargets(width, height, mipLevels);
    }
    catch (EngineException& e)
    {
        LogErr(e, true);
        Shutdown();
        exit(0);
        throw EngineException("can't initialize some of the depth/stencil elements");
    }
}

//---------------------------------------------------------
// Desc:   initialize the DirectX11 depth-stencil buffer texture
//---------------------------------------------------------
void D3DClass::InitDepthStencilTexBuf(
    const UINT width,
    const UINT height,
    const UINT mipLevels)
{
    const DXGI_FORMAT depthFormat = DXGI_FORMAT_R24G8_TYPELESS;

    // describe our depth-stencil Buffer
    D3D11_TEXTURE2D_DESC desc;
    desc.Width              = width;
    desc.Height             = height;
    desc.Format             = depthFormat;
    desc.MipLevels          = mipLevels;
    desc.ArraySize          = 1;

    desc.Usage              = D3D11_USAGE_DEFAULT;
    desc.BindFlags          = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags     = 0;
    desc.MiscFlags          = 0;

    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;

    // Use 4x MSAA?
    // Note: m4xMsaaQuality_ is returned via CheckMultisampleQualitylevels()
    if (IsEnabled4xMSAA())
    {
        desc.SampleDesc.Count   = 4;                      // 4x MSAA
        desc.SampleDesc.Quality = m4xMsaaQuality_ - 1;
    }

    // Create the depth/stencil buffer
    HRESULT hr = pDevice_->CreateTexture2D(&desc, nullptr, &pDepthStencilBuffer_);
    CAssert::NotFailed(hr, "can't create the depth stencil buffer");
} 

//---------------------------------------------------------
// Desc:   initialize the DirectX11 depth-stencil view
//---------------------------------------------------------
void D3DClass::InitDepthStencilView()
{
    // create the depth stencil view:
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    ZeroMemory(&dsvDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));

    // Setup the depth stencil view description
    dsvDesc.Format        = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

    if (IsEnabled4xMSAA())
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;

    HRESULT hr = pDevice_->CreateDepthStencilView(pDepthStencilBuffer_, &dsvDesc, &pDepthStencilView_);
    CAssert::NotFailed(hr, "can't create a depth stencil view");
}

//---------------------------------------------------------
// Desc:  create a depth stencil shader resource view for the shader
//---------------------------------------------------------
void D3DClass::InitDepthStencilSRV(const UINT mipLevels)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

    srvDesc.Format                    = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels       = mipLevels;      // same as orig texture (depth buf)
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2DMS.UnusedField_NothingToDefine = 0;

    if (IsEnabled4xMSAA())
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;

    HRESULT hr = pDevice_->CreateShaderResourceView(pDepthStencilBuffer_, &srvDesc, &pDepthSRV_);
    CAssert::NotFailed(hr, "can't create a shader resource view for depth buffer");
}

//---------------------------------------------------------
// Desc:  create a resolved depth texture (non-MSAA):
//        we'll copy (resolve) the per-sample depth into a single-sample float
//        texture we can use in post-processing
//---------------------------------------------------------
void D3DClass::InitDepthResolvedTexture(
    const UINT width,
    const UINT height,
    const UINT mipLevels)
{
    HRESULT hr = S_OK;

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width              = width;
    texDesc.Height             = height;
    texDesc.MipLevels          = mipLevels;
    texDesc.ArraySize          = 1;
    texDesc.Format             = DXGI_FORMAT_R32_FLOAT; // we'll output linear depth
    texDesc.SampleDesc.Count   = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage              = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags          = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    hr = pDevice_->CreateTexture2D(&texDesc, nullptr, &pResolvedDepthTex_);
    CAssert::NotFailed(hr, "can't create a resolved depth texture");

    // create a shader view for resolved depth texture
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format              = texDesc.Format;
    srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
    srvDesc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;

    hr = pDevice_->CreateShaderResourceView(pResolvedDepthTex_, &srvDesc, &pResolvedDepthSRV_);
    CAssert::NotFailed(hr, "can't create a resolved depth shader resource view");
}

//---------------------------------------------------------
// Desc:  initialize depth prepass targets
//        (also used for SSAO computation: normal/depth)
//---------------------------------------------------------
void D3DClass::InitDepthPrepassTargets(
    const UINT width,
    const UINT height,
    const UINT mipLevels)
{
    // init normal/depth texture map, where RGB stores the view space
    // normal and the alpha channel store the view space depth (z-coord)

    D3D11_TEXTURE2D_DESC desc;
    desc.Width              = width;
    desc.Height             = height;
    desc.Format             = DXGI_FORMAT_R16G16B16A16_FLOAT;
    desc.MipLevels          = mipLevels;
    desc.ArraySize          = 1;

    desc.Usage              = D3D11_USAGE_DEFAULT;
    desc.BindFlags          = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags     = 0;
    desc.MiscFlags          = 0;

    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;

    // Use 4x MSAA?
    // Note: m4xMsaaQuality_ is returned via CheckMultisampleQualitylevels()
    if (IsEnabled4xMSAA())
    {
        desc.SampleDesc.Count   = 4;                      // 4x MSAA
        desc.SampleDesc.Quality = m4xMsaaQuality_ - 1;
    }

    HRESULT hr = pDevice_->CreateTexture2D(&desc, nullptr, &pNormalDepthTex_);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't create the normal/depth texture");
        Shutdown();
        exit(0);
    }

     // create render target view (RTV)
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format          = desc.Format;
    rtvDesc.ViewDimension   = D3D11_RTV_DIMENSION_TEXTURE2D;

    if (IsEnabled4xMSAA())
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;

    hr = pDevice_->CreateRenderTargetView(pNormalDepthTex_, &rtvDesc, &pNormalDepthRTV_);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't create render target (RTV) for normal/depth");
        Shutdown();
        exit(0);
    }



    // create shader resource view (SRV)
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format                    = desc.Format;
    srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels       = 1;

    if (IsEnabled4xMSAA())
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;

    hr = pDevice_->CreateShaderResourceView(pNormalDepthTex_, &srvDesc, &pNormalDepthSRV_);
    if (FAILED(hr))
    {
        LogErr(LOG, "can't create shader resource view (SRV) for normal/depth");
        Shutdown();
        exit(0);
    }
}

//---------------------------------------------------------
// Desc:   initialize the DirectX11 viewport
//---------------------------------------------------------
void D3DClass::InitViewport(const UINT width, const UINT height)
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
            ResizeSwapChain(hwnd, mode.Width, mode.Height);
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
            ResizeSwapChain(hwnd, mode.Width, mode.Height);
        }

        return true;
    }
    catch (EngineException& e)
    {
        LogErr(e);
        LogErr(LOG, "error during switching WINDOWED/FULL_SCREEN mode");
        return false;
    }
}

void D3DClass::ReleaseTargets()
{
    // release depth visualization stuff (post-process)
    SafeRelease(&pResolvedDepthSRV_);
    SafeRelease(&pResolvedDepthTex_);

    // release all the depth stencil stuff
    SafeRelease(&pDepthSRV_);
    SafeRelease(&pDepthStencilView_);
    SafeRelease(&pDepthStencilBuffer_);

    ReleaseDepthPrepassTargets();

    // release targets used for post effects passes
    ReleasePostEffectsStuff();

    // release MSAA targets (used for post processing, depth visualization, etc.)
    ReleaseMSAATargets();

    SafeRelease(&pSwapChainRTV_);
}

//---------------------------------------------------------
// Desc:  release textures, render targets, shader resource views
//        related to post effects render passes
//---------------------------------------------------------
void D3DClass::ReleasePostEffectsStuff()
{
    for (int i = 0; i < 2; ++i)
    {
        SafeRelease(&postFxsPassSRV_[i]);
        SafeRelease(&postFxsPassRTV_[i]);
        SafeRelease(&postFxsPassTex_[i]);
    }
}

//---------------------------------------------------------
// Desc:   release targets which are used for post processing, depth visualization, etc.
//         when 4xMSAA is enabled
//         (firstly we render geometry into this targets, and then
//          we resolve it into non-MSAA target for later using)
//---------------------------------------------------------
void D3DClass::ReleaseMSAATargets()
{
    SafeRelease(&pMSAASRV_);
    SafeRelease(&pMSAARTV_);
    SafeRelease(&pMSAAColorTex_);
}

//---------------------------------------------------------
// Desc:  release targets used for depth prepass computation,
//        and also for SSAO computation
//---------------------------------------------------------
void D3DClass::ReleaseDepthPrepassTargets()
{
    SafeRelease(&pNormalDepthTex_);
    SafeRelease(&pNormalDepthRTV_);
    SafeRelease(&pNormalDepthSRV_);
}

} // namespace Core
