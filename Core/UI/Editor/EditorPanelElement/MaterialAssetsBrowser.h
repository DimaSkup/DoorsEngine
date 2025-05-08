// =================================================================================
// Filename:    MaterialAssetsBrowser.h
// Description: a browser of loaded materials
//
// Created:     28.04.2025 by DimaSkup
// =================================================================================
#pragma once

#include <UICommon/Types.h>
#include <UICommon/IFacadeEngineToUI.h>
#include <imgui.h>

namespace UI
{

class MaterialAssetsBrowser
{
public:
    MaterialAssetsBrowser() {}

    void Initialize(IFacadeEngineToUI* pFacade);
    void Render    (IFacadeEngineToUI* pFacade, bool* pOpen);

    inline ImVec2     GetBrowserWndSize()     const { return ImVec2(iconSize_ * 25, iconSize_ * 15); }
    inline MaterialID GetSelectedMaterialID() const { return selectedMaterialItemID_; }
    bool              WasMaterialChanged();

private:
    void RenderMenuBar       (bool* pOpen);
    void UpdateLayoutSizes   (int availWidth);
    void RenderDebugInfo     (const int availWidth);
    void DrawMaterialIcons   (const ImVec2 startPos, IFacadeEngineToUI* pFacade);
    void ComputeZooming      (const ImVec2 startPos, const int avaiWidth);
    void GetMaterialDataByIdx(const int matIdx, IFacadeEngineToUI* pFacade);

    void RenderMaterialEditWnd(IFacadeEngineToUI* pFacade);
    void RenderMaterialDeleteWnd();

    void RenderMaterialPreview    (IFacadeEngineToUI* pFacade);
    void RenderMaterialPropsFields(IFacadeEngineToUI* pFacade);

private:
    MaterialData matData_;      // material data of chosen material (this data is used for the material editor)

    MaterialID selectedMaterialItemID_ = -1;
    char       selectedMaterialName_[MAX_LENGTH_MATERIAL_NAME]{ '\0' };

    float   materialSphereRotationY_ = 0.0f;  // current rotation angle for the material big icon (is used in the material editor)
    float   iconSize_        = 96.0f;
    float   zoomWheelAccum_  = 0.0f;      // Mouse wheel accumulator to handle smooth wheels better
    int     prevAvailWidth_  = 0;         // if curr and prev available wnd width aren't equal - we call UpdateLayoutSizes()
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

    bool    stretchSpacing_         = false;
    bool    materialWasChanged_     = false;
    bool    isNeedUpdateIcons_      = false;     // set true when we resizing the browser or zooming in/out
    bool    showMaterialEditorWnd_  = false;     // show a window for editing a single material (opens through contex menu when hit RMB over some icon) 
    bool    showMaterialDeleteWnd_  = false;     // show a window for deleting a single material (opens through contex menu when hit RMB over some icon)
    bool    showIconContextMenu_    = false;
    bool    rotateMaterialBigIcon_  = false;
    cvector<ID3D11ShaderResourceView*> materialsIcons_;
};

}; // namespace UI
