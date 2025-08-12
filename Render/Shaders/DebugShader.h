// ********************************************************************************
// Filename:     DebugShader.h
// Description:  is used to show normals, tangens, binormals as color
// Created:      24.11.24
// ********************************************************************************
#pragma once

#include "VertexShader.h"
#include "PixelShader.h"
#include "ConstantBuffer.h"
#include "../Common/RenderTypes.h"

#include <d3d11.h>


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
		int debugType = eDebugState::DBG_SHOW_NORMALS;
	};
};


class DebugShader
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
        const char* vsFilePath,
        const char* psFilePath);

    void ShaderHotReload(
        ID3D11Device* pDevice,
        const char* vsFilename,
        const char* psFilename);

	void Render(
		ID3D11DeviceContext* pContext,
		ID3D11Buffer* pInstancedBuffer,
		const InstanceBatch* instances,
		const int numModels,
		const UINT instancedBuffElemSize);

	// change debug type (show normals, tangents, only diffuse map, etc.)
	void SetDebugType(ID3D11DeviceContext* pContext, const eDebugState state);

    inline int           GetDebugType()                const { return cbpsRareChangedDebug_.data.debugType; }
	inline const char*   GetShaderName()               const { return className_; }
	inline ID3D11Buffer* GetConstBufferPSRareChanged() const { return cbpsRareChangedDebug_.Get(); }

private:
	void InitializeShaders(
		ID3D11Device* pDevice,
		const char* vsFilePath,
        const char* psFilePath);

private:
	VertexShader vs_;
	PixelShader  ps_;                       
	ConstantBuffer<BuffTypesDebug::cbpsRareChanged> cbpsRareChangedDebug_; // cbps - const buffer pixel shader for rare changed stuff

	char className_[32]{"DebugShader"};
};


} // namespace Render
