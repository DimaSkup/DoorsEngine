// =================================================================================
// Filename:    MaterialAssetsBrowser.cpp
//
// Created:     28.04.2025 by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include "MaterialAssetsBrowser.h"
#include <UICommon/StatesGUI.h>

#pragma warning (disable : 4996)


namespace UI
{

//---------------------------------------------------------
//---------------------------------------------------------
void MaterialAssetsBrowser::Initialize(IFacadeEngineToUI* pFacade)
{
    if (!pFacade)
    {
        LogErr(LOG, "can't init materials assets browser: input ptr to UI facade == nullptr");
        return;
    }

    size numMaterials = 0;
    pFacade->GetNumMaterials(numMaterials);
    numItems_ = (int)numMaterials + 1;

    // init frame buffers so we will be able to render material icons into it
    pFacade->InitMaterialsIcons(numItems_, (int)iconSize_, (int)iconSize_);

    // and immediately render material icons so we won't re-render it each frame
    pFacade->RenderMaterialsIcons();
}

//---------------------------------------------------------
// Desc:  render a browser (window) of loaded materials
//---------------------------------------------------------
void MaterialAssetsBrowser::Render(IFacadeEngineToUI* pFacade, bool* pOpen)
{
    if (!pFacade)
    {
        LogErr(LOG, "can't render materials browser: input ptr to facade == nullptr");
        return;
    }

    pFacade_ = pFacade;

    ImGui::SetNextWindowContentSize(ImVec2(0.0f, layoutOuterPadding_ + numLayoutLine_ * (layoutItemSize_.y + layoutItemSpacing_)));

    RenderMenuBar(pOpen);

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

        // setup start position for debug info rendering
        ImVec2 startPos = ImGui::GetCursorScreenPos() + ImVec2(layoutOuterPadding_, layoutOuterPadding_);

        ImGui::SetCursorScreenPos(startPos);
        RenderDebugInfo(availWidth);

        // setup start position for icons rendering
        constexpr float offsetFromDbgInfo = 3;
        startPos = ImVec2(startPos.x, startPos.y + textLineHeight + offsetFromDbgInfo);

        DrawMaterialIcons(startPos);
        ComputeZooming(startPos, availWidth);


        if (showIconContextMenu_)
            ShowContextMenu();

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
bool MaterialAssetsBrowser::WasMaterialChanged()
{
    bool tmp = materialWasChanged_;
    materialWasChanged_ = false;
    return tmp;
}

//---------------------------------------------------------
// Desc:   draw a menu bar for the material browser
//---------------------------------------------------------
void MaterialAssetsBrowser::RenderMenuBar(bool* pOpen)
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
void MaterialAssetsBrowser::UpdateLayoutSizes(int availWidth)
{

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

//---------------------------------------------------------
// Desc:   print some debug info in the upper side of the material browser
//---------------------------------------------------------
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

//---------------------------------------------------------
// Desc:  draw a grid of icons where each icon represents some material
// Args:  - startPos:  a window (children) start position (upper left)
//---------------------------------------------------------
void MaterialAssetsBrowser::DrawMaterialIcons(const ImVec2 startPos)
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
                        GetMaterialDataByIdx(itemIdx);
                    }

                    // select a material when click RMB on some icon and open context menu
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                    {
                        GetMaterialDataByIdx(itemIdx);
                        showIconContextMenu_ = true;
                    }
               
                    // draw texture image icon
                    ImGui::SetCursorScreenPos(pos);
                    ImGui::Image((ImTextureID)pFacade_->materialIcons_[itemIdx], { iconSize_, iconSize_ });

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

//---------------------------------------------------------
// Desc:   load data of the material by index
//---------------------------------------------------------
void MaterialAssetsBrowser::GetMaterialDataByIdx(const int matIdx)
{
    if ((matIdx < 0) || (matIdx >= numItems_))
    {
        LogErr(LOG, "can't get data for material by idx: %d (is invalid)", matIdx);
        return;
    }

    if (pFacade_->GetMaterialIdByIdx(matIdx, selectedMaterialItemID_))
    {
        selectedItemIdx_ = matIdx;                        // update index so the icon of this material will be shown as selected
        materialWasChanged_ = true;                       // material was changed since the prev frame

        pFacade_->GetMaterialNameById(
            selectedMaterialItemID_,
            selectedMaterialName_,
            MAX_LENGTH_MATERIAL_NAME);

        pFacade_->GetMaterialTexIds(
            selectedMaterialItemID_,
            matData_.textureIDs);

        pFacade_->GetTexViewsByIds(
            matData_.textureIDs,
            matData_.textures,
            NUM_TEXTURE_TYPES);
    }
}

//---------------------------------------------------------
// Desc:  if we zoom in/out (Ctrl+mouse_wheel when over the material browser),
//        we need to recompute icons sizes and related stuff
//---------------------------------------------------------
void MaterialAssetsBrowser::ComputeZooming(const ImVec2 startPos, const int availWidth)
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
            const ImVec2 hoveredItemNum = (io.MousePos - startPos + spacing * 0.5f) / layoutItemStep_;
            const int hoveredItemIdx = ((int)hoveredItemNum.y * numLayoutColumn_) + (int)hoveredItemNum.x;

            // zoom
            iconSize_ *= powf(1.1f, zoomWheelAccum_);
            iconSize_ = std::clamp(iconSize_, 16.0f, 512.0f);
            zoomWheelAccum_ -= (int)zoomWheelAccum_;
            g_GuiStates.needUpdateMatBrowserIcons = true;
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

//---------------------------------------------------------
// Desc:   show a window which is used to edit a chosen material
// NOTE:   we split into half this window (left: preview; right: editing fields)
//---------------------------------------------------------
void MaterialAssetsBrowser::RenderEditorWnd()
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
        constexpr ImVec2 padding = { 20.0f, 40.0f };

        const ImVec2 wndPos1 = wndPos + padding;
        const ImVec2 wndPos2 = wndPos + padding + ImVec2(halfWidth, 0);

        // render material preview (model + material)
        ImGui::SetNextWindowPos(wndPos1);

        if (ImGui::BeginChild("Material preview", ImVec2(halfWidth-padding.x, availReg.y)))
        {
            RenderMatPreview();
        }
        ImGui::EndChild();


        // render material properties fields 
        ImGui::SetNextWindowPos(wndPos2);

        if (ImGui::BeginChild("Material props", ImVec2(halfWidth-padding.x, availReg.y)))
        {
            RenderMatPropsFields();

            if (showTextureContextMenu_)
                ShowTextureContextMenu();

        } 
        ImGui::EndChild();
    }
    ImGui::End();
}

//---------------------------------------------------------
// Desc:   in editor window of a single material we render some shape to visualize
//         this material (it can be sphere/cube/teapot/etc. with this material)
//---------------------------------------------------------
void MaterialAssetsBrowser::RenderMatPreview()
{
    const MaterialID matId = selectedMaterialItemID_;

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
    ImGui::Text("ID:   %ld", matId);
    ImGui::Text("Name: %s", selectedMaterialName_);
    ImGui::Separator();

    // if we selected another material we need to reload its data
    if (matData_.id != matId)
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
void MaterialAssetsBrowser::RenderMatPropsFields()
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
        pFacade_->SetMaterialColorData(
            matData_.id,
            matData_.ambient,
            matData_.diffuse,
            matData_.specular,
            matData_.reflect);
    }
    ImGui::Separator();


    // render selectors for choosing render state params of material
    using enum eMaterialPropGroup;

    if (ImGui::TreeNode("Select render state"))
    {

        RenderStatesSelectors("Select fill mode", FILL, matData_.selectedFillModeIdx);
        RenderStatesSelectors("Select cull mode", CULL, matData_.selectedCullModeIdx);

        RenderStatesSelectors("Select blend state",         BLENDING,      matData_.selectedBlendStateIdx);
        RenderStatesSelectors("Select depth-stencil state", DEPTH_STENCIL, matData_.selectedDepthStencilStateIdx);

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
void MaterialAssetsBrowser::RenderDeleteWnd()
{
    ImGui::OpenPopup("Delete?");

    // always center this window when appearing
    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Delete?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Do you really want to delete currenly selected material?");
        ImGui::Text("ID:   %ld", selectedMaterialItemID_);
        ImGui::Text("Name: %s",  selectedMaterialName_);
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
// Desc:   render selectors for choosing render state params of material
// Args:   - label:             text label about this types of selectors
//         - statesType:        what kind of render states selectors we will render
//         - pFacade:           facade interface btw UI and engine
//         - selectedStateIdx:  in/out index of selected state inside its group
//---------------------------------------------------------
void MaterialAssetsBrowser::RenderStatesSelectors(
    const char* label,
    const eMaterialPropGroup statesType,
    index& selectedStateIdx)
{
    if (StrHelper::IsEmpty(label))
    {
        LogErr(LOG, "input label (string) for render state is empty");
        return;
    }

    if (ImGui::TreeNode(label))
    {
        // TODO: maybe optimize somehow (to prevent dynamic alloc each time)
        // get names of each state for the current render states group
        cvector<std::string> statesNames;
        pFacade_->GetMaterialRenderStateNames(statesType, statesNames);


        for (index idx = 0; idx < statesNames.size(); ++idx)
        {
            if (ImGui::Selectable(statesNames[idx].c_str(), selectedStateIdx == idx))
            {
                pFacade_->SetMaterialRenderState(matData_.id, (uint32)idx, statesType);
                selectedStateIdx = idx;
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
void MaterialAssetsBrowser::DrawShadersSelectors()
{
    if (ImGui::TreeNode("Select shader"))
    {
        // TODO: maybe optimize somehow (to prevent dynamic alloc each time)
        cvector<ShaderData> shaderData;
        pFacade_->GetShadersIdAndName(shaderData);

        // currently selected shader of currently selected material
        ShaderID& selectedShaderId = matData_.shaderId;


        for (index idx = 0; idx < shaderData.size(); ++idx)
        {
            const char* name  = shaderData[idx].name;
            const ShaderID id = shaderData[idx].id;

            if (ImGui::Selectable(name, selectedShaderId == id))
            {
                // try to set a shader for material
                if (pFacade_->SetMaterialShaderId(matData_.id, id))
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
void MaterialAssetsBrowser::DrawRelatedTextures()
{
    // push specific colors for borders around textures icons
    ImGui::PushStyleColor(ImGuiCol_Header, { 1,1,1,1 });
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 1,1,1,1 });
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, { 1,1,1,1 });

    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    if (ImGui::TreeNode("Textures"))
    {
        ImVec2 wndPos = ImGui::GetItemRectMin();
        ImVec2 availReg = ImGui::GetItemRectSize();

        const char texTypesNames[NUM_TEXTURE_TYPES + 1][32] =
        {
            "NONE",
            "DIFFUSE",
            "SPECULAR",
            "AMBIENT",
            "EMISSIVE",
            "HEIGHT",
            "NORMALS",
            "SHININESS",
            "OPACITY",
            "DISPLACEMENT",
            "LIGHTMAP",
            "REFLECTION",
            "BASE_COLOR",
            "NORMAL_CAMERA",
            "EMISSION_COLOR",
            "METALNESS",
            "DIFFUSE_ROUGHNESS",
            "AMBIENT_OCCLUSION",
            "UNKNOWN",
            "SHEEN",
            "CLEARCOAT",
            "TRANSMISSION"
        };


        const ImVec2 iconSize    = { 96.0f, 96.0f };
        const ImVec2 padding     = { 20.0f, 20.0f };

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

        enum ContentsType { CT_Text, CT_FillButton };
        static int contents_type = CT_Text;

        if (ImGui::BeginTable("table", 2, flags))
        {
            const ImVec2 borderSize = { 2.0f, 2.0f };
            const float minRowHeight = iconSize.y + borderSize.y*4;

            for (index i = 0; i < NUM_TEXTURE_TYPES; ++i)
            {
                const TexID id = matData_.textureIDs[i];
                const ID3D11ShaderResourceView* pSRV = matData_.textures[i];
                
                if (id != INVALID_TEXTURE_ID)
                {
                    ImGui::TableNextRow(ImGuiTableRowFlags_None, minRowHeight);

                    // first column (texture info)
                    ImGui::TableNextColumn();
                    //ImGui::TableSetColumnIndex(0);
                    ImGui::Text("ID: %" PRIu32, id);
                    ImGui::Text("Type: %s (%d)", texTypesNames[i], (int)i);

                   

                    ImGui::TableNextColumn();

                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 1,1,1,1 });
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, borderSize);

                    char imgButtonId[32]{ '\0' };
                    snprintf(imgButtonId, 32, "##tex_icon %d", (int)id);
                    ImGui::ImageButton(imgButtonId, (ImTextureID)pSRV, iconSize);

                    // select a texture when click RMB on some icon and open a context menu
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                    {
                        textureContextMenuPos_ = ImGui::GetIO().MousePos;
                        showTextureContextMenu_ = true;
                    }

                    ImGui::PopStyleColor();
                    ImGui::PopStyleVar();

#if 0
                     // second column (image)
                    const ImVec2 cellPos = ImGui::GetCursorScreenPos();
                    const ImVec2 bgPos   = cellPos - borderSize;
                    const ImVec2 bgSize  = iconSize + borderSize * 2.0f;

                    char imgButtonId[32]{ '\0' };
                    snprintf(imgButtonId, 32, "##tex_icon %d", (int)id);

                    // render a border around this icon
                    ImGui::SetCursorScreenPos(bgPos);
                    ImGui::Selectable(imgButtonId, false, ImGuiSelectableFlags_None, bgSize);

                    // draw texture image icon
                    ImGui::SetCursorScreenPos(cellPos);
                    ImGui::Image((ImTextureID)pSRV, iconSize);
#endif               
                }
            }

            ImGui::EndTable();
        }

        ImGui::TreePop();
    }

    ImGui::PopStyleColor(3);
}

//---------------------------------------------------------
// Desc:  render a context menu after pressing RMB over some icon
//---------------------------------------------------------
void MaterialAssetsBrowser::ShowContextMenu()
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
// Desc:  render a context menu after pressing RMB over some icon
//---------------------------------------------------------
void MaterialAssetsBrowser::ShowTextureContextMenu()
{
    ImGui::SetNextWindowPos(textureContextMenuPos_);

    if (ImGui::BeginPopupContextWindow())
    {
        showTextureSelectionMenu_ = ImGui::MenuItem("Select", "");
        showTextureLoadingWnd_    = ImGui::MenuItem("Load new", "");

        // if any option in the context menu was chosed we hide the context menu
        if (showTextureSelectionMenu_ || showTextureLoadingWnd_)
            showTextureContextMenu_ = false;

        ImGui::EndPopup();
    }
}

} // namespace UI
