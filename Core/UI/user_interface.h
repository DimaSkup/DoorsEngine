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

// forward declaration (pointer use only)
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
    void Init(
        IFacadeEngineToUI* pFacadeEngineToUI,
        const char* dbgFontDataFilepath,
        const char* dbgFontTexName,
        const char* gameFontDataFilepath,
        const char* gameFontTexName,
        const int wndWidth,
        const int wndHeight,
        const UINT videoCardMemory,
        const std::string& videoCardName);

    void Update(const Core::SystemState& systemState);

    void UndoEditorLastEvent();

    // Public rendering API
    void RenderGameUI(Render::CRender& render, Core::SystemState& systemState);

    void RenderEditor  (Core::SystemState& systemState);
    void RenderSceneWnd(Core::SystemState& systemState);
    void RenderAndHandleGizmo(const EntityID selectedEntt, Core::SystemState& sysState);

    
    IFacadeEngineToUI* GetFacade() const;
    bool IsSceneWndHovered()       const;

    // Public modification API
    SentenceID AddConstStr(const char* text, const POINT& drawAt);

    SentenceID AddDynamicStr(
        const char* key,
        const char* text,
        const POINT& drawAt,
        const uint maxStrLen);

    void     SetSelectedEntt(const EntityID id);
    EntityID GetSelectedEntt(void) const;

    // gizmo stuff
    bool IsGizmoHovered   (void) const;
    void SetGizmoOperation(const int op);
    void SetGizmoClicked  (const bool isClicked);
    void UseSnapping      (const bool use);

private:
    void LoadDebugInfoStringFromFile(const char* filepath);

    void HandleDebugConstStr  (const char* buffer);
    void HandleDebugDynamicStr(const char* buffer);

    
    // debug info for the game mode
    void RenderDebugInfo(Render::CRender& render, const Core::SystemState& sysState);

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

//==================================================================================
// inline functions
//==================================================================================

inline IFacadeEngineToUI* UserInterface::GetFacade() const
{
    return pFacadeEngineToUI_;
}

inline bool UserInterface::IsSceneWndHovered() const
{
    return isSceneWndHovered_;
}

inline void UserInterface::SetSelectedEntt(const EntityID id)
{
    editorPanels_.enttEditorController_.SetSelectedEntt(id);
}

inline EntityID UserInterface::GetSelectedEntt() const
{
    return editorPanels_.enttEditorController_.GetSelectedEnttID();
}

// gizmo stuff
inline void UserInterface::SetGizmoOperation(const int op)
{
    g_GuiStates.gizmoOperation = op;
}

inline void UserInterface::SetGizmoClicked(const bool isClicked)
{
    g_GuiStates.isGizmoClicked = isClicked;
}

inline bool UserInterface::IsGizmoHovered() const
{
    return g_GuiStates.isGizmoHovered;
}

inline void UserInterface::UseSnapping(const bool onOff)
{
    g_GuiStates.useSnapping = onOff;
}

} // namespace

