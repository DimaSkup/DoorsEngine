// =================================================================================
// Filename:    MaterialAssetsBrowser.cpp
//
// Created:     28.04.2025 by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include "MaterialAssetsBrowser.h"

#pragma warning (disable : 4996)


namespace UI
{

void MaterialAssetsBrowser::Initialize(IFacadeEngineToUI* pFacade)
{
    if (pFacade)
    {

        size numMaterials = 0;
        pFacade->GetNumMaterials(numMaterials);
        numItems_ = (int)numMaterials + 1;

        pFacade->RenderMaterialsIcons(0, numItems_, (int)iconSize_, (int)iconSize_);
    }
}

///////////////////////////////////////////////////////////

void MaterialAssetsBrowser::Render(IFacadeEngineToUI* pFacade, bool* pOpen)
{
    // render a browser (window) of loaded materials

    if (!pFacade)
    {
        LogErr("can't render materials browser: input ptr to facade == nullptr");
        return;
    }

    ImGui::SetNextWindowContentSize(ImVec2(0.0f, layoutOuterPadding_ + numLayoutLine_ * (layoutItemSize_.y + layoutItemSpacing_)));

    RenderMenuBar(pOpen);

    const float textLineHeight = ImGui::GetTextLineHeightWithSpacing();

    if (ImGui::BeginChild("MaterialAssets", ImVec2(0.0f, -textLineHeight), ImGuiChildFlags_None, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground))
    {
        const int availWidth = (int)ImGui::GetContentRegionAvail().x;

        // if we changed the width of the browser's window
        if ((prevAvailWidth_ != availWidth) || isNeedUpdateIcons_)
        {
            UpdateLayoutSizes(availWidth);
            prevAvailWidth_ = availWidth;
            isNeedUpdateIcons_ = false;

            // update icons images
            pFacade->RenderMaterialsIcons(0, numItems_, (int)iconSize_, (int)iconSize_);
        }

        // setup start position for debug info rendering
        ImVec2 startPos = ImGui::GetCursorScreenPos() + ImVec2(layoutOuterPadding_, layoutOuterPadding_);

        ImGui::SetCursorScreenPos(startPos);
        RenderDebugInfo(availWidth);

        // setup start position for icons rendering
        constexpr float offsetFromDbgInfo = 3;
        startPos = ImVec2(startPos.x, startPos.y + textLineHeight + offsetFromDbgInfo);

        DrawMaterialIcons(startPos, pFacade);
        ComputeZooming(startPos, availWidth);

        // context menu when press RMB over some icon
        if (showIconContextMenu_)
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

        if (showMaterialEditorWnd_)
            RenderMaterialEditWnd(pFacade);

        if (showMaterialDeleteWnd_)
            RenderMaterialDeleteWnd();
    }
    ImGui::EndChild();
}

///////////////////////////////////////////////////////////

bool MaterialAssetsBrowser::WasMaterialChanged()
{
    bool tmp = materialWasChanged_;
    materialWasChanged_ = false;
    return tmp;
}

// =================================================================================
// Private methods
// =================================================================================
void MaterialAssetsBrowser::RenderMenuBar(bool* pOpen)
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Close", nullptr, false, pOpen != nullptr))
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

///////////////////////////////////////////////////////////

void MaterialAssetsBrowser::UpdateLayoutSizes(int availWidth)
{
    // when we did some manipulations over the browser's window (resized, zoomed)
    // we need to update the icons size and all the related stuff

    layoutItemSpacing_ = (float)iconSpacing_;

    // when not stretching: allow extending into right-most spacing
    if (stretchSpacing_ == false)
        availWidth += (int)floorf(layoutItemSpacing_ * 0.5f);

    // calculate number of icons per line (columns) and the number of lines
    layoutItemSize_  = ImVec2(floorf(iconSize_), floorf(iconSize_));
    numLayoutColumn_ = max((int)(availWidth / (layoutItemSize_.x + layoutItemSpacing_)), 1);
    numLayoutLine_   = (numItems_ + numLayoutColumn_ - 1) / numLayoutColumn_;

    // when stretching: allocate remaining spacing to more spacing. Round before division, so itemSpacing may be non-integer
    if (stretchSpacing_ && (numLayoutColumn_ > 1))
        layoutItemSpacing_ = floorf(availWidth - layoutItemSize_.x * numLayoutColumn_) / numLayoutColumn_;

    layoutItemStep_          = ImVec2(layoutItemSize_.x + layoutItemSpacing_, layoutItemSize_.y + layoutItemSpacing_);
    layoutSelectableSpacing_ = max(floorf(layoutItemSpacing_) - iconHitSpacing_, 0.0f);
    layoutOuterPadding_      = floorf(layoutItemSpacing_ * 0.5f);
}

///////////////////////////////////////////////////////////

void MaterialAssetsBrowser::RenderDebugInfo(const int availWidth)
{
    ImGui::Text("ID: %d; ", selectedMaterialItemID_);
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

///////////////////////////////////////////////////////////

void MaterialAssetsBrowser::DrawMaterialIcons(const ImVec2 startPos, IFacadeEngineToUI* pFacade)
{
    // push specific colors
    ImGui::PushStyleColor(ImGuiCol_Header, { 1,1,1,1 });
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 1,1,1,1 });
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, { 1,1,1,1 });

    const int columnCount = numLayoutColumn_;

    ImGuiListClipper clipper;
    clipper.Begin(numLayoutLine_, layoutItemStep_.y);

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
                const float itemOffsetX = (itemIdx % columnCount) * layoutItemStep_.x;
                const float itemOffsetY = lineIdx * layoutItemStep_.y;
                const ImVec2 pos        = ImVec2(startPos.x + itemOffsetX, startPos.y + itemOffsetY);

                ImGui::SetCursorScreenPos(pos);
                ImGui::SetNextItemSelectionUserData(itemIdx);

                // if we see material icon -- we will render it
                if (ImGui::IsRectVisible(layoutItemSize_))
                {
                    char buf[32]{ '\0' };
                    const bool isSelected = (itemIdx == selectedItemIdx_);
                    snprintf(buf, 32, "##material_item %d", itemIdx);


                    // select a material when click LMB on some icon
                    if (ImGui::Selectable(buf, isSelected, ImGuiSelectableFlags_None, layoutItemSize_))
                    {
                        GetMaterialDataByIdx(itemIdx, pFacade);
                    }

                    // select a material when click RMB on some icon and open context menu
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) &&
                        ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                    {
                        GetMaterialDataByIdx(itemIdx, pFacade);
                        showIconContextMenu_ = true;
                    }
               
                    // draw texture image icon
                    ImGui::SetCursorScreenPos(pos);
                    ImGui::Image((ImTextureID)pFacade->materialIcons_[itemIdx], { iconSize_, iconSize_ });

                    // draw label (just index)
                    const ImVec2 boxMin(pos.x - 1, pos.y - 1);
                    const ImVec2 boxMax     = boxMin + layoutItemSize_ + ImVec2(2, 2);
                    const ImU32  labelColor = ImGui::GetColorU32(ImGuiCol_Text);
                    char label[16]{ '\0' };

                    snprintf(label, 16, "%d", itemIdx);
                    ImDrawList* pDrawList = ImGui::GetWindowDrawList();
                    pDrawList->AddText(ImVec2(boxMin.x, boxMax.y - ImGui::GetFontSize()), labelColor, label);
                }

                ImGui::PopID();

            } // for elems in line
        } // for 
    } // while 

    clipper.End();
    ImGui::PopStyleColor(3);
}

///////////////////////////////////////////////////////////

void MaterialAssetsBrowser::GetMaterialDataByIdx(const int matIdx, IFacadeEngineToUI* pFacade)
{
    // load data of the material by index

    if ((matIdx < 0) || (matIdx >= numItems_))
    {
        LogErr("can't get data for material by idx: %d (is invalid)", matIdx);
        return;
    }

    if (pFacade->GetMaterialIdByIdx(matIdx, selectedMaterialItemID_))
    {
        selectedItemIdx_ = matIdx;                        // update index so the icon of this material will be shown as selected
        materialWasChanged_ = true;                       // material was changed since the prev frame
        constexpr int maxLen = MAX_LENGTH_MATERIAL_NAME;
        char* name = new char[maxLen] {'\0'};

        pFacade->GetMaterialNameById(selectedMaterialItemID_, &name, maxLen);
        strncpy(selectedMaterialName_, name, maxLen);

        if (name)
        {
            delete[] name;
            name = nullptr;
        }
    }
}

///////////////////////////////////////////////////////////

void MaterialAssetsBrowser::ComputeZooming(const ImVec2 startPos, const int availWidth)
{
    // if we zoom in/out (Ctrl+mouse_wheel when over the browser),
    // we need to recompute icons sizes and related stuff

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
            const ImVec2 hoveredItemNum = (io.MousePos - startPos + spacing * 0.5f) / layoutItemStep_;
            const int hoveredItemIdx = ((int)hoveredItemNum.y * numLayoutColumn_) + (int)hoveredItemNum.x;

            // zoom
            iconSize_ *= powf(1.1f, zoomWheelAccum_);
            iconSize_ = std::clamp(iconSize_, 16.0f, 512.0f);
            zoomWheelAccum_ -= (int)zoomWheelAccum_;
            isNeedUpdateIcons_ = true;
            UpdateLayoutSizes(availWidth);
            

            // manipulate scroll to that we will land at the same Y location of currently hovered item
            // - calculate next frame position of item under mouse
            // - set new scroll position to be used in next ImGui::BeginChild() call
            float hoveredItemRelPosY = ((float)(hoveredItemIdx / numLayoutColumn_) + fmodf(hoveredItemNum.y, 1.0f)) * layoutItemStep_.y;
            hoveredItemRelPosY += ImGui::GetStyle().WindowPadding.y;

            float mouseLocalY = io.MousePos.y - ImGui::GetWindowPos().y;
            ImGui::SetScrollY(hoveredItemRelPosY - mouseLocalY);
        }
    }
}

///////////////////////////////////////////////////////////

void MaterialAssetsBrowser::RenderMaterialEditWnd(IFacadeEngineToUI* pFacade)
{
    // show a window which is used to edit a material

    // always center this window when appearing
    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    const ImVec2 size = ImGui::GetMainViewport()->Size / 2;
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(size, ImGuiCond_Appearing);


    if (ImGui::Begin("Material editor", &showMaterialEditorWnd_))
    {
        const ImVec2 wndPos = ImGui::GetWindowPos();
        const ImVec2 availReg = ImGui::GetContentRegionAvail();
        const float halfWidth = availReg.x / 2;
        constexpr float padding = 40.0f;

        // render material preview sphere
        ImGui::SetNextWindowPos(ImVec2(wndPos.x + padding, wndPos.y + padding));

        if (ImGui::BeginChild("Material preview", ImVec2(halfWidth, availReg.y)))
        {
            RenderMaterialPreview(pFacade);
        }
        ImGui::EndChild();


        // render material properties fields
        ImGui::SetNextWindowPos(ImVec2(wndPos.x + halfWidth, wndPos.y + padding));

        if (ImGui::BeginChild("Material props", ImVec2(halfWidth, availReg.y)))
        {
            RenderMaterialPropsFields(pFacade);
        } 
        ImGui::EndChild();
    }
    ImGui::End();
}

///////////////////////////////////////////////////////////

void MaterialAssetsBrowser::RenderMaterialPreview(IFacadeEngineToUI* pFacade)
{
    ImGui::Text("Edit material:");
    ImGui::Text("ID:   %ld", selectedMaterialItemID_);
    ImGui::Text("Name: %s", selectedMaterialName_);
    ImGui::Separator();

    // if we selected another material we need to reload its data
    if (matData_.id != selectedMaterialItemID_)
        pFacade->GetMaterialDataById(selectedMaterialItemID_, matData_);

    if (rotateMaterialBigIcon_)
        materialSphereRotationY_ += pFacade->deltaTime;

    // render material image (sphere + single material)
    pFacade->RenderMaterialBigIconByID(selectedMaterialItemID_, 256, 256, materialSphereRotationY_);
    ImGui::Image((ImTextureID)pFacade->pMaterialBigIcon_, { 256, 256 });

    // rotate a preview sphere when mouse is over image and LMB is down
    if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        ImGuiIO& io = ImGui::GetIO();
        materialSphereRotationY_ += (io.MouseDelta.x * pFacade->deltaTime);
    }
}

///////////////////////////////////////////////////////////

void MaterialAssetsBrowser::RenderMaterialPropsFields(IFacadeEngineToUI* pFacade)
{
    ImGui::Checkbox("Rotate preview", &rotateMaterialBigIcon_);

    static bool matChanged = false;
    matChanged |= ImGui::DragFloat4("Ambient",        matData_.ambient.xyzw, 0.01f, 0.0f, 1.0f);
    matChanged |= ImGui::DragFloat4("Diffuse",        matData_.diffuse.xyzw, 0.01f, 0.0f, 1.0f);
    matChanged |= ImGui::DragFloat3("Specular",       matData_.specular.xyzw, 0.01f, 0.0f, 1.0f);
    matChanged |= ImGui::DragFloat("Specular power", &matData_.specular.w, 0.1f, 0.0f, 128.0f);
    matChanged |= ImGui::DragFloat4("Reflect",        matData_.reflect.xyzw, 0.01f, 0.0f, 1.0f);

    if (matChanged)
    {
        pFacade->SetMaterialColorData(
            matData_.id,
            matData_.ambient,
            matData_.diffuse,
            matData_.specular,
            matData_.reflect);

        matChanged = false;
    }

    // apply changes (if we have any)
    if (ImGui::Button("Apply", ImVec2(120, 0)))
    {
        matChanged = false;
        isNeedUpdateIcons_ = true;
        showMaterialEditorWnd_ = false;
    }
    if (ImGui::Button("Cancel", ImVec2(120, 0)))
    {
        // TODO: reset fields

        showMaterialEditorWnd_ = false;
        matChanged = false;
    }
}

///////////////////////////////////////////////////////////

void MaterialAssetsBrowser::RenderMaterialDeleteWnd()
{
    // show a modal window which is used to delete a material

    ImGui::OpenPopup("Delete?");

    // always center this window when appearing
    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));


    if (ImGui::BeginPopupModal("Delete?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Do you really want to delete currenly selected material?");
        ImGui::Text("ID:   %ld", selectedMaterialItemID_);
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

} // namespace UI
