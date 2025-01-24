// ================================================================================================
// Filename:      ConstBufferTypes.h
// Description:   contains declarations of constant buffer types for shaders;
// 
// Created:       28.11.24
// ================================================================================================
#pragma once
#include <DirectXMath.h>
#include "MaterialLightTypes.h"

namespace Render
{

namespace BuffTypes
{
	struct InstancedData
	{
		DirectX::XMMATRIX  world;
		DirectX::XMMATRIX  worldInvTranspose;
		DirectX::XMMATRIX  texTransform;
		Material           material;
	};

	// ----------------------------------------------------

	struct cbvsPerFrame
	{
		// a structure for vertex shader data which is changed each frame
		DirectX::XMMATRIX  viewProj;
	};

	// ----------------------------------------------------

	struct cbpsPerFrame
	{
		// a structure for pixel shader data which is changed each frame

		DirLight           dirLights[3];
		PointLight         pointLights[25];
		SpotLight          spotLights;
		DirectX::XMFLOAT3  cameraPos;
		int                currNumPointLights = 0;
	};

	// ----------------------------------------------------

	struct cbpsRareChanged
	{
		// a structure for pixel shader data which is rarely changed

		DirectX::XMFLOAT3  fogColor = { 0.5f, 0.5f, 0.5f };
		float              fogStart = 15.0f;               // how far from camera the fog starts?
		float              fogRange = 250.0f;              // how far from camera the object is fully fogged?

		int                numOfDirLights = 1;           // current number of directional light sources

		int                fogEnabled = true;        // turn on/off the fog effect     
		int                turnOnFlashLight = false;       // turn on/off the flashlight
		int                alphaClipping = false;       // do we use alpha clipping?
	};

	// ----------------------------------------------------

	
	struct cbvsPerFrame_SkyDome
	{
		// const buffer data structure for the VS of the sky dome shader
		DirectX::XMMATRIX worldViewProj_ = DirectX::XMMatrixIdentity();
	};

	struct cbpsRareChanged_SkyDome
	{
		// const buffer data structure for the PS of the sky dome shader

		DirectX::XMFLOAT3 colorCenter_{ 1,1,1 };
		float             padding1_ = 1.0f;
		DirectX::XMFLOAT3 colorApex_{ 1,1,1 };
		float             padding2_ = 1.0f;
	};

	// ----------------------------------------------------

	
	struct ConstantMatrixBuffer_FontVS
	{
		// a constant matrix buffer structure for the font vertex shader
		DirectX::XMMATRIX worldViewProj;
	};

	struct ConstantPixelBuffer_FontPS
	{
		// a constant buffer which contains colours are used inside the font pixel shader
		DirectX::XMFLOAT3 pixelColor;         // UI text colour
		float padding;
	};

};


} // namespace Render
