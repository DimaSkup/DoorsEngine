// *********************************************************************************
// Filename:      InitRender.cpp
// Description:   implementation of functional for initialization 
//                of the Render module elements
// 
// Created:       30.08.24
// *********************************************************************************
#include "InitRender.h"
#include "Common/log.h"

namespace Render
{

bool InitRender::InitializeShaders(
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext,
	ShadersContainer& shadersContainer,
	const DirectX::XMMATRIX& WVO)                 // world * base_view * ortho
{
	// initializes all the shader classes (color, texture, light, etc.)
	// and the HLSL shaders as well

	Log::Print();
	Log::Print("---------------------------------------------------------", ConsoleColor::YELLOW);
	Log::Print("                INITIALIZATION: SHADERS                  ", ConsoleColor::YELLOW);
	Log::Print("---------------------------------------------------------", ConsoleColor::YELLOW);
	Log::Debug("shaders initialization: start");

	try
	{
		bool result = false;

		result = shadersContainer.colorShader_.Initialize(pDevice, pContext);
		Assert::True(result, "can't initialize the color shader class");

		result = shadersContainer.textureShader_.Initialize(pDevice, pContext);
		Assert::True(result, "can't initialize the texture shader class");

		result = shadersContainer.lightShader_.Initialize(pDevice, pContext);
		Assert::True(result, "can't initialize the light shader class");


		result = shadersContainer.fontShader_.Initialize(pDevice, pContext, WVO);
		Assert::True(result, "can't initialize the font shader class");

		result = shadersContainer.debugShader_.Initialize(pDevice, pContext);
		Assert::True(result, "can't initialize the debug shader");

		result = shadersContainer.skyDomeShader_.Initialize(pDevice, pContext);
		Assert::True(result, "can't initializer the sky dome shader");

		
		Log::Debug("shaders initialization: finished successfully");
		Log::Print();
	}
	catch (std::bad_alloc& e)
	{
		Log::Error(e.what());
		return false;
	}
	catch (LIB_Exception& e) 
	{
		Log::Error(e, true);
		return false;
	}

	return true;
}

} // namespace Render