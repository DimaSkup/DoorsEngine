// =================================================================================
// Filename:    TextureAssetsBrowser.h
// Description: render browser of loaded textures
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
private:
    IFacadeEngineToUI* pFacade_ = nullptr;
    SRV** arrShaderResourceViews = nullptr;
    size numItems = 0;

    bool            showTypeOverlay = true;
    float           iconSize = 64.0f;
    int             iconSpacing = 10;
    int             iconHitSpacing = 4;         // Increase hit-spacing if you want to make it possible to clear or box-select from gaps. Some spacing is required to able to amend with Shift+box-select. Value is small in Explorer.
    bool            stretchSpacing = true;
    float           zoomWheelAccum = 0.0f;      // Mouse wheel accumulator to handle smooth wheels better


    // Calculated sizes for layout, output of UpdateLayoutSizes(). Could be locals but our code is simpler this way.
    ImVec2          layoutItemSize;
    ImVec2          layoutItemStep;             // == layoutItemSize + layoutItemSpacing
    float           layoutItemSpacing = 0.0f;
    float           layoutSelectableSpacing = 0.0f;
    float           layoutOuterPadding = 0.0f;
    int             layoutColumnCount = 0;
    int             layoutLineCount = 0;

public:
    TextureAssetsBrowser() {}

    void Initialize(IFacadeEngineToUI* pFacade);
    void UpdateLayoutSizes(float availWidth);

    void Render(IFacadeEngineToUI* pFacade);

    constexpr ImVec2 GetWindowSize() { return ImVec2(iconSize * 25, iconSize * 15); }

private:
    void DrawTextureIcons(const ImVec2 startPos);
};

} // namespace UI
