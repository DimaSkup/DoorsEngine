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
#include "ConstantBuffer.h"
#include "../Common/RenderTypes.h"

#include <d3d11.h>

namespace Render
{

class TextureShader final
{

public:
	TextureShader();
	~TextureShader();

	// restrict a copying of this class instance
	TextureShader(const TextureShader& obj) = delete;
	TextureShader& operator=(const TextureShader& obj) = delete;

	// Public modification API
	bool Initialize(ID3D11Device* pDevice, const char* vsFilePath, const char* psFilePath);

	// Public rendering API
	void Render(
		ID3D11DeviceContext* pContext,
		ID3D11Buffer* pInstancedBuffer,
		const InstanceBatch* instances,
		const int numModels,
		const UINT instancedBuffElemSize);

	// Public query API
	inline const char* GetShaderName() const { return className_; }	

private:
    void InitializeShaders(
        ID3D11Device* pDevice,
        const char* vsFilePath,
        const char* psFilePath);

private:
	VertexShader vs_;
	PixelShader  ps_;

	char className_[32]{"TextureShader"};
};

}  // namespace Render
