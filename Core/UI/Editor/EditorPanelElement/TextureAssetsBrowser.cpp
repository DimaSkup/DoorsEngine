// =================================================================================
// Filename:    TextureAssetsBrowser.cpp
// Description: implementation of TextureAssetsBrowser functional
//
// Created:     06.04.2025 by DimaSkup
// =================================================================================
#include "TextureAssetsBrowser.h"
#include <CoreCommon/log.h>

#pragma warning (disable : 4996)

namespace UI
{

void TextureAssetsBrowser::Initialize(IFacadeEngineToUI* pFacade)
{
    if (pFacade)
    {
        pFacade_ = pFacade;
        pFacade->GetArrShaderResourceViews(arrShaderResourceViews, numItems);

       
    }
    else
    {
        Core::Log::Error("can't init textures browser");
    }
}

///////////////////////////////////////////////////////

void TextureAssetsBrowser::UpdateLayoutSizes(float availWidth)
{
    // when not stretching: allow extending into right-most spacing
    layoutItemSpacing = (float)iconSpacing;

    if (stretchSpacing == false)
        availWidth += floorf(layoutItemSpacing * 0.5f);

    // calculate number of icons per line and number of lines
    layoutItemSize    = ImVec2(floorf(iconSize), floorf(iconSize));
    layoutColumnCount = max((int)(availWidth / (layoutItemSize.x + layoutItemSpacing)), 1);
    layoutLineCount   = (numItems + layoutColumnCount - 1) / layoutColumnCount;

    // when stretching: allocate remaining space to more spacing. Round before division, so itemSpacing may be non-integer
    if (stretchSpacing && (layoutColumnCount > 1))
        layoutItemSpacing = floorf(availWidth - layoutItemSize.x * layoutColumnCount) / layoutColumnCount;

    layoutItemStep          = ImVec2(layoutItemSize.x + layoutItemSpacing, layoutItemSize.y + layoutItemSpacing);
    layoutSelectableSpacing = max(floorf(layoutItemSpacing) - iconHitSpacing, 0.0f);
    layoutOuterPadding      = floorf(layoutItemSpacing * 0.5f);
}

///////////////////////////////////////////////////////

void TextureAssetsBrowser::DrawTextureIcons(const ImVec2 startPos)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    const int columnCount = layoutColumnCount;
    ImGuiListClipper clipper;
    clipper.Begin(layoutLineCount, layoutItemStep.y);

    //if (itemCurrIdxToFocus != -1)
    //    clipper.IncludeItemByIndex(itemCurrIdxToFocus / columntCount)

    while (clipper.Step())
    {
        for (int lineIdx = clipper.DisplayStart; lineIdx < clipper.DisplayEnd; ++lineIdx)
        {
            const int itemMinIdxForCurrLine = lineIdx * columnCount;
            const int itemMaxIdxForCurrLine = min((lineIdx + 1) * columnCount, numItems);

            for (int itemIdx = itemMinIdxForCurrLine; itemIdx < itemMaxIdxForCurrLine; ++itemIdx)
            {
                ImGui::PushID(std::string("texture_item" + std::to_string(itemIdx)).c_str());

                // position item
                const float itemOffsetX = (itemIdx % columnCount) * layoutItemStep.x;
                const float itemOffsetY = lineIdx * layoutItemStep.y;
                ImVec2 pos = ImVec2(startPos.x + itemOffsetX, startPos.y + itemOffsetY);
                ImGui::SetCursorScreenPos(pos);

                ImGui::SetNextItemSelectionUserData(itemIdx);
                bool itemIsVisible = ImGui::IsRectVisible(layoutItemSize);

                // show texture image
                if (itemIsVisible)
                {
                    ImVec2 boxMin(pos.x - 1, pos.y - 1);
                    ImVec2 boxMax(boxMin.x + layoutItemSize.x + 2, boxMin.y + layoutItemSize.y + 2);
                    ImU32 bgColor = ImGui::GetColorU32({ 0.5f, 0.5f, 0.5f, 1.0f });
                    draw_list->AddRectFilled(boxMin, boxMax, bgColor);
                    ImGui::Image((ImTextureID)arrShaderResourceViews[itemIdx], { iconSize, iconSize });

                    // display label
                    ImU32 labelColor = ImGui::GetColorU32(ImGuiCol_Text);
                    char label[32];
                    sprintf(label, "%d", itemIdx);
                    draw_list->AddText(ImVec2(boxMin.x, boxMax.y - ImGui::GetFontSize()), labelColor, label);
                }


                ImGui::PopID();
            }
        }
    }

    clipper.End();
}


///////////////////////////////////////////////////////

void TextureAssetsBrowser::Render(IFacadeEngineToUI* pFacade)
{
    if (!pFacade && !arrShaderResourceViews)
    {
        Core::Log::Error("can't render textures browser");
        return;
    }

  
    // child window: assets icons
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowContentSize(ImVec2(0.0f, layoutOuterPadding + layoutLineCount * (layoutItemSize.y + layoutItemSpacing)));
    if (ImGui::BeginChild("Assets", ImVec2(0.0f, -ImGui::GetTextLineHeightWithSpacing()), ImGuiChildFlags_Borders, ImGuiWindowFlags_NoMove))
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        const float availWidth = ImGui::GetContentRegionAvail().x;
        UpdateLayoutSizes(availWidth);

        ImGui::Text("num_items: %d", numItems);
        ImGui::SameLine();
        ImGui::Text("width: %f", availWidth);
        ImGui::SameLine();
        ImGui::Text("lines: %d", layoutLineCount);
        ImGui::SameLine();
        ImGui::Text("columns: %d", layoutColumnCount);
        ImGui::SameLine();
        ImGui::Text("spacing: %f", layoutItemSpacing);


#if 0
        // multi-select
        ImGuiMultiSelectFlags msFlags = ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_ClearOnClickVoid;

        // enable box-select (in 2D mode, so that changing box-select rectangle x1/x2 will affect clipped items)
        if (allowBoxSelect)
            msFlags |= ImGuiMultiSelectFlags_BoxSelect2d;

        // enable keyboard wrapping on X axis
        msFlags |= ImGuiMultiSelectFlags_NavWrapX;

        ImGuiMultiSelectIO* msIO = ImGui::BeginMultiSelect(msFlags, 10, numItems);

        //
#endif
        // calculate and store start position
        ImVec2 startPos = ImGui::GetCursorScreenPos();
        startPos = ImVec2(startPos.x + layoutOuterPadding, startPos.y + layoutOuterPadding);
        ImGui::SetCursorScreenPos(startPos);


        // draw texture icons
        DrawTextureIcons(startPos);


  
        // Zooming with CTRL+Wheel
        if (ImGui::IsWindowAppearing())
            zoomWheelAccum = 0.0f;

        if (ImGui::IsWindowHovered() && io.MouseWheel != 0.0f && ImGui::IsKeyDown(ImGuiMod_Ctrl) && ImGui::IsAnyItemActive() == false)
        {
            zoomWheelAccum += io.MouseWheel;

            if (fabsf(zoomWheelAccum) >= 1.0f)
            {
                // calculate hovered item index from mouse location
                const float hoveredItemNX = (io.MousePos.x - startPos.x + layoutItemSpacing * 0.5f) / layoutItemStep.x;
                const float hoveredItemNY = (io.MousePos.y - startPos.y + layoutItemSpacing * 0.5f) / layoutItemStep.y;
                const int hoveredItemIdx = ((int)hoveredItemNY * layoutColumnCount) + (int)hoveredItemNX;

                // zoom
                iconSize *= powf(1.1f, zoomWheelAccum);

                // clamp in range [16, 128]
                iconSize = (iconSize < 16.0f)  ? 16.0f : iconSize;
                iconSize = (iconSize > 128.0f) ? 128.0f : iconSize;
                zoomWheelAccum -= (int)zoomWheelAccum;
                UpdateLayoutSizes(availWidth);

                // manipulate scroll to that we will land at the same Y location of currently hovered item
                // - calculate next frame position of item under mouse
                // - set new scroll position to be used in next ImGui::BeginChild() call
                float hoveredItemRelPosY = ((float)(hoveredItemIdx / layoutColumnCount) + fmodf(hoveredItemNY, 1.0f)) * layoutItemStep.y;
                hoveredItemRelPosY += ImGui::GetStyle().WindowPadding.y;

                float mouseLocalY = io.MousePos.y - ImGui::GetWindowPos().y;
                ImGui::SetScrollY(hoveredItemRelPosY - mouseLocalY);
            }
        }

        // context menu
        if (ImGui::BeginPopupContextWindow())
        {
            ImGui::Text("TODO: print here a texture ID + name");
            ImGui::EndPopup();
        }
    }
    ImGui::EndChild();
    // TODO: render selectable grid of textures
    // TODO: zooming with CTRL+Wheel
}

}
