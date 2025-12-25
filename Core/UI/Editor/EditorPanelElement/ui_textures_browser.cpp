// =================================================================================
// Filename:    ui_textures_browser.cpp
// Description: implementation of UITexturesBrowser functional
//
// Created:     06.04.2025 by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include "ui_textures_browser.h"
#include <win_file_dialog.h>
#include <UICommon/gui_states.h>
#include <UICommon/IFacadeEngineToUI.h>
#include <imgui.h>

#pragma warning (disable : 4996)


namespace UI
{

void UITexturesBrowser::Init(IFacadeEngineToUI* pFacade)
{
    if (!pFacade)
    {
        LogErr(LOG, "can't init textures browser");
    }

    // get a pointer to the array of textures shader resource views
    size tmpNumItems = 0;
    ID3D11ShaderResourceView** tempArr = nullptr;

    pFacade->GetArrTexturesSRVs(tempArr, tmpNumItems);
    numItems_ = (int)tmpNumItems;
}

//---------------------------------------------------------
// render a browser (window) of loaded textures
//---------------------------------------------------------
void UITexturesBrowser::Render(IFacadeEngineToUI* pFacade, bool* pOpen)
{
    if (!pFacade)
    {
        LogErr(LOG, "can't render textures browser");
        return;
    }
    pFacade_ = pFacade;


    ImGui::SetNextWindowContentSize(ImVec2(0.0f, layoutOuterPadding_ + numLayoutLine_ * (layoutItemSizeY_ + layoutItemSpacing_)));
    RenderMenuBar(pOpen);

    // ------------------------------------------

    const float textLineHeight = ImGui::GetTextLineHeightWithSpacing();

    if (ImGui::BeginChild("TextureAssets", ImVec2(0, -textLineHeight), ImGuiChildFlags_None, ImGuiWindowFlags_NoMove))
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
        ImVec2 padding  = { layoutOuterPadding_, layoutOuterPadding_ };

        startPos += padding;
        ImGui::SetCursorScreenPos(startPos);
        RenderDebugInfo(availWidth);

        // setup start position for icons rendering
        constexpr float offsetFromDbgInfo = 3;
        startPos.y += (textLineHeight + offsetFromDbgInfo);

        DrawTextureIcons(startPos.x, startPos.y);
        ComputeZooming(startPos.x, startPos.y, availWidth);


        if (showIconContextMenu_)
            ShowContextMenu();

        if (showEditorWnd_)
            ShowEditorWnd();
    }
    ImGui::EndChild();
}


// =================================================================================
// Private methods
// =================================================================================
void UITexturesBrowser::RenderMenuBar(bool* pOpen)
{
    if (pOpen == nullptr)
    {
        LogErr(LOG, "input ptr == nullptr");
        return;
    }

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
            if (ImGui::MenuItem("Load"))
            {
                std::string texPath;
                DialogWndFileOpen(texPath);

                if (!pFacade_->LoadTextureFromFile(texPath.c_str()))
                {
                    LogErr(LOG, "can't load a texture from file: %s", texPath.c_str());
                }
            }
            ImGui::MenuItem("Generate");

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}

//-----------------------------------------------------------------------------
// when we did some manipulations over the browser's window (resized, zoomed)
// we need to update the icons size and all the related stuff
//-----------------------------------------------------------------------------
void UITexturesBrowser::UpdateLayoutSizes(float availWidth)
{
    layoutItemSpacing_ = (float)iconSpacing_;    

    // when not stretching: allow extending into right-most spacing
    if (stretchSpacing_ == false)
        availWidth += floorf(layoutItemSpacing_ * 0.5f);

    // calc number of icons per line and number of lines
    layoutItemSizeX_ = floorf(iconSize_);
    layoutItemSizeY_ = layoutItemSizeX_;

    // calc how many icons rows and columns will we have
    numLayoutColumn_ = max((int)(availWidth / (layoutItemSizeX_ + layoutItemSpacing_)), 1);
    numLayoutLine_   = (numItems_ + numLayoutColumn_ - 1) / numLayoutColumn_;

    // when stretching: allocate remaining space to more spacing. Round before division, so itemSpacing may be non-integer
    if (stretchSpacing_ && (numLayoutColumn_ > 1))
        layoutItemSpacing_   = floorf(availWidth - layoutItemSizeX_ * numLayoutColumn_) / numLayoutColumn_;

    layoutItemStepX_         = layoutItemSizeX_ + layoutItemSpacing_;
    layoutItemStepY_         = layoutItemSizeY_ + layoutItemSpacing_;
    layoutSelectableSpacing_ = max(floorf(layoutItemSpacing_) - iconHitSpacing_, 0.0f);
    layoutOuterPadding_      = floorf(layoutItemSpacing_ * 0.5f);
}

//---------------------------------------------------------
// Desc:  draw a grid of texture icons
//---------------------------------------------------------
void UITexturesBrowser::DrawTextureIcons(const float startPosX, const float startPosY)
{
    // push specific colors (for
    ImGui::PushStyleColor(ImGuiCol_Header, { 1,1,1,1 });
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 1,1,1,1 });
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, { 1,1,1,1 });

    const int columnCount = numLayoutColumn_;
    const ImVec2 iconSize = { iconSize_, iconSize_ };
    const ImVec2 itemSize = { layoutItemSizeX_, layoutItemSizeY_ };

    // get a ptr to arr of shader resource views of all the currently loaded textures
    size tmpNumItems = 0;
    ID3D11ShaderResourceView** arrSRVs = nullptr;
    pFacade_->GetArrTexturesSRVs(arrSRVs, tmpNumItems);
    numItems_ = (int)tmpNumItems;


    // render icons
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
     
                // position of current item
                const float  itemOffsetX = (itemIdx % columnCount) * layoutItemStepX_;
                const float  itemOffsetY = lineIdx * layoutItemStepY_;
                const ImVec2 iconPos     = { startPosX + itemOffsetX, startPosY + itemOffsetY };

                ImGui::SetCursorScreenPos(iconPos);
                ImGui::SetNextItemSelectionUserData(itemIdx);

                // check if we see a texture icon
                if (!ImGui::IsRectVisible(itemSize))
                {
                    ImGui::PopID();
                    continue;
                }
                    

                //
                // render icon
                //
                char buf[32]{'\0'};
                snprintf(buf, 32, "##texture_item %d", itemIdx);

                bool chooseAnother = false;
                const bool isSelected = (itemIdx == selectedItemIdx_);

                // we can select a texture when click on some icon
                // ALSO: render a border around this icon
                if (ImGui::Selectable(buf, isSelected, ImGuiSelectableFlags_None, itemSize))
                {
                    chooseAnother = true;
                }

                // select a texture when click RMB on some icon and open a context menu
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                {
                    chooseAnother = true;
                    showIconContextMenu_ = true;
                }

                if (chooseAnother)
                {
                    // try to get a texture ID by index
                    const TexID texId = pFacade_->GetTextureIdByIdx(selectedItemIdx_);

                    if (texId != INVALID_TEXTURE_ID)
                    {
                        const char* texName = pFacade_->GetTextureNameById(selectedTexItemID_);

                        textureWasChanged_ = true;
                        selectedTexItemID_ = texId;
                        strcpy(selectedTextureName_.name, texName);
                    }
                }

                // draw background (so we will have a border around icon)
                const ImVec2 boxMin      = { iconPos.x - 1, iconPos.y - 1 };
                const ImVec2 boxMax      = { boxMin + itemSize + ImVec2(2, 2) };
                const ImU32  iconBgColor = ImGui::GetColorU32({ 0,0,0,1 });

                ImDrawList* pDrawList = ImGui::GetWindowDrawList();
                pDrawList->AddRectFilled(boxMin, boxMax, iconBgColor);

                // draw texture image icon
                ImGui::SetCursorScreenPos(iconPos);
                ImGui::Image((ImTextureID)arrSRVs[itemIdx], iconSize);

                // draw label (just an index)
                const ImVec2 labelPos   = { boxMin.x, boxMax.y - ImGui::GetFontSize() };
                const ImU32  labelColor = ImGui::GetColorU32(ImGuiCol_Text);

                char label[16]{'\0'};
                snprintf(label, 16, "%d", itemIdx);
                pDrawList->AddText(labelPos, labelColor, label);
               
                ImGui::PopID();
            }
        }
    }

    clipper.End();
    ImGui::PopStyleColor(3);
}

///////////////////////////////////////////////////////

void UITexturesBrowser::RenderDebugInfo(const float availWidth)
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

//---------------------------------------------------------
// Desc:   if we zoom in/out (Ctrl+mouse_wheel when over the browser),
//         we need to recompute icons sizes and related stuff
//---------------------------------------------------------
void UITexturesBrowser::ComputeZooming(
    const float startPosX,
    const float startPosY,
    const float availWidth)
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
            const float hoveredItemNX = (io.MousePos.x - startPosX + layoutItemSpacing_ * 0.5f) / layoutItemStepX_;
            const float hoveredItemNY = (io.MousePos.y - startPosY + layoutItemSpacing_ * 0.5f) / layoutItemStepY_;
            const int  hoveredItemIdx = ((int)hoveredItemNY * numLayoutColumn_) + (int)hoveredItemNX;

            // zoom
            iconSize_ *= powf(1.1f, zoomWheelAccum_);
            iconSize_ = std::clamp(iconSize_, 16.0f, 512.0f);
            zoomWheelAccum_ -= (int)zoomWheelAccum_;
            UpdateLayoutSizes(availWidth);

            // manipulate scroll to that we will land at the same Y location of currently hovered item
            // - calculate next frame position of item under mouse
            // - set new scroll position to be used in next ImGui::BeginChild() call
            float hoveredItemRelPosY = ((float)(hoveredItemIdx / numLayoutColumn_) + fmodf(hoveredItemNY, 1.0f)) * layoutItemStepY_;
            hoveredItemRelPosY += ImGui::GetStyle().WindowPadding.y;

            float mouseLocalY = io.MousePos.y - ImGui::GetWindowPos().y;
            ImGui::SetScrollY(hoveredItemRelPosY - mouseLocalY);
        }
    }
}

//---------------------------------------------------------
// Desc:  render a context menu after pressing RMB over some icon
//---------------------------------------------------------
void UITexturesBrowser::ShowContextMenu()
{
    if (ImGui::BeginPopupContextWindow())
    {
        showEditorWnd_ = ImGui::MenuItem("Edit", "");
        showDeleteWnd_ = ImGui::MenuItem("Delete", "");

        // if any option in the context menu was chosed we hide the context menu
        if (showEditorWnd_ || showDeleteWnd_)
            showIconContextMenu_ = false;

        ImGui::EndPopup();
    }
}

//---------------------------------------------------------
// Desc:   show a window which is used to edit a chosen material
// NOTE:   we split into half this window (left: preview; right: editing fields)
//---------------------------------------------------------
void UITexturesBrowser::ShowEditorWnd()
{
    // always center this window when appearing
    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    const ImVec2 size   = ImGui::GetMainViewport()->Size / 2;
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(size, ImGuiCond_Appearing);


    if (ImGui::Begin("Texture editor", &showEditorWnd_, ImGuiWindowFlags_NoScrollbar))
    {
        const ImVec2 wndPos     = ImGui::GetWindowPos();
        const ImVec2 availReg   = ImGui::GetContentRegionAvail();
        const float halfWidth   = 0.5f * availReg.x;
        constexpr float padding = 20.0f;
        const float wndWidth    = halfWidth - (padding * 0.5f);

        // setup upper-left pos for each half, and its size as well
        const ImVec2 wndPos1    = ImVec2(wndPos.x + padding,             wndPos.y + padding*2);
        const ImVec2 wndPos2    = ImVec2(wndPos.x + halfWidth + padding, wndPos.y + padding*2);
        const ImVec2 wndSize    = ImVec2(wndWidth, availReg.y);

        
        // get a ptr to arr of shader resource views of all the currently loaded textures
        ptrdiff_t sz = 0;
        ID3D11ShaderResourceView** arrSRVs = nullptr;
        pFacade_->GetArrTexturesSRVs(arrSRVs, sz);

        // render texture preview
        ImGui::SetNextWindowPos(wndPos1);

        if (ImGui::BeginChild("Preview", wndSize))
        {
            const float       imgSize = halfWidth - padding*2;
            const ImTextureID texId   = (ImTextureID)arrSRVs[selectedItemIdx_];

            ImGui::Image(texId, { imgSize, imgSize });
        }
        ImGui::EndChild();


        // render texture properties fields 
        ImGui::SetNextWindowPos(wndPos2);

        if (ImGui::BeginChild("Properties", wndSize))
        {
            const TexID texId = selectedTexItemID_;
            ImGui::Text("ID:   %" PRIu32, texId);
            ImGui::Text("Name: %s", selectedTextureName_.name);

            //-------------------------

            if (ImGui::Button("Reload"))
            {
                // open dialog window to choose another texture, so we will get its path
                std::string texPath;
                DialogWndFileOpen(texPath);

                if (!pFacade_->ReloadTextureFromFile(texId, texPath.c_str()))
                {
                    LogErr(LOG, "can't reload a texture (%" PRIu32 ") from file : %s", texId, texPath.c_str());
                }

                // update UI info about the texture
                const char* texName = pFacade_->GetTextureNameById(selectedTexItemID_);
                if (texName)
                    strcpy(selectedTextureName_.name, texName);

                // update material icons since we changed a texture
                g_GuiStates.needUpdateMatBrowserIcons = true;
            }

            //-------------------------

            if (ImGui::Button("Set another"))
            {

            }

            //-------------------------

            if (ImGui::Button("Generate"))
            {

            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}


} // namespace UI
