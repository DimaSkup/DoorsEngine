////////////////////////////////////////////////////////////////////
// Filename:     InitializeGraphics.h
// Description:  this class is responsible for initialization all the 
//               graphics in the engine;
//
// Created:      02.12.22
////////////////////////////////////////////////////////////////////
#pragma once

#include "../Engine/Settings.h"
#include "../Camera/Camera.h"

#include "../Render/d3dclass.h"
#include "../Texture/TextureMgr.h"

#include "../UI/UserInterface.h"
#include "../Render/Framebuffer.h"
#include "../Model/ModelsCreator.h"

#include "Entity/EntityMgr.h"   // from the ECS module


namespace Core
{

class InitializeGraphics final
{
public:
	InitializeGraphics();

	// initialized all the DirectX stuff
	bool InitializeDirectX(
		D3DClass& d3d,
		HWND hwnd,
		const Settings& settings);

	bool InitializeCameras(
		D3DClass& d3d,
		Camera& gameCamera,
		Camera& editorCamera,
		DirectX::XMMATRIX& baseViewMatrix,      // is used for 2D rendering
		ECS::EntityMgr& enttMgr,
		const Settings& settings);

	bool InitializeScene(
		const Settings& settings,
		D3DClass& d3d,
		ECS::EntityMgr& entityMgr);

	bool InitializeModels(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		ECS::EntityMgr& entityMgr,
		const Settings & settings,
		const float farZ);

#if 0
	// initialize the main wrapper for all of the terrain processing 
	bool InitializeTerrainZone(
		ZoneClass & zone,
		BasicCamera & Camera,
		Settings & settings,
		const float farZ);                            // screen depth
#endif


	bool InitializeSprites(const UINT screenWidth, const UINT screenHeight);
	bool InitializeLightSources(ECS::EntityMgr& mgr, const Settings& settings);

private:  // restrict a copying of this class instance
	InitializeGraphics(const InitializeGraphics & obj);
	InitializeGraphics & operator=(const InitializeGraphics & obj);

};

}