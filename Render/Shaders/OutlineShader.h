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
		const std::string& pathToShadersDir);

	void Render(
		ID3D11DeviceContext* pContext,
		ID3D11Buffer* pInstancedBuffer,
		const Instance* instances,
		const int numInstances,
		const UINT instancesBuffElemSize);

	inline const std::string& GetShaderName() const { return className_; }

private:
	void InitializeShaders(
		ID3D11Device* pDevice,
		const std::string& vsFilePath,
		const std::string& psFilePath);

private:
	VertexShader vs_;
	PixelShader  ps_;

	const std::string className_{ "outline_shader" };
};


} // namespace Render
