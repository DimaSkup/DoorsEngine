// *********************************************************************************
// Filename:      InitRender.h
// Description:   functional for initialization of the Render module elements
// 
// Created:       30.08.24
// *********************************************************************************
#pragma once

#include "Shaders/ShadersContainer.h"


namespace Render
{

class InitRender final
{
public:
	InitRender() {};

	// initialize all the shaders (color, texture, light, etc.)
	bool InitializeShaders(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pDeviceContext,
		ShadersContainer& shadersContainer,
		const DirectX::XMMATRIX& WVO);
};


} // namespace Render