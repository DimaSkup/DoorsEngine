// ====================================================================================/////
// Filename:       FrameBuffer.h
// 
// Description:    FrameBuffer allows you to render your scene to a 
//                 texture resource instead of just the back buffer. 
//                 It also allows you to retrieve 
//                 the data rendered to it in the form of a regular texture
//
// Created:        20.09.23  by DimaSkup
// ====================================================================================
#pragma once

#include <d3d11.h>
#include <DirectXMath.h>


namespace Core
{
    
struct FrameBufSpec
{
    UINT        width = 0;
    UINT        height = 0;
    DXGI_FORMAT format = DXGI_FORMAT(0);   // texture surface format
    float       screenNear = 0.0f;
    float       screenDepth = 0.0f;
};

///////////////////////////////////////////////////////////

class FrameBuffer
{
public:
    FrameBuffer();
    ~FrameBuffer();

    // move-semantic
    FrameBuffer(FrameBuffer&&) noexcept;
    FrameBuffer& operator=(FrameBuffer&&) noexcept;

    // restrict a copying of this class instance 
    FrameBuffer(const FrameBuffer& obj) = delete;
    FrameBuffer& operator=(const FrameBuffer& obj) = delete;

    bool Init(const FrameBufSpec& fbSpec);

    void ResizeBuffers(const int width, const int height);

    void Shutdown();

    void Bind();
    void ClearBuffers(float r, float g, float b, float a);


    //
    // inline getters
    //
    inline bool IsInit()                                      const { return isInit_; }

    inline ID3D11ShaderResourceView*  GetSRV()                const { return pShaderResourceView_; }
    inline ID3D11ShaderResourceView** GetAddressOfSRV()             { return &pShaderResourceView_; }

    inline const DirectX::XMMATRIX&   GetProjMatrix()         const { return projection_; }
    inline const DirectX::XMMATRIX&   GetOrthoMatrix()        const { return orthoMatrix_; }

    inline UINT  GetWidth()     const { return specification_.width; }
    inline UINT  GetHeight()    const { return specification_.height; }
    inline float GetAspect()    const { return (float)GetWidth() / (float)GetHeight(); }
    inline float GetNearZ()     const { return specification_.screenNear; }
    inline float GetFarZ()      const { return specification_.screenDepth; }


private:
    void CreateRenderTargetTexture();
    void CreateRenderTargetView();
    void CreateShaderResourceView();
    void CreateDepthStencilBuffer();
    void CreateDepthStencilView();
    void SetupViewportAndMatrices();

private:
    FrameBufSpec              specification_;
    ID3D11Texture2D*          pRenderTargetTexture_ = nullptr;
    ID3D11RenderTargetView*   pRenderTargetView_    = nullptr;
    ID3D11ShaderResourceView* pShaderResourceView_  = nullptr;
    ID3D11Texture2D*          pDepthStencilBuffer_  = nullptr;
    ID3D11DepthStencilView*   pDepthStencilView_    = nullptr;
    D3D11_VIEWPORT            viewport_             = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    DirectX::XMMATRIX         projection_           = DirectX::XMMatrixIdentity();
    DirectX::XMMATRIX         orthoMatrix_          = DirectX::XMMatrixIdentity();
    bool                      isInit_               = false;
};

} // namespace Core
