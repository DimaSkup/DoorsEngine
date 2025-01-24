////////////////////////////////////////////////////////////////////////////////////////////
// Filename:     UserInterface.h
// Description:  a functional for initialization, 
//               updating and rendering of the UI
// 
// Created:      22.05.23
////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "../Engine/SystemState.h"

#include "UICommon/WndParams.h"
#include "UICommon/IFacadeEngineToUI.h"
#include "UICommon/StatesGUI.h"

#include "Editor/MainMenuBar/MainMenuBar.h"
#include "Editor/Panels/EditorPanels.h"

#include "../UI/Text/TextStore.h"
#include "Shaders/fontshaderclass.h"   // from the Render module



class UserInterface
{
public:
	UserInterface();
	~UserInterface();

private:  // restrict a copying of this class instance
	UserInterface(const UserInterface & obj);    
	UserInterface & operator=(const UserInterface & obj);

public:
	void Initialize(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		IFacadeEngineToUI* pFacadeEngineToUI,
		const std::string & fontDataFilePath,      // a path to file with data about this type of font
		const std::string & fontTextureFilePath,   // a path to texture file for this font
		const UINT windowWidth,
		const UINT windowHeight,
		const UINT videoCardMemory,
		const std::string & videoCardName);        

	// Public modification API
	SentenceID CreateConstStr(
		ID3D11Device* pDevice,
		const std::string& str,             // content
		const POINT& drawAt);               // upper left position of the text in the window

	SentenceID CreateDynamicStr(
		ID3D11Device* pDevice,
		const std::string& str,
		const POINT& drawAt,
		const int maxStrSize);              // max possible length for this string


	void Update(
		ID3D11DeviceContext* pContext,
		const SystemState & systemState);


	// Public rendering API
	void Render(
		ID3D11DeviceContext* pContext, 
		Render::FontShaderClass& fontShader,
		SystemState& systemState);


	void ComputeEditorPanelsPosAndSizes();

private:
	void CreateDebugInfoStrings(
		ID3D11Device* pDevice,
		const std::string& videoCardName,
		const int videoCardMemory);

	void CreateFpsFrameTimeStrings(ID3D11Device* pDevice);
	void CreatePositionInfoStrings(ID3D11Device* pDevice);
	void CreateRotationInfoStrings(ID3D11Device* pDevice);
	void CreateRenderInfoStrings(ID3D11Device* pDevice);

	void RenderEditor(SystemState& systemState);
	void RenderGameScenePanel();
	void RenderBottomPanel(const WndParams& bottomPanel);
	void RenderRightPanelUpperHalf(const SystemState& systemState);
	void RenderRightPanelBottomHalf();

	// debug info for the game mode
	void RenderDebugInfo(
		ID3D11DeviceContext* pContext,
		Render::FontShaderClass& fontShader,
		const SystemState& sysState);

	void ComputePosOnScreen(const POINT& drawAt, DirectX::XMFLOAT2& outDrawAtPos);

private:
	UINT                    windowWidth_ = 800;
	UINT                    windowHeight_ = 600;
	StatesGUI               guiStates_;

	FontClass               font1_;        // a font class object (represents a font style)
	TextStore               textStorage_;  // constains strings with debug data: fps, position/rotation, etc.
	
	// editor main parts
	MainMenuBar             editorMainMenuBar_;
	EditorPanels            editorPanels_;
	

	// a facade interface which are used by UI to contact with some engine's parts 
	IFacadeEngineToUI*      pFacadeEngineToUI_ = nullptr;

	// defines if we need to recompute GUI elements positions/sizes for the next frame
	bool                    isNeedToRecomputeGUI_ = true;
};