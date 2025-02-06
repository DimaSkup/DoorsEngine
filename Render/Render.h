// =================================================================================
// Filename:     Render.h
// Description:  this class is responsible for rendering all the 
//               graphics onto the screen;
// 
// Created:      02.12.22 (moved into the Render module at 29.08.24)
// =================================================================================
#pragma once

#include "Common/ConstBufferTypes.h"
#include "Shaders/ShadersContainer.h"
#include "Common/RenderTypes.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <list>


namespace Render
{
	struct InitParams
	{
		// this struct contains init params for the rendering

		// for 2D rendering
		DirectX::XMMATRIX worldViewOrtho = DirectX::XMMatrixIdentity();

		// fog params
		DirectX::XMFLOAT3 fogColor{ 0.5f, 0.5f, 0.5f };
		float             fogStart = 50;                 // a distance where the fog starts
		float             fogRange = 200;                // max distance after which all the objects are fogged
	};

class Render final
{

public:
	Render();
	~Render();

	// restrict a copying of this class instance
	Render(const Render& obj) = delete;
	Render& operator=(const Render& obj) = delete;

	// setup logger file ptr if we want to write logs into the file
	void SetupLogger(FILE* pFile, std::list<std::string>* pMsgsList);

	// initialize the rendering subsystem
	bool Initialize(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const InitParams& params);


	// ================================================================================
	//                                   Updating 
	// ================================================================================
	void UpdatePerFrame(ID3D11DeviceContext* pContext, const PerFrameData& data);

	void UpdateInstancedBuffer(
		ID3D11DeviceContext* pContext,
		const InstBuffData& data);

	void UpdateInstancedBuffer(
		ID3D11DeviceContext* pContext,
		const DirectX::XMMATRIX* worlds,
		const DirectX::XMMATRIX* texTransforms,
		const Material* materials,
		const int count);

	void UpdateInstancedBufferWorlds(
		ID3D11DeviceContext* pContext, 
		std::vector<DirectX::XMMATRIX>& worlds);

	void UpdateInstancedBufferMaterials(
		ID3D11DeviceContext* pContext,
		std::vector<Material>& materials);


	// ================================================================================
	//                                  Rendering
	// ================================================================================

	void RenderBoundingLineBoxes(
		ID3D11DeviceContext* pContext,
		const Instance* instances,
		const int numModels);

	void RenderInstances(
		ID3D11DeviceContext* pContext,
		const ShaderTypes type,
		const Instance* instances,
		const int numModels);

	void RenderSkyDome(
		ID3D11DeviceContext* pContext,
		const SkyInstance& sky,
		const DirectX::XMMATRIX& worldViewProj);




	// ================================================================================
	//                                   Getters
	// ================================================================================
	inline ShadersContainer& GetShadersContainer() { return shadersContainer_; }
	inline LightShaderClass& GetLightShader()      { return shadersContainer_.lightShader_; }

	inline void GetFogData(DirectX::XMFLOAT3& color, float& start, float& range)
	{
		color = cbpsRareChanged_.data.fogColor;
		start = cbpsRareChanged_.data.fogStart;
		range = cbpsRareChanged_.data.fogRange;
	}

	// ================================================================================
	//                                   Setters
	// ================================================================================
	void SwitchFogEffect(ID3D11DeviceContext* pContext, const bool state);
	void SwitchFlashLight(ID3D11DeviceContext* pContext);

	void SwitchAlphaClipping(ID3D11DeviceContext* pContext, const bool state);
	void SwitchDebugState(ID3D11DeviceContext* pContext, const DebugState state);
	void SetDirLightsCount(ID3D11DeviceContext* pContext, int numOfLights);

	void SetFogParams(
		ID3D11DeviceContext* pContext,
		const DirectX::XMFLOAT3& fogColor,
		const float fogStart,
		const float fogRange);

	void SetSkyGradient(
		ID3D11DeviceContext* pContext,
		const DirectX::XMFLOAT3& colorCenter,
		const DirectX::XMFLOAT3& colorApex);

	void SetSkyColorCenter(ID3D11DeviceContext* pContext, const DirectX::XMFLOAT3& color);
	void SetSkyColorApex  (ID3D11DeviceContext* pContext, const DirectX::XMFLOAT3& color);



private:
	void UpdateLights(
		const DirLight* dirLights,
		const PointLight* pointLights,
		const SpotLight* spotLights,
		const int numDirLights,
		const int numPointLights,
		const int numSpotLights);

public:
	ID3D11Buffer*                              pInstancedBuffer_ = nullptr;
	std::vector<BuffTypes::InstancedData>      instancedData_;    // instances common buffer

	ConstantBuffer<BuffTypes::cbvsPerFrame>    cbvsPerFrame_;     // for vertex shader 
	ConstantBuffer<BuffTypes::cbpsPerFrame>    cbpsPerFrame_;     // for pixel shader
	ConstantBuffer<BuffTypes::cbpsRareChanged> cbpsRareChanged_;  // for pixel shader

	RenderDataStorage dataStorage_;
	PerFrameData      perFrameData_;                              // we need to keep this data because we use it multiple times during the frame
	ShadersContainer  shadersContainer_;                          // a struct with shader classes objects

	bool              isDebugMode_ = false;                       // do we use the debug shader?
};

}; // namespace Render