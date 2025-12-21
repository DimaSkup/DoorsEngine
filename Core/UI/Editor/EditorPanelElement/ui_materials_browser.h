// =================================================================================
// Filename:    ui_material_browser.h
// Description: a browser of loaded materials
//
// Created:     28.04.2025 by DimaSkup
// =================================================================================
#pragma once

#include <UICommon/mat_data_container.h>

namespace UI
{
// forward declaration
class IFacadeEngineToUI;


class UIMaterialsBrowser
{
public:
    void Init   (IFacadeEngineToUI* pFacade);
    void Render (IFacadeEngineToUI* pFacade, bool* pOpen);

    inline float      GetWndSizeX()      const { return iconSize_ * 15; }
    inline float      GetWndSizeY()      const { return iconSize_ * 15; }
    inline MaterialID GetSelectedMatId() const { return selectedMatId_; }
    bool              WasMaterialChanged();

private:
    void RenderMenuBar       (bool* pOpen);
    void UpdateLayoutSizes   (int availWidth);
    void RenderDebugInfo     (const int availWidth);
    void DrawMaterialIcons   (const float startPosX, const float startPosY);
    void ComputeZooming      (const float startPosX, const float startPosY, const int availWidth);
    void GetMaterialDataByIdx(const int matIdx);

    // render elements of a single material editor
    void RenderEditorWnd();
    void RenderDeleteWnd();

    void RenderMatPreview    ();
    void RenderMatPropsFields();
    void RenderStatesSelectors(
        const char* label,
        const uint matRenderProp,
        index& selectedStateIdx);

    void DrawShadersSelectors();
    void DrawRelatedTextures();

    void ShowMaterialContextMenu();
    void ShowTextureContextMenu();

    void ShowWndAddTexture();
    void RenderTexturesTable();
    void SelectTexture();
    void LoadTextureFromFile();
    void UnbindTexture();

private:
    IFacadeEngineToUI* pFacade_ = nullptr;
    MaterialData matData_;      // material data of chosen material (this data is used for the material editor)

    MaterialID selectedMatId_ = -1;
    char       selectedMaterialName_[MAX_LEN_MAT_NAME]{ '\0' };
    TexID      modifyTexId_   = -1;
    uint       modifyTexType_ = -1;

    float   materialSphereRotationY_ = 0.0f;  // current rotation angle for the material big icon (is used in the material editor)
    float   iconSize_        = 96.0f;
    float   zoomWheelAccum_  = 0.0f;      // Mouse wheel accumulator to handle smooth wheels better
    int     bigIconSize_     = 256;       // size of material's preview image in the editor
    int     prevAvailWidth_  = 0;         // if curr and prev available wnd width aren't equal - we call UpdateLayoutSizes()
    int     iconSpacing_     = 8;
    int     iconHitSpacing_  = 4;         // Increase hit-spacing if you want to make it possible to clear or box-select from gaps. Some spacing is required to able to amend with Shift+box-select. Value is small in Explorer.
    int     numItems_        = 0;
    int     selectedItemIdx_ = -1;

    // Calculated sizes for layout, output of UpdateLayoutSizes(). Could be locals but our code is simpler this way.
    float   layoutItemSizeX_ = 0;
    float   layoutItemSizeY_ = 0;
    float   layoutItemStepX_ = 0;
    float   layoutItemStepY_ = 0;
    float   layoutItemSpacing_       = 0.0f;
    float   layoutSelectableSpacing_ = 0.0f;
    float   layoutOuterPadding_      = 0.0f;
    int     numLayoutColumn_         = 0;
    int     numLayoutLine_           = 0;

    bool    stretchSpacing_         = false;
    bool    updateSelectedMatData_  = false;     // do we need to reload anew data of the selected material?
    bool    materialWasChanged_     = false;
    bool    showMaterialEditorWnd_  = false;     // show a window for editing a single material (opens through contex menu when hit RMB over some icon) 
    bool    showMaterialDeleteWnd_  = false;     // show a window for deleting a single material (opens through contex menu when hit RMB over some icon)
    bool    showIconContextMenu_    = false;
    bool    showTextureContextMenu_ = false;

    bool    showWndTextureAdd_      = false;
    bool    showWndTextureSelect_   = false;
    bool    showWndTextureLoad_     = false;
    bool    showWndTextureUnbind_   = false;
    bool    rotateMaterialBigIcon_  = false;

    float   contextMenuPosX_ = 0;
    float   contextMenuPosY_ = 0;
};

}; // namespace UI
