/////////////////////////////////////////////////////////////////////
// Filename:       colorshaderclass.h
// Description:    We use this class to invoke HLSL shaders 
//                 for drawing our 3D models which are on the GPU
//
// Revising:       06.04.22
/////////////////////////////////////////////////////////////////////
#pragma once


#include "VertexShader.h"
#include "PixelShader.h"

#include "../Common/RenderTypes.h"

#include <d3d11.h>
#include <DirectXMath.h>

namespace Render
{

class ColorShaderClass final
{
private:
	struct ConstBuffPerFrame
	{
		DirectX::XMMATRIX viewProj;
	};

	struct InstancedData
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4 color;
	};


public:
	ColorShaderClass();
	~ColorShaderClass();

	// restrict a copying of this class instance
	ColorShaderClass(const ColorShaderClass& obj) = delete;
	ColorShaderClass& operator=(const ColorShaderClass& obj) = delete;


	bool Initialize(
        ID3D11Device* pDevice,
        ID3D11DeviceContext* pContext,
        const std::string& pathToShadersDir);

	void Render(
		ID3D11DeviceContext* pContext,
		ID3D11Buffer* pInstancedBuffer,
		const Instance* instances,
		const int numModels,
		const UINT instancesBuffElemSize);

	inline const std::string& GetShaderName() const { return className_; }


private:
	void InitializeShaders(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const std::string& vsFilename,
		const std::string& psFilename);

private:
	VertexShader   vs_;
	PixelShader    ps_;

	const std::string className_{ "color_shader_class" };
};

} // namespace Render
