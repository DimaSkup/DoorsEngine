// =================================================================================
// Filename:     OutlineShader.h
// Description:  a wrapper for the outline shader
// 
// Created:      07.02.25  by DimaSkup
// =================================================================================
#pragma once

#include "VertexShader.h"
#include "PixelShader.h"
#include "../Common/RenderTypes.h"


namespace Render
{

class OutlineShader
{
public:
	OutlineShader();
	~OutlineShader();

	// restrict a copying of this class instance
	OutlineShader(const OutlineShader& obj) = delete;
	OutlineShader& operator=(const OutlineShader& obj) = delete;

	// ----------------------------------------------------

	bool Initialize(
		ID3D11Device* pDevice,
        const char* vsFilePath,
        const char* psFilePath);

	void Render(
		ID3D11DeviceContext* pContext,
		ID3D11Buffer* pInstancedBuffer,
		const Instance* instances,
		const int numInstances,
		const UINT instancesBuffElemSize);

	inline const char* GetShaderName() const { return className_; }

private:
    void InitializeShaders(
        ID3D11Device* pDevice,
        const char* vsFilePath,
        const char* psFilePath);

private:
	VertexShader vs_;
	PixelShader  ps_;
	char className_[32]{"OutlineShader"};
};


} // namespace Render
