////////////////////////////////////////////////////////////////////////////////////////////
// Filename:     UserInterface.h
// Description:  a functional for initialization, 
//               updating and rendering of the UI
// 
// Created:      22.05.23
////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <CoreCommon/system_state.h>
#include <UICommon/events_history.h>
#include <UICommon/gui_states.h>

#include "Editor/MainMenuBar/MainMenuBar.h"
#include "Editor/Panels/EditorPanels.h"

#include "../UI/Text/TextStore.h"
#include <Render/CRender.h>

namespace UI
{
// forward declaration
class IFacadeEngineToUI;


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
        IFacadeEngineToUI* pFacadeEngineToUI,
        const char* dbgFontDataFilepath,
        const char* dbgFontTexFilepath,
        const char* gameFontDataFilepath,
        const char* gameFontTexFilepath,
        const int wndWidth,
        const int wndHeight,
        const UINT videoCardMemory,
        const std::string& videoCardName);

	void Update(
		ID3D11DeviceContext* pContext, 
		const Core::SystemState& systemState);

	void UndoEditorLastEvent();

	// Public rendering API
	void RenderGameUI(
		ID3D11DeviceContext* pContext,
		Render::CRender& render,
		Core::SystemState& systemState);

	void RenderEditor(Core::SystemState& systemState);
	void RenderSceneWnd(Core::SystemState& systemState);
    void RenderAndHandleGizmo(const EntityID selectedEntt, Core::SystemState& sysState);

	
    inline IFacadeEngineToUI* GetFacade() const { return pFacadeEngineToUI_; }
	inline bool IsSceneWndHovered()       const { return isSceneWndHovered_; }

	// Public modification API
    SentenceID AddConstStr(
        ID3D11Device* pDevice,
        const char* text,
        const POINT& drawAt);

    SentenceID AddDynamicStr(
        const char* key,
        const char* text,
        const POINT& drawAt,
        const uint maxStrLen);

	inline void SetSelectedEntt(const uint32_t entityID)       { editorPanels_.enttEditorController_.SetSelectedEntt(entityID); }
	inline uint32_t GetSelectedEntt()                    const { return editorPanels_.enttEditorController_.GetSelectedEnttID(); }

	// gizmo stuff
	inline void SetGizmoOperation(const int op)                { g_GuiStates.gizmoOperation = op; }
	inline void SetGizmoClicked(const bool isClicked)          { g_GuiStates.isGizmoClicked = isClicked;}
	inline bool IsGizmoHovered()                         const { return g_GuiStates.isGizmoHovered; }
	inline void UseSnapping(const bool use)                    { g_GuiStates.useSnapping = use;}

private:
	void LoadDebugInfoStringFromFile(
		ID3D11Device* pDevice,
		const std::string& videoCardName,
		const int videoCardMemory);

    void HandleDebugConstStr  (const char* buffer);
    void HandleDebugDynamicStr(const char* buffer);

	
	// debug info for the game mode
	void RenderDebugInfo(
        Render::CRender& render,
		const Core::SystemState& sysState);

	DirectX::XMFLOAT2 ComputePosOnScreen(const POINT& drawAt);

private:
	int                windowWidth_ = 800;
	int                windowHeight_ = 600;


	FontClass          dbgFont01_;        // represents a font style
	FontClass          gameFont01_;       // represents a font style
	TextStore          textStorage_;      // constains strings with debug data: fps, position/rotation, etc.
	
	// editor main parts
	MainMenuBar        editorMainMenuBar_;
	EditorPanels       editorPanels_;

	// a facade interface which are used by UI to contact with some engine's parts 
	IFacadeEngineToUI* pFacadeEngineToUI_ = nullptr;

    ImportModelWndData* pImportModelWndData_ = nullptr;

	bool               isNeedToRecomputeGUI_ = true;     // defines if we need to recompute GUI elements positions/sizes for the next frame
	bool               isSceneWndHovered_ = false;       // is currently scene windows is hovered by mouse
};

} // namespace UI
