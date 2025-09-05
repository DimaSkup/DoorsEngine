// =================================================================================
// Filename:    TextureAssetsBrowser.h
// Description: a browser of loaded textures
//
// Created:     06.04.2025 by DimaSkup
// =================================================================================
#pragma once

#include <UICommon/IFacadeEngineToUI.h>
#include <imgui.h>

namespace UI
{
class TextureAssetsBrowser
{
public:
    TextureAssetsBrowser() {}

    void Initialize(IFacadeEngineToUI* pFacade);
    void Render    (IFacadeEngineToUI* pFacade, bool* pOpen);

    inline ImVec2 GetBrowserWndSize() const { return ImVec2(iconSize_ * 25, iconSize_ * 15); }
    inline TexID  GetSelectedTexID()  const { return selectedTexItemID_; }

    inline bool TexWasChanged()
    {
        // check if texture selection was changed, if so we set the flag to false
        // and return previous state (it is unnecessary to be true)
        bool tmp = textureWasChanged_;
        textureWasChanged_ = false;
        return tmp;
    }

private:
    void RenderMenuBar    (bool* pOpen);
    void UpdateLayoutSizes(const float availWidth);       
    void RenderDebugInfo  (const float availWidth);   
    void DrawTextureIcons (const ImVec2 startPos);
    void ComputeZooming   (const ImVec2 startPos, const float availWidth);
    void ShowContextMenu  ();

    void ShowEditorWnd();

private:
    IFacadeEngineToUI* pFacade_ = nullptr;
    TexID   selectedTexItemID_ = -1;
    TexName selectedTextureName_;

    float   prevAvailWidth_  = 0.0f;      // if curr and prev avail width aren't equal - we call UpdateLayoutSizes()
    float   iconSize_        = 96.0f;
    float   zoomWheelAccum_  = 0.0f;      // Mouse wheel accumulator to handle smooth wheels better
    int     iconSpacing_     = 8;
    int     iconHitSpacing_  = 4;         // Increase hit-spacing if you want to make it possible to clear or box-select from gaps. Some spacing is required to able to amend with Shift+box-select. Value is small in Explorer.
    int     numItems_        = 0;
    int     selectedItemIdx_ = -1;

    // Calculated sizes for layout, output of UpdateLayoutSizes(). Could be locals but our code is simpler this way.
    ImVec2  layoutItemSize_;
    ImVec2  layoutItemStep_;         // == layoutItemSize_ + layoutItemSpacing_
    float   layoutItemSpacing_       = 0.0f;
    float   layoutSelectableSpacing_ = 0.0f;
    float   layoutOuterPadding_      = 0.0f;
    int     numLayoutColumn_         = 0;
    int     numLayoutLine_           = 0;

    bool    stretchSpacing_      = false;
    bool    textureWasChanged_   = false;

    bool    showIconContextMenu_ = false;   // show a context menu when we click RMB over some texture icon
    bool    showEditorWnd_       = false;   // show a window where we edit a chosen texture
    bool    showDeleteWnd_       = false;   // show a window where we can decide to delete a chosen texture
};

} // namespace UI
