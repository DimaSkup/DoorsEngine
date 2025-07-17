// ================================================================================
// Filename:     d3dclass.h
// Description:  here we initialize all the stuff which is reponsible
//               for work with DirectX; enumerate adapters, execute all
//               the primary initialization of devices, etc.
// Revising:     21.03.22
// ================================================================================
#pragma once

#include "AdapterReader.h"
#include "RenderStates.h"

#include <d3dcommon.h>
#include <DirectXMath.h>

namespace Core
{

class D3DClass
{
public:
    D3DClass();
    ~D3DClass();

    // restrict a copying of this class instance
    D3DClass(const D3DClass& obj) = delete;
    D3DClass& operator=(const D3DClass& obj) = delete;
        

    bool Initialize(
        HWND hwnd, 
        const bool vsync, 
        const bool fullScreen, 
        const bool enable4xMSAA,
        const float screenNear, 
        const float screenDepth);

    void SetupViewportParams(
        const float width,
        const float height,
        const float maxDepth,
        const float minDepth,
        const float topLeftX,
        const float topLeftY);

    void Shutdown();

    // execute some operations before each frame and after each frame
    void BeginScene();
    void EndScene();
    

    void GetDeviceAndContext(ID3D11Device*& pDevice, ID3D11DeviceContext*& pContext);
    void GetVideoCardInfo(char* outCardName, const int maxCardNameSize, int& outMemorySize);

    //
    // inline getters
    //
    inline static D3DClass*          Get()                       { return pInstance_; }
    inline ID3D11Device*             GetDevice()           const { return pDevice_; }
    inline ID3D11DeviceContext*      GetDeviceContext()    const { return pContext_; }
    inline ID3D11DepthStencilView*   GetDepthStencilView() const { return pDepthStencilView_;}
    inline ID3D11Texture2D*          GetBackBufferTex()    const { return pBackBuffer_;}
    inline ID3D11RenderTargetView*   GetRenderTargetView() const { return pRenderTargetView_; }
    inline DXGI_FORMAT               GetBackBufferFormat() const { return backBufferFormat_; }
    inline int                       GetWindowWidth()      const { return wndWidth_; }
    inline int                       GetWindowHeight()     const { return wndHeight_; }
    inline float                     GetAspectRatio()      const { return (float)wndWidth_ / (float)wndHeight_; }
    inline float                     GetScreenNear()       const { return screenNear_; }
    inline float                     GetScreenDepth()      const { return screenDepth_; }

    // get world/ortho matrix
    //inline const DirectX::XMMATRIX& GetWorldMatrix()       const { return worldMatrix_; }
    //inline const DirectX::XMMATRIX& GetOrthoMatrix()       const { return orthoMatrix_; }
    //inline void GetWorldMatrix(DirectX::XMMATRIX& worldMatrix)   { worldMatrix = worldMatrix_; }
    //inline void GetOrthoMatrix(DirectX::XMMATRIX& orthoMatrix)   { orthoMatrix = orthoMatrix_; }

    inline RenderStates& GetRenderStates()                       { return renderStates_; }

    // set rasterizer states (RS)
    inline void SetRS(const eRenderState state)                  { renderStates_.SetRS(pContext_, state); }
    inline void SetRS(const std::set<eRenderState>& states)      { renderStates_.SetRS(pContext_, states); }

    // turning the Z buffer on and off when rendering 2D images
    inline void TurnZBufferOn()                                  { pContext_->OMSetDepthStencilState(renderStates_.GetDSS(eRenderState::DEPTH_ENABLED), 1); }
    inline void TurnZBufferOff()                                 { pContext_->OMSetDepthStencilState(renderStates_.GetDSS(eRenderState::DEPTH_DISABLED), 1); }

    inline void TurnOnBlending(const eRenderState state)         { renderStates_.SetBS(pContext_, state); }
    inline void TurnOffBlending()                                { renderStates_.SetBS(pContext_, eRenderState::ALPHA_DISABLE); }

    // set default render target/viewport
    inline void ResetBackBufferRenderTarget()                    { pContext_->OMSetRenderTargets(1, &pRenderTargetView_, pDepthStencilView_); }
    inline void ResetViewport()                                  { pContext_->RSSetViewports(1, &viewport_); }

    void TurnOnRSfor2Drendering();
    inline void TurnOffRSfor2Drendering()                        { renderStates_.SetRSByHash(pContext_, prevRasterStateHash_); }

    // fullscreen/windowed stuff
    bool ToggleFullscreen(HWND hwnd, bool isFullscreen);
    inline SIZE GetWindowedWndSize()                       const { return { windowedModeWidth_, windowedModeHeight_ }; }
    inline SIZE GetFullscreenWndSize()                     const { return { fullScreenWndWidth_, fullScreenWndHeight_ }; }

    // handler for the window resizing
    bool ResizeSwapChain(HWND hwnd, SIZE newSize);

    // memory allocation
    void* operator new(size_t i);
    void operator delete(void* p);

private:

    void InitializeDirectX(
        HWND hwnd, 
        const UINT windowWidth,
        const UINT windowHeight, 
        const float nearZ, 
        const float farZ);

    void EnumerateAdapters(); // get data about the video card, user's screen, etc.
    void InitializeDevice();
    void InitializeSwapChain(HWND hwnd, const int width, const int height);
    void InitializeRenderTargetView();

    // initialize depth stencil parts
    void InitializeDepthStencil(const UINT width, const UINT height);
    void InitializeDepthStencilTextureBuffer(const UINT width, const UINT height);
    void InitializeDepthStencilView();

    void InitializeViewport(const UINT width, const UINT height);

    
private:
    static D3DClass* pInstance_;

    //DirectX::XMMATRIX         worldMatrix_ = DirectX::XMMatrixIdentity();
    //DirectX::XMMATRIX         orthoMatrix_ = DirectX::XMMatrixIdentity();

    IDXGISwapChain*			  pSwapChain_        = nullptr;    
    ID3D11Device*			  pDevice_           = nullptr;    
    ID3D11DeviceContext*	  pContext_          = nullptr;    
    ID3D11Texture2D*          pBackBuffer_       = nullptr;    // the render target texture resource
    ID3D11RenderTargetView*   pRenderTargetView_ = nullptr;    // where we are going to render our buffers
    D3D11_VIEWPORT            viewport_{0};

    // depth stencil stuff
    ID3D11Texture2D*		  pDepthStencilBuffer_ = nullptr;
    ID3D11DepthStencilView*	  pDepthStencilView_ = nullptr;

    AdapterReader             adaptersReader_;
    RenderStates              renderStates_;
    uint8_t                   prevRasterStateHash_ = 0;

    DXGI_FORMAT backBufferFormat_ = DXGI_FORMAT_R8G8B8A8_UNORM;
    int   wndWidth_             = 800;       // current window width
    int   wndHeight_            = 600;       // current window height
    int   windowedModeWidth_    = 800;       // current wnd width for the windowed mode (we will use it when turn back to the windowed mode)
    int   windowedModeHeight_   = 600;       // current wnd height for the windowed mode
    int   fullScreenWndWidth_   = 800;       // current wnd width for the full screen mode
    int   fullScreenWndHeight_  = 600;       // current wnd height for the full screen mode
    float screenNear_           = 0.0f;
    float screenDepth_          = 100.0f;
    
    bool enableAltEnter_        = true;
    bool vsyncEnabled_          = false;
    bool fullScreen_            = false;
    bool enable4xMsaa_          = false;   // use 4X MSAA?
    UINT m4xMsaaQuality_        = 0;       // 4X MSAA quality level
    UINT displayAdapterIndex_   = 0;       // set adapter idx (if there is any discrete graphics adapter we use this discrete adapter as primary)
};


//==================================================================================
// Global pointers of DX11 device and device context
//==================================================================================
extern ID3D11Device*        g_pDevice;
extern ID3D11DeviceContext* g_pContext;

} // namespace Core
