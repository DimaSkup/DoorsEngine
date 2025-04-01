// ********************************************************************************
// Filename:     DebugShader.h
// Description:  is used to show normals, tangens, binormals as color
// Created:      24.11.24
// ********************************************************************************
#pragma once

#include "../Common/RenderTypes.h"

#include "VertexShader.h"
#include "PixelShader.h"
#include "SamplerState.h"         // for using textures sampler
#include "ConstantBuffer.h"

#include <d3d11.h>
#include <DirectXMath.h>

namespace Render
{

//**********************************************************************************
//                DECLARATIONS OF STRUCTURES FOR CONST BUFFERS
//**********************************************************************************

namespace BuffTypesDebug
{
	struct cbpsRareChanged
	{
		// a structure for specific DEBUG PIXEL shader data which is rarely changed
		int debugType = DebugState::SHOW_NORMALS;
	};
};


//**********************************************************************************
// Class name: DebugShader
//**********************************************************************************
class DebugShader final
{
public:
	DebugShader();
	~DebugShader();

	// restrict a copying of this class instance
	DebugShader(const DebugShader& obj) = delete;
	DebugShader& operator=(const DebugShader& obj) = delete;

	// ---------------------------------------------

    bool Initialize(
        ID3D11Device* pDevice,
        ID3D11DeviceContext* pContext,
        const std::string& pathToShadersDir);

	void Render(
		ID3D11DeviceContext* pContext,
		ID3D11Buffer* pInstancedBuffer,
		const Instance* instances,
		const int numModels,
		const UINT instancedBuffElemSize);

	// change debug type (show normals, tangents, only diffuse map, etc.)
	void SetDebugType(ID3D11DeviceContext* pContext, const DebugState state);

	inline const std::string& GetShaderName()          const { return className_; }
	inline ID3D11Buffer* GetConstBufferPSRareChanged() const { return cbpsRareChangedDebug_.Get(); }

private:
	void InitializeShaders(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const std::string& vsFilename,
		const std::string& psFilename);

private:
	VertexShader vs_;
	PixelShader  ps_;                       
	SamplerState samplerState_;                                            // a sampler for texturing

	ConstantBuffer<BuffTypesDebug::cbpsRareChanged> cbpsRareChangedDebug_; // for debugging

	const std::string className_{ "debug_vector_shader" };
};


} // namespace Render
