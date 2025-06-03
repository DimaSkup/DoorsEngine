// =================================================================================
// Filename:    TextureAssetsBrowser.cpp
// Description: implementation of TextureAssetsBrowser functional
//
// Created:     06.04.2025 by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include "TextureAssetsBrowser.h"

#pragma warning (disable : 4996)

namespace UI
{

void TextureAssetsBrowser::Initialize(IFacadeEngineToUI* pFacade)
{
    if (pFacade)
    {
        // get a pointer to the array of textures shader resource views
        size tmpNumItems = 0;
        pFacade->GetArrTexturesSRVs(arrShaderResourceViews_, tmpNumItems);
        numItems_ = (int)tmpNumItems;
    }
    else
    {
        LogErr("can't init textures browser");
    }
}

///////////////////////////////////////////////////////

void TextureAssetsBrowser::Render(IFacadeEngineToUI* pFacade, bool* pOpen)
{
    // render a browser (window) of loaded textures

    if (!pFacade && !arrShaderResourceViews_)
    {
        LogErr("can't render textures browser");
        return;
    }

    ImGui::SetNextWindowContentSize(ImVec2(0.0f, layoutOuterPadding_ + numLayoutLine_ * (layoutItemSize_.y + layoutItemSpacing_)));
  
    RenderMenuBar(pOpen);

    // ------------------------------------------

    const float textLineHeight = ImGui::GetTextLineHeightWithSpacing();

    if (ImGui::BeginChild("TextureAssets", ImVec2(0.0f, -textLineHeight), ImGuiChildFlags_None, ImGuiWindowFlags_NoMove))
    {
        const float availWidth = ImGui::GetContentRegionAvail().x;

        // if we changed the width of the browser's window
        if (prevAvailWidth_ != availWidth)
        {
            UpdateLayoutSizes(availWidth);
            prevAvailWidth_ = availWidth;
        }

        // setup start position for debug info rendering
        ImVec2 startPos = ImGui::GetCursorScreenPos();
        startPos = ImVec2(startPos.x + layoutOuterPadding_, startPos.y + layoutOuterPadding_);
        ImGui::SetCursorScreenPos(startPos);

        RenderDebugInfo(availWidth);

        // setup start position for icons rendering
        constexpr float offsetFromDbgInfo = 3;
        startPos = ImVec2(startPos.x, startPos.y + textLineHeight + offsetFromDbgInfo);
        DrawTextureIcons(startPos, pFacade);
        ComputeZooming(startPos, availWidth);

        // context menu
        if (ImGui::BeginPopupContextWindow())
        {
            ImGui::Text("TODO: print here a texture ID + name");
            ImGui::EndPopup();
        }
    }
    ImGui::EndChild();
}


// =================================================================================
// Private methods
// =================================================================================
void TextureAssetsBrowser::RenderMenuBar(bool* pOpen)
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Add texture"))
            {
                // TODO: loading a texture from file using UI
            }
            if (ImGui::MenuItem("Close", NULL, false, pOpen != nullptr))
            {
                *pOpen = false;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Add"))
        {
            ImGui::MenuItem("Load");
            ImGui::MenuItem("Generate");

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Update"))
        {
            ImGui::MenuItem("Load");
            ImGui::MenuItem("Set another");
            ImGui::MenuItem("Generate");

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}

///////////////////////////////////////////////////////////

void TextureAssetsBrowser::UpdateLayoutSizes(float availWidth)
{
    // when we did some manipulations over the browser's window (resized, zoomed)
    // we need to update the icons size and all the related stuff

    layoutItemSpacing_ = (float)iconSpacing_;    

    // when not stretching: allow extending into right-most spacing
    if (stretchSpacing_ == false)
        availWidth += floorf(layoutItemSpacing_ * 0.5f);

    // calculate number of icons per line and number of lines
    layoutItemSize_  = ImVec2(floorf(iconSize_), floorf(iconSize_));
    numLayoutColumn_ = max((int)(availWidth / (layoutItemSize_.x + layoutItemSpacing_)), 1);
    numLayoutLine_   = (numItems_ + numLayoutColumn_ - 1) / numLayoutColumn_;

    // when stretching: allocate remaining space to more spacing. Round before division, so itemSpacing may be non-integer
    if (stretchSpacing_ && (numLayoutColumn_ > 1))
        layoutItemSpacing_ = floorf(availWidth - layoutItemSize_.x * numLayoutColumn_) / numLayoutColumn_;

    layoutItemStep_          = ImVec2(layoutItemSize_.x + layoutItemSpacing_, layoutItemSize_.y + layoutItemSpacing_);
    layoutSelectableSpacing_ = max(floorf(layoutItemSpacing_) - iconHitSpacing_, 0.0f);
    layoutOuterPadding_      = floorf(layoutItemSpacing_ * 0.5f);
}

///////////////////////////////////////////////////////

void TextureAssetsBrowser::DrawTextureIcons(const ImVec2 startPos, IFacadeEngineToUI* pFacade)
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
     
                // position of current item
                const float itemOffsetX = (itemIdx % columnCount) * layoutItemStep_.x;
                const float itemOffsetY = lineIdx * layoutItemStep_.y;
                const ImVec2 pos        = ImVec2(startPos.x + itemOffsetX, startPos.y + itemOffsetY);

                ImGui::SetCursorScreenPos(pos);
                ImGui::SetNextItemSelectionUserData(itemIdx);

                // if we see texture icon -- we will render it
                if (ImGui::IsRectVisible(layoutItemSize_))
                {
                    char buf[32]{'\0'};
                    const bool isSelected = (itemIdx == selectedItemIdx_);

                    snprintf(buf, 32, "##texture_item %d", itemIdx);

                    // we can select a texture when click on some icon
                    if (ImGui::Selectable(buf, isSelected, ImGuiSelectableFlags_None, layoutItemSize_))
                    {
                        // try to get a texture ID by index
                        selectedItemIdx_   = itemIdx;
                        textureWasChanged_ = pFacade->GetTextureIdByIdx(itemIdx, selectedTexItemID_);
                    }

                    // draw background
                    const ImVec2 boxMin(pos.x - 1, pos.y - 1);
                    const ImVec2 boxMax(boxMin + layoutItemSize_ + ImVec2(2, 2));
                    const ImU32  iconBgColor = ImGui::GetColorU32({ 0,0,0,1 });

                    ImDrawList* pDrawList = ImGui::GetWindowDrawList();
                    pDrawList->AddRectFilled(boxMin, boxMax, iconBgColor);

                    // draw texture image icon
                    ImGui::SetCursorScreenPos(pos);
                    ImGui::Image((ImTextureID)arrShaderResourceViews_[itemIdx], { iconSize_, iconSize_ });

                    // draw label (just index)
                    const ImU32 labelColor = ImGui::GetColorU32(ImGuiCol_Text);
                    char label[16]{'\0'};

                    snprintf(label, 16, "%d", itemIdx);
                    pDrawList->AddText(ImVec2(boxMin.x, boxMax.y - ImGui::GetFontSize()), labelColor, label);
                }

                ImGui::PopID();
            }
        }
    }

    clipper.End();
    ImGui::PopStyleColor(3);
}

///////////////////////////////////////////////////////

void TextureAssetsBrowser::RenderDebugInfo(const float availWidth)
{
    ImGui::Text("texture ID: %d;", selectedTexItemID_);
    ImGui::SameLine();
    ImGui::Text("selected item idx: %d;", selectedItemIdx_);
    ImGui::SameLine();

    ImGui::Text("num_items: %d;", numItems_);
    ImGui::SameLine();

    ImGui::Text("avail width: %f;", availWidth);
    ImGui::SameLine();

    ImGui::Text("lines: %d;", numLayoutLine_);
    ImGui::SameLine();

    ImGui::Text("columns: %d;", numLayoutColumn_);
    ImGui::SameLine();

    ImGui::Text("spacing: %f", layoutItemSpacing_);
}

///////////////////////////////////////////////////////

void TextureAssetsBrowser::ComputeZooming(const ImVec2 startPos, const float availWidth)
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
            const float hoveredItemNX = (io.MousePos.x - startPos.x + layoutItemSpacing_ * 0.5f) / layoutItemStep_.x;
            const float hoveredItemNY = (io.MousePos.y - startPos.y + layoutItemSpacing_ * 0.5f) / layoutItemStep_.y;
            const int hoveredItemIdx = ((int)hoveredItemNY * numLayoutColumn_) + (int)hoveredItemNX;

            // zoom
            iconSize_ *= powf(1.1f, zoomWheelAccum_);
            iconSize_ = std::clamp(iconSize_, 16.0f, 512.0f);
            zoomWheelAccum_ -= (int)zoomWheelAccum_;
            UpdateLayoutSizes(availWidth);

            // manipulate scroll to that we will land at the same Y location of currently hovered item
            // - calculate next frame position of item under mouse
            // - set new scroll position to be used in next ImGui::BeginChild() call
            float hoveredItemRelPosY = ((float)(hoveredItemIdx / numLayoutColumn_) + fmodf(hoveredItemNY, 1.0f)) * layoutItemStep_.y;
            hoveredItemRelPosY += ImGui::GetStyle().WindowPadding.y;

            float mouseLocalY = io.MousePos.y - ImGui::GetWindowPos().y;
            ImGui::SetScrollY(hoveredItemRelPosY - mouseLocalY);
        }
    }
}

} // namespace UI
