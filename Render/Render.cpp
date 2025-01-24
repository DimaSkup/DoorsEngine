// =================================================================================
// Filename:     Render.cpp
// Description:  there are functions for rendering graphics;
// Created:      01.01.23
// =================================================================================
#include "Render.h"

#include "Common/MathHelper.h"
#include "Common/log.h"
#include "InitRender.h"

using XMFLOAT3 = DirectX::XMFLOAT3;
using XMFLOAT4 = DirectX::XMFLOAT4;
using XMMATRIX = DirectX::XMMATRIX;


namespace Render
{

Render::Render() {}
Render::~Render() {}

// =================================================================================
//                               public methods
// =================================================================================

void Render::SetupLogger(FILE* pFile, std::list<std::string>* pMsgsList)
{
	// setup a file for writing log msgs into it;
	// also setup a list which will be filled with log messages;
	Log::Setup(pFile, pMsgsList);
	Log::Debug("logger is setup successfully");
}

///////////////////////////////////////////////////////////

bool Render::Initialize(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext,
	const InitParams& params) 
{
	try
	{
		bool result = true;
		HRESULT hr = S_OK;
		InitRender init;

		result = init.InitializeShaders(
			pDevice, 
			pContext,
			shadersContainer_,
			params.worldViewOrtho);
		Assert::True(result, "can't initialize shaders");

		// --------------------------------------------

		// create instances buffer
		const UINT maxInstancesNum = 500;
		D3D11_BUFFER_DESC vbd;

		// setup buffer's description
		vbd.Usage               = D3D11_USAGE_DYNAMIC;
		vbd.ByteWidth           = static_cast<UINT>(sizeof(BuffTypes::InstancedData) * maxInstancesNum);
		vbd.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
		vbd.MiscFlags           = 0;
		vbd.StructureByteStride = 0;

		hr = pDevice->CreateBuffer(&vbd, nullptr, &pInstancedBuffer_);
		Assert::NotFailed(hr, "can't create an instanced buffer");


		// ------------------------ CONSTANT BUFFERS ------------------------------ 

		hr = cbvsPerFrame_.Initialize(pDevice, pContext);
		Assert::NotFailed(hr, "can't init a const buffer (VS)");

		hr = cbpsPerFrame_.Initialize(pDevice, pContext);
		Assert::NotFailed(hr, "can't init a const buffer (PS)");

		hr = cbpsRareChanged_.Initialize(pDevice, pContext);
		Assert::NotFailed(hr, "can't init a const buffer (PS)");


		SetFogParams(pContext, params.fogColor, params.fogStart, params.fogRange);


		// load rare changed data (default values if we didn't setup any) into GPU 
		cbpsRareChanged_.ApplyChanges(pContext);

		ColorShaderClass& colorShader   = shadersContainer_.colorShader_;
		LightShaderClass& lightShader   = shadersContainer_.lightShader_;
		SkyDomeShader&    skyDomeShader = shadersContainer_.skyDomeShader_;
		FontShaderClass&  fontShader    = shadersContainer_.fontShader_;
		DebugShader&      debugShader   = shadersContainer_.debugShader_;
		
		// const buffers for vertex shaders
		ID3D11Buffer* vsCBs[3] = 
		{
			cbvsPerFrame_.Get(),                         // slot_0: is common for color/light/debug shader
			skyDomeShader.GetConstBufferVSPerFrame(),    // slot_1: sky dome
			fontShader.GetConstBufferVS(),               // slot_2: font shader
		};	

		// const buffers for pixel shaders
		ID3D11Buffer* psCBs[5] = 
		{ 
			cbpsPerFrame_.Get(),                         // slot_0: light / debug shader
			cbpsRareChanged_.Get(),                      // slot_1: light / debug shader
			skyDomeShader.GetConstBufferPSRareChanged(), // slot_2: sky dome shader
			fontShader.GetConstBufferPS(),               // slot_3: font shader 
			debugShader.GetConstBufferPSRareChanged(),   // slot_4: debug shader
		};

		// bind constant buffers 
		pContext->VSSetConstantBuffers(0, 3, vsCBs);
		pContext->PSSetConstantBuffers(0, 5, psCBs);
	}
	catch (LIB_Exception& e)
	{
		Log::Error(e, true);
		Log::Error("can't initialize the Render module");
		return false;
	}

	return true;
}


// =================================================================================
//                               updating methods
// =================================================================================

void Render::UpdatePerFrame(
	ID3D11DeviceContext* pContext,
	const PerFrameData& data)
{
	// update the common data for shaders for this frame
	// update constant buffers for this frame

	try 
	{
		// view * proj matrix must be already transposed
		cbvsPerFrame_.data.viewProj = data.viewProj;

		// update the camera pos
		cbpsPerFrame_.data.cameraPos = data.cameraPos;

		// ---------------------------------------------

		UpdateLights(
			data.dirLights,
			data.pointLights,
			data.spotLights,
			data.numDirLights,
			data.numPointLights,
			data.numSpotLights);

		// after all we apply updates
		cbvsPerFrame_.ApplyChanges(pContext);
		cbpsPerFrame_.ApplyChanges(pContext);


	}
	catch (LIB_Exception& e)
	{
		Log::Error(e);
	}
	catch (...)
	{
		Log::Error("something went wrong during updating data for rendering");
	}
}

///////////////////////////////////////////////////////////

void Render::UpdateInstancedBuffer(
	ID3D11DeviceContext* pContext,
	const InstBuffData& data)
{
	UpdateInstancedBuffer(
		pContext,
		data.worlds_,
		data.texTransforms_,
		data.materials_,
		data.GetSize());     // get the number of elements to render
}

///////////////////////////////////////////////////////////

void Render::UpdateInstancedBuffer(
	ID3D11DeviceContext* pContext,
	const DirectX::XMMATRIX* worlds,
	const DirectX::XMMATRIX* texTransforms,
	const Material* materials,
	const int count)
{
	//
	// fill in the instanced buffer with data
	//

	try
	{
		// check input data
		Assert::True(worlds && texTransforms && materials, "some of input arrays == nullptr");
		Assert::True(count > 0, "wrong number of input data elements (must be > 0)");

		// map the instanced buffer to write into it
		D3D11_MAPPED_SUBRESOURCE mappedData;
		HRESULT hr = pContext->Map(pInstancedBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
		Assert::NotFailed(hr, "can't map the instanced buffer");

		BuffTypes::InstancedData* dataView = (BuffTypes::InstancedData*)mappedData.pData;

		// write data into the subresource
		for (int i = 0; i < count; ++i)
		{
			dataView[i].world = worlds[i];
			dataView[i].worldInvTranspose = MathHelper::InverseTranspose(worlds[i]);
			dataView[i].texTransform = texTransforms[i];
			dataView[i].material = materials[i];
		}

		pContext->Unmap(pInstancedBuffer_, 0);
	}
	catch (LIB_Exception& e)
	{
		Log::Error(e);
		Log::Error("can't update instanced buffer for rendering");
	}
	catch (...)
	{
		Log::Error("can't update instanced buffer for rendering for some unknown reason :)");
	}
}

///////////////////////////////////////////////////////////

void Render::UpdateInstancedBufferWorlds(
	ID3D11DeviceContext* pContext,
	std::vector<DirectX::XMMATRIX>& worlds)
{
	try
	{
		// map the instanced buffer to write to it
		D3D11_MAPPED_SUBRESOURCE mappedData;
		HRESULT hr = pContext->Map(pInstancedBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
		Assert::NotFailed(hr, "can't map the instanced buffer");

		BuffTypes::InstancedData* dataView = (BuffTypes::InstancedData*)mappedData.pData;

		// write data into the subresource
		for (index i = 0; i < std::ssize(worlds); ++i)
		{
			dataView[i].world = worlds[i];
			dataView[i].worldInvTranspose = MathHelper::InverseTranspose(worlds[i]);
		}

		pContext->Unmap(pInstancedBuffer_, 0);
	}
	catch (LIB_Exception& e)
	{
		Log::Error(e);
		Log::Error("can't update instanced buffer WORLDS for rendering");
	}
	catch (...)
	{
		Log::Error("can't update instanced buffer for rendering for some unknown reason :)");
	}
}

///////////////////////////////////////////////////////////

void Render::UpdateInstancedBufferMaterials(
	ID3D11DeviceContext* pContext,
	std::vector<Material>& materials)
{
	try
	{
		// map the instanced buffer to write to it
		D3D11_MAPPED_SUBRESOURCE mappedData;
		HRESULT hr = pContext->Map(pInstancedBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
		Assert::NotFailed(hr, "can't map the instanced buffer");

		BuffTypes::InstancedData* dataView = (BuffTypes::InstancedData*)mappedData.pData;

		// write data into the subresource
		for (index i = 0; i < std::ssize(materials); ++i)
			dataView[i].material = materials[i];

		pContext->Unmap(pInstancedBuffer_, 0);
	}
	catch (LIB_Exception& e)
	{
		Log::Error(e);
		Log::Error("can't update instanced buffer MATERIALS for rendering");
	}
	catch (...)
	{
		Log::Error("can't update instanced buffer for rendering for some unknown reason :)");
	}
}




// =================================================================================
//                               rendering methods
// =================================================================================

void Render::RenderBoundingLineBoxes(
	ID3D11DeviceContext* pContext,
	const Instance* instances,
	const int numModels)
{
	try
	{
		const UINT instancedBuffElemSize = static_cast<UINT>(sizeof(BuffTypes::InstancedData));

		shadersContainer_.colorShader_.Render(
			pContext,
			pInstancedBuffer_,
			instances,
			numModels,
			instancedBuffElemSize);
	}
	catch (LIB_Exception& e)
	{
		Log::Error(e);
		Log::Error("can't render the bounding line boxes onto the screen");
	}
	catch (...)
	{
		Log::Error("can't render the bounding line boxes for some unknown reason :(");
	}
}

///////////////////////////////////////////////////////////

void Render::RenderInstances(
	ID3D11DeviceContext* pContext,
	const ShaderTypes type,
	const Instance* instances,
	const int numModels)
{
	try
	{
		const UINT instancedBuffElemSize = static_cast<UINT>(sizeof(BuffTypes::InstancedData));

		if (isDebugMode_)
		{
			shadersContainer_.debugShader_.Render(
				pContext,
				pInstancedBuffer_,
				instances,
				numModels,
				instancedBuffElemSize);

			return;
		}

		switch (type)
		{
			case COLOR:
			{
				shadersContainer_.colorShader_.Render(
					pContext,
					pInstancedBuffer_,
					instances,
					numModels,
					instancedBuffElemSize);

				break;
			}
			case LIGHT:
			{
				shadersContainer_.lightShader_.Render(
					pContext,
					pInstancedBuffer_,
					instances,
					numModels,
					instancedBuffElemSize);

				break;
			}
			default:
			{
				Log::Error("unknown shader type: " + std::to_string(type));
			}
		}
	}
	catch (LIB_Exception& e)
	{
		Log::Error(e);
		Log::Error("can't render the mesh instances onto the screen");
	}
	catch (...)
	{
		Log::Error("can't render mesh instances for some unknown reason :(");
	}
}

///////////////////////////////////////////////////////////

void Render::RenderSkyDome(
	ID3D11DeviceContext* pContext,
	const SkyInstance& sky,
	const XMMATRIX& worldViewProj)
{
	try 
	{
		shadersContainer_.skyDomeShader_.Render(
			pContext,
			sky,
			worldViewProj);
	}
	catch (LIB_Exception& e)
	{
		Log::Error(e);
		Log::Error("can't render the sky dome");
	}
	catch (...)
	{
		Log::Error("can't render the sky dome for some unknown reason :(");
	}
}



// =================================================================================
//                              effects control
// =================================================================================

void Render::SwitchFogEffect(ID3D11DeviceContext* pContext, const bool state)
{
	// turn on/off the fog effect
	cbpsRareChanged_.data.fogEnabled = state;
	cbpsRareChanged_.ApplyChanges(pContext);
}

///////////////////////////////////////////////////////////

void Render::SwitchFlashLight(ID3D11DeviceContext* pContext, const bool state)
{
	// turn on/off the flashlight
	cbpsRareChanged_.data.turnOnFlashLight = state;
	cbpsRareChanged_.ApplyChanges(pContext);
}

///////////////////////////////////////////////////////////

void Render::SwitchAlphaClipping(ID3D11DeviceContext* pContext, const bool state)
{
	// turn on/off alpha clipping
	cbpsRareChanged_.data.alphaClipping = state;
	cbpsRareChanged_.ApplyChanges(pContext);
}

///////////////////////////////////////////////////////////

void Render::SwitchDebugState(ID3D11DeviceContext* pContext, const DebugState state)
{
	// turn on/off debug rendering;
	// if we turn on debug rendering we setup the debug state as well;
	switch (state)
	{
		// use default basic shader
		case TURN_OFF:
		{
			isDebugMode_ = false;
			break;
		}
		// use debug shader with particular debug state
		case SHOW_NORMALS:
		case SHOW_TANGENTS:
		case SHOW_BINORMALS:
		case SHOW_BUMPED_NORMALS:
		case SHOW_ONLY_LIGTHING:
		case SHOW_ONLY_DIRECTED_LIGHTING:
		case SHOW_ONLY_POINT_LIGHTING:
		case SHOW_ONLY_SPOT_LIGHTING:
		case SHOW_ONLY_DIFFUSE_MAP:
		case SHOW_ONLY_NORMAL_MAP:
		{
			isDebugMode_ = true;
			shadersContainer_.debugShader_.SetDebugType(pContext, state);
			break;
		}
		default:
		{
			isDebugMode_ = false;
			Log::Error("unknown debug state:" + std::to_string(state));
		}
	}
}

///////////////////////////////////////////////////////////

void Render::SetDirLightsCount(ID3D11DeviceContext* pContext, int numOfLights)
{
	// set how many directional lights we used for lighting of the scene
	// NOTICE: the maximal number of directional light sources is 3

	try
	{
		Assert::True((-1 < numOfLights) || (numOfLights <= 3), "wrong number of dir lights: " + std::to_string(numOfLights));

		cbpsRareChanged_.data.numOfDirLights = numOfLights;
		cbpsRareChanged_.ApplyChanges(pContext);
	}
	catch (LIB_Exception& e)
	{
		Log::Error(e);
	}
}

///////////////////////////////////////////////////////////

void Render::SetFogParams(
	ID3D11DeviceContext* pContext,
	const DirectX::XMFLOAT3& fogColor,
	const float fogStart,
	const float fogRange)
{
	const float start = (fogStart) ? fogStart : 0.0f;
	const float range = (fogRange) ? fogRange : 10.0f;

	// since fog props is changed very rarely we use this separate function to 
	// control various fog params
	cbpsRareChanged_.data.fogColor = fogColor;
	cbpsRareChanged_.data.fogStart = start;
	cbpsRareChanged_.data.fogRange = range;

	cbpsRareChanged_.ApplyChanges(pContext);
}

///////////////////////////////////////////////////////////

void Render::SetSkyGradient(
	ID3D11DeviceContext* pContext,
	const DirectX::XMFLOAT3& colorCenter,
	const DirectX::XMFLOAT3& colorApex)
{
	try
	{
		shadersContainer_.skyDomeShader_.SetSkyGradient(pContext, colorCenter, colorApex);
	}
	catch (LIB_Exception& e)
	{
		Log::Error(e);
		Log::Error("can't update the sky gradient");
	}
}

///////////////////////////////////////////////////////////

void Render::SetSkyColorCenter(
	ID3D11DeviceContext* pContext,
	const DirectX::XMFLOAT3& color)
{
	try
	{
		shadersContainer_.skyDomeShader_.SetSkyColorCenter(pContext, color);
	}
	catch (LIB_Exception& e)
	{
		Log::Error(e);
		Log::Error("can't update the sky center (horizon) color");
	}
}

///////////////////////////////////////////////////////////

void Render::SetSkyColorApex(
	ID3D11DeviceContext* pContext,
	const DirectX::XMFLOAT3& color)
{
	try
	{
		shadersContainer_.skyDomeShader_.SetSkyColorApex(pContext, color);
	}
	catch (LIB_Exception& e)
	{
		Log::Error(e);
		Log::Error("can't update the sky apex (top) color");
	}
}


///////////////////////////////////////////////////////////

void Render::UpdateLights(
	const DirLight* dirLights,
	const PointLight* pointLights,
	const SpotLight* spotLights,
	const int numDirLights,
	const int numPointLights,
	const int numSpotLights)
{
	//
	// load updated light sources data into const buffers
	//

	cbpsPerFrame_.data.currNumPointLights = (int)numPointLights;

	// update directional light sources
	for (int i = 0; i < cbpsRareChanged_.data.numOfDirLights; ++i)
		cbpsPerFrame_.data.dirLights[i] = dirLights[i];

	// we want to copy the proper number of point lights
	const int pointLightsCountLimit = ARRAYSIZE(cbpsPerFrame_.data.pointLights);
	const int pointLightsCount = (numPointLights >= pointLightsCountLimit) ? pointLightsCountLimit : numPointLights;

	// update point light sources
	// NOTICE: a size of the point lights buffer must be >= than the number of actual point light sources
	for (int i = 0; i < pointLightsCount; ++i)
		cbpsPerFrame_.data.pointLights[i] = pointLights[i];

	cbpsPerFrame_.data.spotLights = spotLights[0];
}

}; // namespace Render