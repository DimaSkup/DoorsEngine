// =================================================================================
// Filename:    ui_materials_browser.cpp
//
// Created:     28.04.2025 by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include "ui_materials_browser.h"
#include <UICommon/gui_states.h>
#include <UICommon/IFacadeEngineToUI.h>
#include <win_file_dialog.h>               // for using OS dialog window to load file from a file
#include <UICommon/IFacadeEngineToUI.h>
#include <imgui.h>


#pragma warning (disable : 4996)


namespace UI
{

//---------------------------------------------------------
//---------------------------------------------------------
void UIMaterialsBrowser::Init(IFacadeEngineToUI* pFacade)
{
    if (!pFacade)
    {
        LogErr(LOG, "can't init materials assets browser: input ptr to UI facade == nullptr");
        return;
    }
    pFacade_ = pFacade;

    const size numMaterials = pFacade->GetNumMaterials();
    numItems_ = (int)numMaterials + 1;

    // init frame buffers so we will be able to render material icons into it
    pFacade->InitMaterialsIcons(numItems_, (int)iconSize_, (int)iconSize_);

    // and immediately render material icons so we won't re-render it each frame
    pFacade->RenderMaterialsIcons();

    pFacade_->GetShadersIdsAndNames(shadersIds_, shadersNames_);
}

//---------------------------------------------------------
// Desc:  render a browser (window) of loaded materials
//---------------------------------------------------------
void UIMaterialsBrowser::Render(IFacadeEngineToUI* pFacade, bool* pOpen)
{
    if (!pFacade)
    {
        LogErr(LOG, "can't render materials browser: input ptr to facade == nullptr");
        return;
    }
    pFacade_ = pFacade;

    ImGui::SetNextWindowContentSize(ImVec2(0, layoutOuterPadding_ + numLayoutLine_ * (layoutItemSizeY_ + layoutItemSpacing_)));
    RenderMenuBar(pOpen);

    // ------------------------------------------

    const float textLineHeight = ImGui::GetTextLineHeightWithSpacing();

    if (ImGui::BeginChild("MaterialAssets", ImVec2(0.0f, -textLineHeight), ImGuiChildFlags_None, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground))
    {
        const int availWidth = (int)ImGui::GetContentRegionAvail().x;

        // if we changed the width of the browser's window
        if ((prevAvailWidth_ != availWidth) || g_GuiStates.needUpdateMatBrowserIcons)
        {
            UpdateLayoutSizes(availWidth);
            prevAvailWidth_ = availWidth;
            g_GuiStates.needUpdateMatBrowserIcons = false;

            // re-init and re-render materials icons since we have some change
            pFacade->InitMaterialsIcons(numItems_, (int)iconSize_, (int)iconSize_);
            pFacade->RenderMaterialsIcons();
        }

        if (updateSelectedMatData_)
            GetMaterialDataByIdx(selectedMatId_);

        // setup start position for debug info rendering
        ImVec2 startPos = ImGui::GetCursorScreenPos() ;
        ImVec2 padding = { layoutOuterPadding_, layoutOuterPadding_ };

        startPos += padding;
        ImGui::SetCursorScreenPos(startPos);
        RenderDebugInfo(availWidth);

        // setup start position for icons rendering
        constexpr float offsetFromDbgInfo = 3;
        startPos.y += (textLineHeight + offsetFromDbgInfo);

        DrawMaterialIcons(startPos.x, startPos.y);
        ComputeZooming   (startPos.x, startPos.y, availWidth);


        if (showIconContextMenu_)
            ShowMaterialContextMenu();

        if (showMaterialEditorWnd_)
            RenderEditorWnd();

        if (showMaterialDeleteWnd_)
            RenderDeleteWnd();
    }
    ImGui::EndChild();
}

//---------------------------------------------------------
// Desc:   check if some material was changed during the frame
// NOTE:   after this check we set a flag to false (!!!)
//---------------------------------------------------------
bool UIMaterialsBrowser::WasMaterialChanged()
{
    bool tmp = materialWasChanged_;
    materialWasChanged_ = false;
    return tmp;
}

//---------------------------------------------------------
// Desc:   draw a menu bar for the material browser
//---------------------------------------------------------
void UIMaterialsBrowser::RenderMenuBar(bool* pOpen)
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Close browser", nullptr, false, pOpen != nullptr))
            {
                *pOpen = false;
            }
            ImGui::EndMenu();
        }

        if (ImGui::Button("Add"))
        {
            // TODO: open a window for creation of a new material
        }
        if (ImGui::Button("Search"))
        {
            // TODO: add stuff for searching for some particular material
            //       by its ID or name
        }

        ImGui::EndMenuBar();
    }
}

//---------------------------------------------------------
// when we did some manipulations over the browser's window (resized, zoomed)
// we need to update the icons size and all the related stuff
//---------------------------------------------------------
void UIMaterialsBrowser::UpdateLayoutSizes(int availWidth)
{
    layoutItemSpacing_ = (float)iconSpacing_;

    // when not stretching: allow extending into right-most spacing
    if (stretchSpacing_ == false)
        availWidth += (int)floorf(layoutItemSpacing_ * 0.5f);

    layoutItemSizeX_ = floorf(iconSize_);
    layoutItemSizeY_ = layoutItemSizeX_;

    // calc how many icons rows and columns will we have
    numLayoutColumn_ = max((int)(availWidth / (layoutItemSizeX_ + layoutItemSpacing_)), 1);
    numLayoutLine_   = (numItems_ + numLayoutColumn_ - 1) / numLayoutColumn_;

    // when stretching: allocate remaining spacing to more spacing. Round before division, so itemSpacing may be non-integer
    if (stretchSpacing_ && (numLayoutColumn_ > 1))
        layoutItemSpacing_ = floorf(availWidth - layoutItemSizeX_ * numLayoutColumn_) / numLayoutColumn_;

    layoutItemStepX_         = layoutItemSizeX_ + layoutItemSpacing_;
    layoutItemStepY_         = layoutItemSizeY_ + layoutItemSpacing_;
    layoutSelectableSpacing_ = max(floorf(layoutItemSpacing_) - iconHitSpacing_, 0.0f);
    layoutOuterPadding_      = floorf(layoutItemSpacing_ * 0.5f);
}

//---------------------------------------------------------
// Desc:   print some debug info in the upper side of the material browser
//---------------------------------------------------------
void UIMaterialsBrowser::RenderDebugInfo(const int availWidth)
{
    ImGui::Text("ID: %d; ", selectedMatId_);
    ImGui::SameLine();
    ImGui::Text("Name: %s", selectedMaterialName_);
    ImGui::SameLine();
    ImGui::Text("selected item idx: %d; ", selectedItemIdx_);
    ImGui::SameLine();

    ImGui::Text("num items: %d; ", numItems_);
    ImGui::SameLine();

    ImGui::Text("avail width: %d; ", availWidth);
    ImGui::SameLine();

    ImGui::Text("lines: %d; ", numLayoutLine_);
    ImGui::SameLine();

    ImGui::Text("columns: %d; ", numLayoutColumn_);
    ImGui::SameLine();

    ImGui::Text("spacing: %f", layoutItemSpacing_);
}

//---------------------------------------------------------
// Desc:  draw a grid of icons where each icon represents some material
// Args:  - startPos:  a window (children) start position (upper left)
//---------------------------------------------------------
void UIMaterialsBrowser::DrawMaterialIcons(const float startPosX, const float startPosY)
{
    // push specific colors
    ImGui::PushStyleColor(ImGuiCol_Header, { 1,1,1,1 });
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 1,1,1,1 });
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, { 1,1,1,1 });

    const int    columnCount  = numLayoutColumn_;
    const ImVec2 layoutItemSz = { layoutItemSizeX_, layoutItemSizeY_ };
    const ImVec2 iconSize     = { iconSize_, iconSize_ };

    ImGuiListClipper clipper;
    clipper.Begin(numLayoutLine_, layoutItemStepY_);

    while (clipper.Step())
    {
        for (int lineIdx = clipper.DisplayStart; lineIdx < clipper.DisplayEnd; ++lineIdx)
        {
            // compute min/max idxs for the current line of icons
            const int itemMinIdx = lineIdx * columnCount;
            const int itemMaxIdx = min(itemMinIdx + columnCount, numItems_);

            // render a line of textures icons
            for (int itemIdx = itemMinIdx; itemIdx < itemMaxIdx; ++itemIdx)
            {
                ImGui::PushID(itemIdx);

                // position of the current item
                const float itemOffsetX = (itemIdx % columnCount) * layoutItemStepX_;
                const float itemOffsetY = lineIdx * layoutItemStepY_;
                const ImVec2 iconPos    = { startPosX + itemOffsetX, startPosY + itemOffsetY };

                ImGui::SetCursorScreenPos(iconPos);
                ImGui::SetNextItemSelectionUserData(itemIdx);

                // check if we see a material icon
                if (!ImGui::IsRectVisible(layoutItemSz))
                {
                    ImGui::PopID();
                    continue;
                }


                char buf[32]{ '\0' };
                const bool isSelected = (itemIdx == selectedItemIdx_);
                snprintf(buf, 32, "##material_item %d", itemIdx);


                // select a material when click LMB on some icon
                if (ImGui::Selectable(buf, isSelected, ImGuiSelectableFlags_None, layoutItemSz))
                {
                    GetMaterialDataByIdx(itemIdx);
                }

                // select a material when click RMB on some icon and open context menu
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                {
                    GetMaterialDataByIdx(itemIdx);
                    showIconContextMenu_ = true;
                }
               
                // draw texture image icon
                ImGui::SetCursorScreenPos(iconPos);
                ImGui::Image((ImTextureID)pFacade_->materialIcons_[itemIdx], iconSize);

                // draw label (just index)
                const ImVec2 boxMin     = { iconPos.x - 1, iconPos.y - 1 };
                const ImVec2 boxMax     = boxMin + layoutItemSz + ImVec2(2, 2);
                const ImU32  labelColor = ImGui::GetColorU32(ImGuiCol_Text);

                char label[16]{ '\0' };
                snprintf(label, 16, "%d", itemIdx);
                ImDrawList* pDrawList = ImGui::GetWindowDrawList();
                pDrawList->AddText(ImVec2(boxMin.x, boxMax.y - ImGui::GetFontSize()), labelColor, label);
               

                ImGui::PopID();

            } // for elems in row
        } // for 
    } // while 

    clipper.End();
    ImGui::PopStyleColor(3);
}

//---------------------------------------------------------
// Desc:   load data of the material by index
//---------------------------------------------------------
void UIMaterialsBrowser::GetMaterialDataByIdx(const int matIdx)
{
    if ((matIdx < 0) || (matIdx >= numItems_))
    {
        LogErr(LOG, "can't get data for material by idx: %d (is invalid)", matIdx);
        return;
    }

    const MaterialID matId = pFacade_->GetMaterialIdByIdx(matIdx);

    if (matId == INVALID_MATERIAL_ID)
    {
        LogErr(LOG, "can't get material id by index: %d", matIdx);
        return;
    }

    selectedMatId_ = matId;

    selectedItemIdx_ = matIdx;                        // update index so the icon of this material will be shown as selected
    materialWasChanged_ = true;                       // material was changed since the prev frame

    const char* matName = pFacade_->GetMaterialNameById(selectedMatId_);
    if (matName)
    {
        strncpy(selectedMaterialName_, matName, MAX_LEN_MAT_NAME);
    }

    pFacade_->GetMaterialTexIds(
        selectedMatId_,
        matData_.textureIDs);

    pFacade_->GetTexViewsByIds(
        matData_.textureIDs,
        matData_.textures,
        NUM_TEXTURE_TYPES);

    matData_.currRsId  = pFacade_->GetMaterialRndStateId(selectedMatId_, RND_STATES_RASTER);
    matData_.currBsId  = pFacade_->GetMaterialRndStateId(selectedMatId_, RND_STATES_BLEND);
    matData_.currDssId = pFacade_->GetMaterialRndStateId(selectedMatId_, RND_STATES_DEPTH_STENCIL);
}

//---------------------------------------------------------
// Desc:  if we zoom in/out (Ctrl+mouse_wheel when over the material browser),
//        we need to recompute icons sizes and related stuff
//---------------------------------------------------------
void UIMaterialsBrowser::ComputeZooming(
    const float startPosX,
    const float startPosY,
    const int availWidth)
{
    ImGuiIO& io = ImGui::GetIO();

    // Zooming with CTRL+Wheel
    if (ImGui::IsWindowAppearing())
        zoomWheelAccum_ = 0.0f;

    if (ImGui::IsWindowHovered() &&
        io.MouseWheel != 0.0f &&
        ImGui::IsKeyDown(ImGuiMod_Ctrl) &&
        ImGui::IsAnyItemActive() == false)
    {
        zoomWheelAccum_ += io.MouseWheel;

        if (fabsf(zoomWheelAccum_) >= 1.0f)
        {
            // calculate hovered item index from mouse location
            const ImVec2 spacing(layoutItemSpacing_, layoutItemSpacing_);
            const ImVec2 startPos(startPosX, startPosY);
            const ImVec2 itemStep(layoutItemStepX_, layoutItemStepY_);

            const ImVec2 hoveredItemNum = (io.MousePos - startPos + spacing * 0.5f) / itemStep;
            const int    hoveredItemIdx = ((int)hoveredItemNum.y * numLayoutColumn_) + (int)hoveredItemNum.x;

            // zoom
            iconSize_ *= powf(1.1f, zoomWheelAccum_);
            iconSize_ = std::clamp(iconSize_, 16.0f, 512.0f);
            zoomWheelAccum_ -= (int)zoomWheelAccum_;
            g_GuiStates.needUpdateMatBrowserIcons = true;
            UpdateLayoutSizes(availWidth);
            

            // manipulate scroll to that we will land at the same Y location of currently hovered item
            // - calculate next frame position of item under mouse
            // - set new scroll position to be used in next ImGui::BeginChild() call
            float hoveredItemRelPosY = ((float)(hoveredItemIdx / numLayoutColumn_) + fmodf(hoveredItemNum.y, 1.0f)) * itemStep.y;
            hoveredItemRelPosY += ImGui::GetStyle().WindowPadding.y;

            float mouseLocalY = io.MousePos.y - ImGui::GetWindowPos().y;
            ImGui::SetScrollY(hoveredItemRelPosY - mouseLocalY);
        }
    }
}

//---------------------------------------------------------
// Desc:   show a window which is used to edit a chosen material
// NOTE:   we split into half this window (left: preview; right: editing fields)
//---------------------------------------------------------
void UIMaterialsBrowser::RenderEditorWnd()
{
    // always center this window when appearing
    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    const float sizeX   = ImGui::GetMainViewport()->Size.x * 0.5f;
    const float sizeY   = ImGui::GetMainViewport()->Size.y * 0.66f;
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(sizeX, sizeY), ImGuiCond_Appearing);


    if (ImGui::Begin("Material editor", &showMaterialEditorWnd_, ImGuiWindowFlags_NoScrollbar))
    {
        const ImVec2 wndPos     = ImGui::GetWindowPos();
        const ImVec2 availReg   = ImGui::GetContentRegionAvail();
        const float halfWidth   = availReg.x / 2;
        const ImVec2 padding    = { 20.0f, 40.0f };

        const ImVec2 wndPosL    = wndPos + padding;
        const ImVec2 wndPosR    = wndPos + padding + ImVec2(halfWidth, 0);

        const ImVec2 wndSizeL   = { halfWidth - padding.x, availReg.y };
        const ImVec2 wndSizeR   = wndSizeL;


        // render material preview (model + material)
        ImGui::SetNextWindowPos(wndPosL);

        if (ImGui::BeginChild("Material preview", wndSizeL))
        {
            RenderMatPreview();
        }
        ImGui::EndChild();


        // render material properties fields 
        ImGui::SetNextWindowPos(wndPosR);

        if (ImGui::BeginChild("Material props", wndSizeR))
        {
            RenderMatPropsFields();
        } 
        ImGui::EndChild();
    }
    ImGui::End();
}

//---------------------------------------------------------
// Desc:   in editor window of a single material we render some shape to visualize
//         this material (it can be sphere/cube/teapot/etc. with this material)
//---------------------------------------------------------
void UIMaterialsBrowser::RenderMatPreview()
{
    const MaterialID matId = selectedMatId_;

    const ImVec2 wndPos   = ImGui::GetItemRectMin();
    const ImVec2 availReg = ImGui::GetContentRegionAvail();
    const int iAvailRegX  = (int)availReg.x;
    const int iAvailRegY  = (int)availReg.y;
    const int smallerSide = min(iAvailRegX, iAvailRegY);
    const int iIconSize   = smallerSide;
    const float fIconSize = (float)smallerSide;

    if (!pFacade_->InitMaterialBigIcon(iIconSize, iIconSize))
    {
        LogErr(LOG, "can't init a material's big icon");
    }

    ImGui::Text("Edit material:");
    ImGui::Text("ID:   %" PRIu32, matId);
    ImGui::Text("Name: %s", selectedMaterialName_);
    ImGui::Separator();

    // if we selected another material we need to reload its data
    if (matData_.materialId != matId)
        pFacade_->GetMaterialDataById(matId, matData_);

    // material shape (sphere, cube, etc.) rotation
    materialSphereRotationY_ += (pFacade_->deltaTime * rotateMaterialBigIcon_);

    // render material image
    pFacade_->RenderMaterialBigIconById(matId, materialSphereRotationY_);

    const ImVec2 imgSize = { fIconSize, fIconSize };
    ImGui::Image((ImTextureID)pFacade_->pMaterialBigIcon_, imgSize);

    // rotate a preview sphere when mouse is over image and LMB is down
    if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        ImGuiIO& io = ImGui::GetIO();
        materialSphereRotationY_ += (io.MouseDelta.x * pFacade_->deltaTime);
    }
}

//---------------------------------------------------------
// Desc:   draw fields for editing material properties
//---------------------------------------------------------
void UIMaterialsBrowser::RenderMatPropsFields()
{
    ImGui::Checkbox("Rotate preview", &rotateMaterialBigIcon_);

    bool matColorChanged = false;
    ImGui::Text("Ambient color ....................");
    ImGui::SameLine();
    matColorChanged |= ImGui::ColorEdit4("Ambient", matData_.ambient.xyzw, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

    ImGui::Text("Diffuse color ......................");
    ImGui::SameLine();
    matColorChanged |= ImGui::ColorEdit4("Diffuse", matData_.diffuse.xyzw, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

    ImGui::Text("Specular color ...................");
    ImGui::SameLine();
    matColorChanged |= ImGui::ColorEdit4("Specular", matData_.specular.xyzw, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

    ImGui::Text("Reflect color ......................");
    ImGui::SameLine();
    matColorChanged |= ImGui::ColorEdit4("Reflect", matData_.reflect.xyzw, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

    ImGui::Text("Glossiness");
    ImGui::SameLine();
    matColorChanged |= ImGui::DragFloat("##glossiness", &matData_.specular.w, 0.1f, 0.0f, 256.0f);

    if (matColorChanged)
    {
        // if we want change terrain's material
        if (strcmp(matData_.name, "terrain_mat") == 0)
            pFacade_->SetTerrainMaterialColors(
                matData_.ambient,
                matData_.diffuse,
                matData_.specular,
                matData_.reflect);

        else
            pFacade_->SetMaterialColorData(
                matData_.materialId,
                matData_.ambient,
                matData_.diffuse,
                matData_.specular,
                matData_.reflect);
    }
    ImGui::Separator();


    // render selectors for choosing render state params of material
    if (ImGui::TreeNode("Setup render state"))
    {
        DrawRasterStatesSelectors();
        DrawBlendStatesSelectors();
        DrawDepthStencilStatesSelectors();

        ImGui::TreePop();
    }

    DrawShadersSelectors();
    DrawRelatedTextures();


    // apply changes (if we have any)
    if (ImGui::Button("Apply", ImVec2(120, 0)))
    {
        g_GuiStates.needUpdateMatBrowserIcons = true;
        showMaterialEditorWnd_ = false;
    }
    if (ImGui::Button("Cancel", ImVec2(120, 0)))
    {
        // TODO: reset fields

        showMaterialEditorWnd_ = false;
    }
}

//---------------------------------------------------------
// show a modal window which is used to delete a material
//---------------------------------------------------------
void UIMaterialsBrowser::RenderDeleteWnd()
{
    ImGui::OpenPopup("Delete?");

    // always center this window when appearing
    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Delete?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Do you really want to delete currenly selected material?");
        ImGui::Text("ID:   %" PRIu32, selectedMatId_);
        ImGui::Text("Name: %s", selectedMaterialName_);
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            // TODO: actually remove this material from the materials manager
            // TODO: update materials icons after deleting this one

            showMaterialDeleteWnd_ = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            showMaterialDeleteWnd_ = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

//---------------------------------------------------------

void UIMaterialsBrowser::DrawRasterStatesSelectors(void)
{
    if (ImGui::TreeNode("Rasterizer states"))
    {
        const cvector<RenderStateName>& statesNames = *pFacade_->GetRenderStateNames(RND_STATES_RASTER);
        const index currRasterStateId = (index)matData_.currRsId;


        // "custom" rasterizer state: we will be able to manually setup each parameter
        if (currRasterStateId == 1)
        {

        }

        // print a selectable list of render states
        for (index i = 0; i < statesNames.size(); ++i)
        {
            if (ImGui::Selectable(statesNames[i].name, i == currRasterStateId))
            {
                RenderStateSetup params;
                params.matId = matData_.materialId;
                params.rndState = RND_STATES_RASTER;
                params.rndStateId = (int)i;

                pFacade_->SetMaterialRenderState(params);
                matData_.currRsId = (uint)i;
            }
        }
        ImGui::TreePop();
    }
    ImGui::Separator();
}

//---------------------------------------------------------
//---------------------------------------------------------
bool DrawBlendWriteMaskControl(
    const char* label,
    const char* comboId,
    const char** currMask,
    const char** maskNames,
    const int numMasks)
{
    if (!label)
    {
        LogErr(LOG, "empty label");
        return false;
    }
    if (!comboId)
    {
        LogErr(LOG, "empty combo id");
        return false;
    }
    if (!currMask || !(*currMask) || (*currMask[0] == '\0'))
    {
        LogErr(LOG, "empty current factor");
        return false;
    }
    if (!maskNames)
    {
        LogErr(LOG, "empty arr of factors names");
        return false;
    }

    bool changed = false;

    ImGui::TableNextColumn();
    ImGui::Text("Blend render target write mask");

    ImGui::TableNextColumn();
    ImGui::PushItemWidth(-FLT_MIN);

    if (ImGui::BeginCombo("##blend_write_mask", *currMask))
    {
        for (int i = 0; i < numMasks; ++i)
        {
            const bool isSelected = (strcmp(*currMask, maskNames[i]) == 0);
            if (ImGui::Selectable(maskNames[i], isSelected))
            {
                *currMask = maskNames[i];
                changed = true;
            }

            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();

    return changed;
}

//---------------------------------------------------------
//---------------------------------------------------------
bool DrawBlendFactorControl(
    const char* label,
    const char* comboId,
    const char** currFactor,
    const char** factors,
    const int numFactors)
{
    if (!label)
    {
        LogErr(LOG, "empty label");
        return false;
    }
    if (!comboId)
    {
        LogErr(LOG, "empty combo id");
        return false;
    }
    if (!currFactor || !(*currFactor) || (*currFactor[0] == '\0'))
    {
        LogErr(LOG, "empty current factor");
        return false;
    }
    if (!factors)
    {
        LogErr(LOG, "empty arr of factors names");
        return false;
    }

    bool changed = false;

    ImGui::TableNextColumn();
    ImGui::Text(label);

    ImGui::TableNextColumn();
    ImGui::PushItemWidth(-FLT_MIN);   // make widget fill available width in the column

    if (ImGui::BeginCombo(comboId, *currFactor))
    {
        for (int i = 0; i < numFactors; ++i)
        {
            const bool isSelected = (strcmp(*currFactor, factors[i]) == 0);
            if (ImGui::Selectable(factors[i], isSelected))
            {
                *currFactor = factors[i];
                changed = true;
            }

            // set the initial focus when opening the combo
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();

    return changed;
}

//---------------------------------------------------------
//---------------------------------------------------------
bool DrawBlendOpControl(
    const char* label,
    const char* comboId,
    const char** currOp,
    const char** ops,
    const int numOps)
{
    if (!label)
    {
        LogErr(LOG, "empty label");
        return false;
    }
    if (!comboId)
    {
        LogErr(LOG, "empty combo id");
        return false;
    }
    if (!currOp || !(*currOp) || (*currOp[0] == '\0'))
    {
        LogErr(LOG, "empty current operation");
        return false;
    }
    if (!ops)
    {
        LogErr(LOG, "empty arr of operations names");
        return false;
    }

    bool changed = false;

    ImGui::TableNextColumn();
    ImGui::Text(label);

    ImGui::TableNextColumn();
    ImGui::PushItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo(comboId, *currOp))
    {
        for (int i = 0; i < numOps; ++i)
        {
            const bool isSelected = (strcmp(*currOp, ops[i]) == 0);
            if (ImGui::Selectable(ops[i], isSelected))
            {
                *currOp = ops[i];
                changed = true;
            }

            // set the initial focus when opening the combo
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();

    return changed;
}

//---------------------------------------------------------

void UIMaterialsBrowser::DrawBlendStatesSelectors(void)
{
    if (ImGui::TreeNode("Blend states"))
    {
        const BsID currBsId = matData_.currBsId;

        bool alphaToCoverage  = pFacade_->GetBsParamBool(currBsId, UI_BLEND_IS_ALPHA_TO_COVERAGE);
        bool independentBlend = pFacade_->GetBsParamBool(currBsId, UI_BLEND_IS_INDEPENDENT);
        bool blendEnabled     = pFacade_->GetBsParamBool(currBsId, UI_BLEND_IS_ENABLED);

        // if it is a "custom" blend state we will be able to manually setup each parameter
        if (currBsId != 1)
        {
            ImGui::BeginDisabled(true);
        }

        int numBlendOps               = pFacade_->GetNumBlendStateParams(UI_BLEND_OPERATION);
        int numBlendFactors           = pFacade_->GetNumBlendStateParams(UI_BLEND_FACTOR);
        int numBlendWriteMasks        = pFacade_->GetNumBlendStateParams(UI_BLEND_RND_TARGET_WRITE_MASK);

        const char** blendOps         = pFacade_->GetBlendStateParamsNames(UI_BLEND_OPERATION);
        const char** blendFactors     = pFacade_->GetBlendStateParamsNames(UI_BLEND_FACTOR);
        const char** blendWriteMasks  = pFacade_->GetBlendStateParamsNames(UI_BLEND_RND_TARGET_WRITE_MASK);

        // color blend (factor, factor, operation)
        const char* currSrcBlend      = pFacade_->GetBsParamStr(currBsId, UI_BLEND_SRC_BLEND);
        const char* currDstBlend      = pFacade_->GetBsParamStr(currBsId, UI_BLEND_DST_BLEND);
        const char* currBlendOp       = pFacade_->GetBsParamStr(currBsId, UI_BLEND_OP);

        // alpha blend (factor, factor, operation)
        const char* currSrcBlendAlpha = pFacade_->GetBsParamStr(currBsId, UI_BLEND_SRC_BLEND_ALPHA);
        const char* currDstBlendAlpha = pFacade_->GetBsParamStr(currBsId, UI_BLEND_DST_BLEND_ALPHA);
        const char* currBlendOpAlpha  = pFacade_->GetBsParamStr(currBsId, UI_BLEND_OP_ALPHA);

        // render target write mask
        const char* currWriteMask     = pFacade_->GetBsParamStr(currBsId, UI_BLEND_RND_TARGET_WRITE_MASK);

        bool bsModified = false;


        bsModified |= ImGui::Checkbox("Alpha to coverage", &alphaToCoverage);
        bsModified |= ImGui::Checkbox("Independent blend", &independentBlend);
        bsModified |= ImGui::Checkbox("Blend enabled",     &blendEnabled);

        ImGui::NewLine();


        const int             numColumns = 2;
        const ImGuiTableFlags tableFlags = 0;   // or ImGuiTableFlags_SizingFixedFit


        if (ImGui::BeginTable("SetupBlendParams", numColumns, tableFlags))
        {
           
            // 1st row (souce color blend factor)
            bsModified |= DrawBlendFactorControl(
                "Src color blend factor",
                "##src_col_blend_factor",
                &currSrcBlend,
                blendFactors,
                numBlendFactors);

            // 2nd row (destination color blend factor)
            bsModified |= DrawBlendFactorControl(
                "Dst color blend factor",
                "##dst_col_blend_factor",
                &currDstBlend,
                blendFactors,
                numBlendFactors);

            // 3rd row (color blend operation)
            bsModified |= DrawBlendOpControl(
                "Color blend op",
                "##col_blend_op",
                &currBlendOp,
                blendOps,
                numBlendOps);
        
            // 4th row (src alpha blend factor)
            bsModified |= DrawBlendFactorControl(
                "Src alpha blend factor",
                "##src_alpha_blend_factor",
                &currSrcBlendAlpha,
                blendFactors,
                numBlendFactors);

            // 5th row (dst alpha blend factor)
            bsModified |= DrawBlendFactorControl(
                "Dst alpha blend factor",
                "##dst_alpha_blend_factor",
                &currDstBlendAlpha,
                blendFactors,
                numBlendFactors);

            // 6th row (alpha blend operation)
            bsModified |= DrawBlendOpControl(
                "Alpha blend op",
                "##alpha_blend_op",
                &currBlendOpAlpha,
                blendOps,
                numBlendOps);

            // 7th row (blend render target write mask)
            bsModified |= DrawBlendWriteMaskControl(
                "Blend render target write mask",
                "##blend_write_mask",
                &currWriteMask,
                blendWriteMasks,
                numBlendWriteMasks);

            if (bsModified)
            {
                pFacade_->UpdateCustomBlendState(
                    alphaToCoverage,
                    independentBlend,
                    blendEnabled,
                    currSrcBlend,
                    currDstBlend,
                    currBlendOp,
                    currSrcBlendAlpha,
                    currDstBlendAlpha,
                    currBlendOpAlpha,
                    currWriteMask);
            }

            ImGui::EndTable();
        }

        ImGui::NewLine();
        ImGui::Separator();


        // if "custom" blend state
        if (currBsId != 1)
        {
            ImGui::EndDisabled();
        }

        // print a selectable list of render states
        const cvector<RenderStateName>& statesNames = *pFacade_->GetRenderStateNames(RND_STATES_BLEND);

        for (index i = 0; i < statesNames.size(); ++i)
        {
            const bool isSelected = (i == currBsId);
            if (ImGui::Selectable(statesNames[i].name, isSelected))
            {
                RenderStateSetup params;
                params.matId = matData_.materialId;
                params.rndState = RND_STATES_BLEND;
                params.rndStateId = (int)i;

                pFacade_->SetMaterialRenderState(params);
                matData_.currBsId = (uint)i;
            }
        }
        ImGui::TreePop();
    }
    ImGui::Separator();
}

//---------------------------------------------------------

void UIMaterialsBrowser::DrawDepthStencilStatesSelectors(void)
{
    if (ImGui::TreeNode("Depth-stencil states"))
    {
        const cvector<RenderStateName>& statesNames = *pFacade_->GetRenderStateNames(RND_STATES_DEPTH_STENCIL);
        const index         currDepthStencilStateId = (index)matData_.currDssId;


        // "custom" depth-stencil state: we will be able to manually setup each parameter
        if (ImGui::Selectable("custom dss...", false))
        {
            //pFacade_->SetMaterialRenderState(matData_.materialId, (uint32)idx, rndStateType);
            //selectedStateIdx = idx;
        }

        // print a selectable list of render states
        for (index i = 0; i < statesNames.size(); ++i)
        {
            if (ImGui::Selectable(statesNames[i].name, i == currDepthStencilStateId))
            {
                RenderStateSetup params;
                params.matId = matData_.materialId;
                params.rndState = RND_STATES_DEPTH_STENCIL;
                params.rndStateId = (int)i;

                pFacade_->SetMaterialRenderState(params);
                matData_.currDssId = (uint)i;
            }
        }
        ImGui::TreePop();
    }
    ImGui::Separator();
}

//---------------------------------------------------------
// Desc:   draw a selection menu where we can
//         switch rendering shader for the chosen material
//---------------------------------------------------------
void UIMaterialsBrowser::DrawShadersSelectors()
{
    if (ImGui::TreeNode("Select shader"))
    {
        // currently selected shader of currently selected material
        ShaderID& selectedShaderId = matData_.shaderId;

        for (index idx = 0; idx < shadersIds_.size(); ++idx)
        {
            const ShaderID id = shadersIds_[idx];
            const char* name  = shadersNames_[idx].name;

            char str[64]{'\0'};
            snprintf(str, 64, "(id: %" PRIu32 ") %s", (int)id, name);

            if (ImGui::Selectable(str, selectedShaderId == id))
            {
                // try to set a shader for material
                if (pFacade_->SetMaterialShaderId(matData_.materialId, id))
                    selectedShaderId = id;
            }
        }

        ImGui::TreePop();
    }
    ImGui::Separator();
}

//---------------------------------------------------------
// Desc:   draw a list of textures which are bounded to the chosen material
//         (so we will be able to change them)
//---------------------------------------------------------
void UIMaterialsBrowser::DrawRelatedTextures()
{
    // push specific colors for borders around textures icons
    ImGui::PushStyleColor(ImGuiCol_Header, { 1,1,1,1 });
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 1,1,1,1 });
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, { 1,1,1,1 });

    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);

    if (ImGui::TreeNode("Textures"))
    {
        if (ImGui::Button("Add texture"))
        {
            showWndTextureAdd_ = true;
            modifyTexType_ = 0;
        }

        if (showWndTextureAdd_)
            ShowWndAddTexture();

        RenderTexturesTable();
        
        if (showTextureContextMenu_)
            ShowTextureContextMenu();

        if (showWndTextureLoad_)
            LoadTextureFromFile();

        if (showWndTextureUnbind_)
            UnbindTexture();

        ImGui::TreePop();
    }

    ImGui::PopStyleColor(3);
}

//---------------------------------------------------------
// Desc:  render a context menu after pressing RMB over some icon
//---------------------------------------------------------
void UIMaterialsBrowser::ShowMaterialContextMenu()
{
    if (ImGui::BeginPopupContextWindow())
    {
        showMaterialEditorWnd_ = ImGui::MenuItem("Edit", "");
        showMaterialDeleteWnd_ = ImGui::MenuItem("Delete", "");

        // if any option in the context menu was chosed we hide the context menu
        if (showMaterialEditorWnd_ || showMaterialDeleteWnd_)
            showIconContextMenu_ = false;

        ImGui::EndPopup();
    }
}

//---------------------------------------------------------
//---------------------------------------------------------
void UIMaterialsBrowser::ShowWndAddTexture()
{
    ImGui::PushStyleColor(ImGuiCol_Header, { 1,1,1,1 });
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 1,1,1,1 });
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, { 1,1,1,1 });

    if (ImGui::Begin("Add texture", &showWndTextureAdd_, ImGuiWindowFlags_NoScrollbar))
    {
        ImGui::Text("Add a texture of another texture type");

        char buf[32]{ '\0' };
        const char** texTypesNames    = pFacade_->GetTexTypesNames();
        const uint   numTexTypesNames = pFacade_->GetNumTexTypesNames();
        const uint   selectedTexType  = modifyTexType_;

        if (ImGui::TreeNode("Select texture type"))
        {
            for (uint i = 0; i < numTexTypesNames; ++i)
            {
                const char* typeName = texTypesNames[i];
                const bool isSelected = (i == selectedTexType);

                char label[64]{ '\0' };
                sprintf(label, "[slot_%u]: %s", i, typeName);

                if (ImGui::Selectable(label, isSelected, ImGuiSelectableFlags_DontClosePopups))
                    modifyTexType_ = i;
            }

            ImGui::TreePop();
        }
       

        if (ImGui::Button("Load"))
            LoadTextureFromFile();
       
        updateSelectedMatData_ = true;
    }
    ImGui::End();
    ImGui::PopStyleColor(3);
}

//---------------------------------------------------------
// Desc:  render a context menu after pressing RMB over some icon
//---------------------------------------------------------
void UIMaterialsBrowser::ShowTextureContextMenu()
{
    const ImVec2 contextMenuPos(contextMenuPosX_, contextMenuPosY_);
    ImGui::SetNextWindowPos(contextMenuPos);

    if (ImGui::BeginPopupContextWindow())
    {
        showWndTextureSelect_ = ImGui::MenuItem("Select",   "select another texture from loaded");
        showWndTextureLoad_   = ImGui::MenuItem("Load",     "load another texture from file");
        showWndTextureUnbind_ = ImGui::MenuItem("Unbind",   "unbind texture from material");

        // if any option in the context menu was chosed we hide the context menu
        if (showWndTextureSelect_ || showWndTextureLoad_ || showWndTextureUnbind_)
            showTextureContextMenu_ = false;

        ImGui::EndPopup();
    }
}

//---------------------------------------------------------
// Desc:  render a table of 2 columns where:
//        on the left side we have info about texture and
//        on the right side we have responsible texture icon
//---------------------------------------------------------
void UIMaterialsBrowser::RenderTexturesTable()
{
    const char** texTypesNames  = pFacade_->GetTexTypesNames();
    const ImVec2 wndPos         = ImGui::GetItemRectMin();
    const ImVec2 iconSize       = { 96.0f, 96.0f };
    const ImVec2 padding        = { 20.0f, 20.0f };

    if (!texTypesNames)
        return;

    // setup upper-left position of the table
    ImGui::SetNextWindowPos(wndPos + padding);

    constexpr ImGuiTableFlags flags =
        ImGuiTableFlags_Borders |
        ImGuiTableFlags_BordersH |
        ImGuiTableFlags_BordersV |
        ImGuiTableFlags_BordersOuterH |
        ImGuiTableFlags_BordersOuterV |
        ImGuiTableFlags_BordersInnerH |
        ImGuiTableFlags_BordersInnerV |
        ImGuiTableFlags_BordersOuter |
        ImGuiTableFlags_BordersInner;

    //enum ContentsType { CT_Text, CT_FillButton };
    //static int contents_type = CT_Text;

    if (ImGui::BeginTable("table", 2, flags))
    {
        const ImVec2 borderSize = { 2.0f, 2.0f };
        const float minRowHeight = iconSize.y + borderSize.y*4;

        for (index i = 0; i < NUM_TEXTURE_TYPES; ++i)
        {
            const TexID id = matData_.textureIDs[i];
            
            // skip since we don't have any texture by this slot
            if (id == INVALID_TEX_ID)
                continue;


            ImGui::TableNextRow(ImGuiTableRowFlags_None, minRowHeight);

            // first column (texture info)
            ImGui::TableNextColumn();
            ImGui::Text("ID: %" PRIu32, id);
            ImGui::Text("Type: %s (%d)", texTypesNames[i], (int)i);

            // second column (texture icon)
            ImGui::TableNextColumn();
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 1,1,1,1 });
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, borderSize);

            const ID3D11ShaderResourceView* pSRV = matData_.textures[i];
            char imgButtonId[32]{ '\0' };
            sprintf(imgButtonId, "##tex_icon %d", (int)i);
            ImGui::ImageButton(imgButtonId, (ImTextureID)pSRV, iconSize);

            // select a texture when click RMB on some icon and open a context menu
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            {
                // where to show a context menu
                const ImVec2 mousePos = ImGui::GetIO().MousePos;
                contextMenuPosX_ = mousePos.x;
                contextMenuPosY_ = mousePos.y;

                showTextureContextMenu_ = true;
                modifyTexId_            = id;
                modifyTexType_          = (uint)i;
            }

            ImGui::PopStyleColor();
            ImGui::PopStyleVar();          
        }

        ImGui::EndTable();
    }
}

//---------------------------------------------------------
// Desc:  selected another texture for material among
//        the textures which are already loaded into project
//---------------------------------------------------------
void UIMaterialsBrowser::SelectTexture()
{
    assert(0 && "IMPLEMENT ME");
}

//---------------------------------------------------------
// Desc:  load another texture from file and bind it
//        to the selected material at particular texture slot
//---------------------------------------------------------
void UIMaterialsBrowser::LoadTextureFromFile()
{
    // open dialog window to choose another texture, so we will get its path
    std::string texPath;
    DialogWndFileOpen(texPath);

    // if we didn't choose any file and close the dialog window
    if (texPath.empty())
        return;

    const TexID texId = pFacade_->LoadTextureFromFile(texPath.c_str());

    if (texId == 0)
    {
        LogErr(LOG, "can't load a texture from file: %s", texPath.c_str());
        showWndTextureLoad_ = false;
        return;
    }
    pFacade_->SetMaterialTexture(selectedMatId_, texId, modifyTexType_);

    // update icons in material/texture browsers since we changed a texture
    g_GuiStates.needUpdateMatBrowserIcons = true;
    g_GuiStates.needUpdateTexBrowserIcons = true;

    showWndTextureLoad_    = false;
    updateSelectedMatData_ = true;
}

//---------------------------------------------------------
// Desc:  show modal window to unbind a texture from material
//        (but don't delete this texture resource by itself)
//---------------------------------------------------------
void UIMaterialsBrowser::UnbindTexture()
{
    ImGui::OpenPopup("Unbind?");

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));


    if (ImGui::BeginPopupModal("Unbind?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (modifyTexType_ >= NUM_TEXTURE_TYPES)
        {
            LogErr(LOG, "you want to unbind a texture by wrong tex type: %u", (int)modifyTexType_);
            ImGui::EndPopup();
            return;
        }

        const char* texTypesName = pFacade_->GetTexTypeName(modifyTexType_);

        ImGui::Text("Do you really want to unbind this texture?");
        ImGui::Text("ID:   %" PRIu32, modifyTexId_);
        ImGui::Text("Type: %s (%u)", texTypesName, modifyTexType_);
        ImGui::Separator();

        // unbind texture
        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            pFacade_->SetMaterialTexture(selectedMatId_, 0, modifyTexType_);
            showWndTextureUnbind_  = false;
            updateSelectedMatData_ = true;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        // cancel unbinding
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            showWndTextureUnbind_ = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}


} // namespace UI
