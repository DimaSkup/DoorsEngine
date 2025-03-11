#pragma once

#include "VertexShader.h"
#include "GeometryShader.h"
#include "PixelShader.h"
#include "SamplerState.h"
#include "../Common/ConstBufferTypes.h"
#include "../Common/RenderTypes.h"

#include <d3d11.h>

namespace Render
{

class BillboardShader final
{
	using SRV = ID3D11ShaderResourceView;

public:
	BillboardShader();
	~BillboardShader();

	// restrict a copying of this class instance
	BillboardShader(const BillboardShader& obj) = delete;
	BillboardShader& operator=(const BillboardShader& obj) = delete;

	// Public modification API
	bool Initialize(ID3D11Device* pDevice, const std::string& pathToShadersDir);

	void UpdateInstancedBuffer(
		ID3D11DeviceContext* pContext,
		const Material* materials,
		const DirectX::XMFLOAT3* positions,   // positions in world space
		const DirectX::XMFLOAT2* sizes,       // sizes of billboards
		const int count);                     // the number of billboards to render

	void Render(
		ID3D11DeviceContext* pContext,
		ID3D11Buffer* pBillboardVB,
		ID3D11Buffer* pBillboardIB,
		SRV* const* ppTextureArrSRV,
		const UINT stride,            // vertex stride
		const DirectX::XMFLOAT3 position);

	// Public rendering API
	void Render(ID3D11DeviceContext* pContext, const Instance& instance);

	void Shutdown();

	// Public query API
	inline const std::string& GetShaderName() const { return className_; }

private:
	void InitializeShaders(
		ID3D11Device* pDevice,
		const std::string& vsFilename,
		const std::string& gsFilename,
		const std::string& psFilename);

private:
	VertexShader        vs_;
	GeometryShader      gs_;
	PixelShader         ps_;
	SamplerState        samplerState_;

	ID3D11Buffer*                                   pInstancedBuffer_ = nullptr;
	std::vector<BuffTypes::InstancedDataBillboards> instancedData_;

	const std::string className_{ "billboard_shader" };
	const int numMaxInstances_ = 500;                     // limit of instances
	int numCurrentInstances_ = 0;                         // how many instances will we render for this frame?
};

}  // namespace Render