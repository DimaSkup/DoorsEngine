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
    void Render(IFacadeEngineToUI* pFacade, bool* pOpen);

    constexpr ImVec2 GetBrowserWndSize() { return ImVec2(iconSize_ * 25, iconSize_ * 15); }

    inline bool TexWasChanged() 
    {
        bool tmp = textureWasChanged_;
        textureWasChanged_ = false;
        return tmp;
    }

    inline TexID GetSelectedTexID() const
    {
        return selectedTexItemID_;
    }

private:
    void UpdateLayoutSizes(float availWidth);
    void RenderDebugInfo(const float availWidth);
    void DrawTextureIcons(const ImVec2 startPos);
    void ComputeZooming(const ImVec2 startPos, const float availWidth);

private:
    IFacadeEngineToUI*  pFacade_ = nullptr;
    SRV**               arrShaderResourceViews_ = nullptr;
    
    TexID               selectedTexItemID_ = -1;

    float               iconSize_ = 96.0f;
    float               zoomWheelAccum_ = 0.0f;      // Mouse wheel accumulator to handle smooth wheels better
    int                 iconSpacing_ = 8;
    int                 iconHitSpacing_ = 4;         // Increase hit-spacing if you want to make it possible to clear or box-select from gaps. Some spacing is required to able to amend with Shift+box-select. Value is small in Explorer.
    int                 numItems_ = 0;
    int                 selectedItemIdx_ = -1;

    // Calculated sizes for layout, output of UpdateLayoutSizes(). Could be locals but our code is simpler this way.
    ImVec2              layoutItemSize_;
    ImVec2              layoutItemStep_;             // == layoutItemSize_ + layoutItemSpacing_
    float               layoutItemSpacing_ = 0.0f;
    float               layoutSelectableSpacing_ = 0.0f;
    float               layoutOuterPadding_ = 0.0f;
    int                 numLayoutColumn_ = 0;
    int                 numLayoutLine_ = 0;

    bool                showTypeOverlay_ = true;
    bool                stretchSpacing_ = false;
    bool                allowBoxSelect_ = true;
    bool textureWasChanged_ = false;
};

} // namespace UI
