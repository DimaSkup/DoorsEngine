////////////////////////////////////////////////////////////////////
// Filename:     fontshaderclass.h
// Description:  this is a class for rendering fonts images
//               using HLSL shaders.
//
// Revising:     23.07.22
////////////////////////////////////////////////////////////////////
#pragma once

#include "VertexShader.h"
#include "PixelShader.h"
#include "SamplerState.h"
#include "ConstantBuffer.h"

#include "../Common/ConstBufferTypes.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>


namespace Render
{

class FontShaderClass final
{
public:
	using SRV = ID3D11ShaderResourceView;

public:
	FontShaderClass();
	~FontShaderClass();

	// restrict a copying of this class instance
	FontShaderClass(const FontShaderClass& obj) = delete;
	FontShaderClass& operator=(const FontShaderClass& obj) = delete;


	bool Initialize(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const DirectX::XMMATRIX& WVO);

	// Public rendering API
	void Render(
		ID3D11DeviceContext* pContext,
		const std::vector<ID3D11Buffer*>& textVBs,    // array of text vertex buffers
		const std::vector<ID3D11Buffer*>& textIBs,    // array of text indices buffers
		const std::vector<uint32_t>& indexCounts,
		const uint32_t fontVertexSize,
		SRV* const* ppFontTexSRV);
	
	// Public modification API
	void SetWorldViewOrtho(ID3D11DeviceContext* pContext, const DirectX::XMMATRIX& WVO);
	void SetFontColor(ID3D11DeviceContext* pContext, const DirectX::XMFLOAT3& color);
	

	// Public query API
	inline const std::string & GetShaderName() const { return className_; }
	inline ID3D11Buffer* GetConstBufferVS()   const { return matrixBuffer_.Get(); }
	inline ID3D11Buffer* GetConstBufferPS()   const { return pixelBuffer_.Get(); }

private:
	// initializes the HLSL shaders, input layout, sampler state and buffers
	void InitializeShaders(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const std::string& vsFilename,
		const std::string& psFilename,
		const DirectX::XMMATRIX& WVO);

private:
	VertexShader   vs_;
	PixelShader    ps_;
	SamplerState   samplerState_;

	ConstantBuffer<BuffTypes::ConstantMatrixBuffer_FontVS> matrixBuffer_;
	ConstantBuffer<BuffTypes::ConstantPixelBuffer_FontPS>  pixelBuffer_;   // text colour for the pixel shader

	std::string className_{ "font_shader_class" };
};

}