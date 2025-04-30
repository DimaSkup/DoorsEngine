// =================================================================================
// Filename:    MaterialAssetsBrowser.cpp
//
// Created:     28.04.2025 by DimaSkup
// =================================================================================
#include "MaterialAssetsBrowser.h"
#include <CoreCommon/log.h>


#pragma warning (disable : 4996)


namespace UI
{

void MaterialAssetsBrowser::Initialize(IFacadeEngineToUI* pFacade)
{
    if (pFacade)
    {
        // get a pointer to the array of framebuffers (which contains images of material: sphere model + material)
        //size numItems = 0;
        //pFacade->GetArrMaterialsSRVs(materialsIcons_.data(), materialsIcons_.size())

        size numMaterials = 0;
        pFacade->GetNumMaterials(numMaterials);
        numItems_ = (int)numMaterials;

        pFacade->RenderMaterialsIcons(0, numItems_, iconSize_, iconSize_);
    }
}

///////////////////////////////////////////////////////////

void MaterialAssetsBrowser::Render(IFacadeEngineToUI* pFacade, bool* pOpen)
{
    // render a browser (window) of loaded materials

    if (!pFacade)
    {
        Core::LogErr("can't render materials browser: input ptr to facade == nullptr");
        return;
    }

    ImGui::SetNextWindowContentSize(ImVec2(0.0f, layoutOuterPadding_ + numLayoutLine_ * (layoutItemSize_.y + layoutItemSpacing_)));

    RenderMenuBar(pOpen);

    // ------------------------------------------

    const float textLineHeight = ImGui::GetTextLineHeightWithSpacing();

    if (ImGui::BeginChild("MaterialAssets", ImVec2(0.0f, -textLineHeight), ImGuiChildFlags_None, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground))
    {
        const int availWidth = (int)ImGui::GetContentRegionAvail().x;

        // if we changed the width of the browser's window
        if (prevAvailWidth_ != availWidth)
        {
            UpdateLayoutSizes(availWidth);
            prevAvailWidth_ = availWidth;

            // update icons images
            pFacade->RenderMaterialsIcons(0, numItems_, iconSize_, iconSize_);
        }

        // setup start position for debug info rendering
        ImVec2 startPos = ImGui::GetCursorScreenPos();
        startPos = ImVec2(startPos.x + layoutOuterPadding_, startPos.y + layoutOuterPadding_);

        ImGui::SetCursorScreenPos(startPos);
        RenderDebugInfo(availWidth);

        // setup start position for icons rendering
        constexpr float offsetFromDbgInfo = 3;
        startPos = ImVec2(startPos.x, startPos.y + textLineHeight + offsetFromDbgInfo);

        DrawMaterialIcons(startPos, pFacade);
        ComputeZooming(startPos, availWidth);

        // context menu
        if (ImGui::BeginPopupContextWindow())
        {
            ImGui::Text("TODO: print here a material ID + name + operations");
            ImGui::EndPopup();
        }
    }
    ImGui::EndChild();
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
            // TODO: creation of a new material (open a window for it)
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
    ImGui::Text("material ID: %d; ", selectedMaterialItemID_);
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

    const int columnCount         = numLayoutColumn_;
    //const int currSelectedItemIdx = selectedItemIdx_;

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

                    sprintf(buf, "##material_item %d", itemIdx);

                    // we can select a material when click on some icon
                    if (ImGui::Selectable(buf, isSelected, ImGuiSelectableFlags_None, layoutItemSize_))
                    {
                        selectedItemIdx_ = itemIdx;

                        // try to get a material ID by idx
                       //materialWasChanged_ = pFacade->GetMaterialIdByIdx(itemIdx, selectedMaterialItemID_);
                    }
               
                    // draw background
                    const ImVec2 boxMin(pos.x - 1, pos.y - 1);
                    const ImVec2 boxMax = boxMin + layoutItemSize_ + ImVec2(2, 2);
                    const ImU32  iconBgColor = ImGui::GetColorU32({ 0,0,0,0 });

                    ImDrawList* pDrawList = ImGui::GetWindowDrawList();
                    
                   // pDrawList->AddRectFilled(boxMin, boxMax, iconBgColor);
                    //const ImVec2 circleCenter = (boxMin + boxMax) / 2;
                    //const float circleRadius = layoutItemSize_.x / 2 - 2;
                    //pDrawList->AddCircleFilled(circleCenter, circleRadius, iconBgColor);
                    // draw texture image icon
                    ImGui::SetCursorScreenPos(pos);
                    ImGui::Image((ImTextureID)pFacade->materialIcons_[itemIdx], { iconSize_, iconSize_ });

                    // draw label (just index)
                    const ImU32 labelColor = ImGui::GetColorU32(ImGuiCol_Text);
                    char label[16]{ '\0' };

                    sprintf(label, "%d", itemIdx);
                    pDrawList->AddText(ImVec2(boxMin.x, boxMax.y - ImGui::GetFontSize()), labelColor, label);
                }

                ImGui::PopID();
            }
        }
    }

    clipper.End();
    ImGui::PopStyleColor(3);
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
            iconSize_ = std::clamp(iconSize_, 16.0f, 128.0f);
            zoomWheelAccum_ -= (int)zoomWheelAccum_;
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

} // namespace UI
