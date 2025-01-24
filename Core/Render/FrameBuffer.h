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

#include "../Common/Types.h"

#include <d3d11.h>
#include <DirectXMath.h>



struct FrameBufferSpecification
{
	UINT width = 0;
	UINT height = 0;
	DXGI_FORMAT format = DXGI_FORMAT(0);   // texture surface format
	float screenNear = 0.0f;
	float screenDepth = 0.0f;
	//bool swapChainTarget = false;
};


class FrameBuffer
{


public:
	FrameBuffer();
	~FrameBuffer();

	// restrict a copying of this class instance 
	FrameBuffer(const FrameBuffer& obj) = delete;
	FrameBuffer& operator=(const FrameBuffer& obj) = delete;

	bool Initialize(
		ID3D11Device* pDevice,
		const FrameBufferSpecification& spec);

	void ResizeBuffers(
		ID3D11DeviceContext* pContext,
		const int newWidth,
		const int newHeight);

	void Shutdown();

	void Bind(ID3D11DeviceContext* pContext);
	void ClearBuffers(ID3D11DeviceContext* pContext, const DirectX::XMFLOAT4& rgbaColor);
	void Unbind(ID3D11DeviceContext* pContext);


	//
	// inline getters
	//
	inline ID3D11ShaderResourceView*  GetSRV()                const { return pShaderResourceView_; }
	inline ID3D11ShaderResourceView** GetAddressOfSRV()             { return &pShaderResourceView_; }

	inline void GetProjectionMatrix(DirectX::XMMATRIX& proj)  const { proj = projection_; }
	inline void GetOrthoMatrix(DirectX::XMMATRIX& ortho)      const { ortho = orthoMatrix_; }

	inline UINT GetTexWidth()                                 const { return specification_.width; }
	inline UINT GetTexHeight()                                const { return specification_.height; }



private:
	void CreateRenderTargetTexture(ID3D11Device* pDevice);
	void CreateRenderTargetView(ID3D11Device* pDevice);
	void CreateShaderResourceView(ID3D11Device* pDevice);
	void CreateDepthStencilBuffer(ID3D11Device* pDevice);
	void CreateDepthStencilView(ID3D11Device* pDevice);
	void SetupViewportAndMatrices();

private:
	FrameBufferSpecification  specification_;
	ID3D11Texture2D*          pRenderTargetTexture_ = nullptr;
	ID3D11RenderTargetView*   pRenderTargetView_    = nullptr;
	ID3D11ShaderResourceView* pShaderResourceView_  = nullptr;
	ID3D11Texture2D*          pDepthStencilBuffer_  = nullptr;
	ID3D11DepthStencilView*   pDepthStencilView_    = nullptr;
	D3D11_VIEWPORT            viewport_             = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	DirectX::XMMATRIX         projection_           = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX         orthoMatrix_          = DirectX::XMMatrixIdentity();

};
