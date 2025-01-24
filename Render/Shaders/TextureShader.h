////////////////////////////////////////////////////////////////////
// Filename:     TextureShader.h
// Description:  this class will be used to draw the 2D/3D models
//               using the texture vertex and pixel shaders.
//               It renders only a texture;
//
// Revising:     09.04.22
////////////////////////////////////////////////////////////////////
#pragma once

#include "VertexShader.h"
#include "PixelShader.h"
#include "SamplerState.h"
#include "ConstantBuffer.h"
#include "../Common/RenderTypes.h"

#include <d3d11.h>
//#include <d3dx11async.h>

namespace Render
{

class TextureShader final
{
	using SRV = ID3D11ShaderResourceView;

	struct InstancedData
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX texTransform;
	};

public:
	TextureShader();
	~TextureShader();

	// restrict a copying of this class instance
	TextureShader(const TextureShader& obj) = delete;
	TextureShader& operator=(const TextureShader& obj) = delete;

	// Public modification API
	bool Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

	// Public rendering API
	void Render(
		ID3D11DeviceContext* pContext,
		ID3D11Buffer* pInstancedBuffer,
		const Instance* instances,
		const int numModels,
		const UINT instancedBuffElemSize);

	// Public API for controlling of shader rendering state
	void SwitchFog(ID3D11DeviceContext* pContext);
	void SwitchAlphaClipping(ID3D11DeviceContext* pContext);
	void SetFogParams(ID3D11DeviceContext* pContext, const DirectX::XMFLOAT3 & fogColor, const float fogStart, const float fogRange);
	
	// Public query API
	inline const std::string& GetShaderName() const { return className_; }	

private:
	void InitializeShaders(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const std::string& vsFilename,
		const std::string& psFilename);

private:
	VertexShader        vs_;
	PixelShader         ps_;
	SamplerState        samplerState_;

	const std::string className_{ "texture_shader" };
};

}  // namespace Render