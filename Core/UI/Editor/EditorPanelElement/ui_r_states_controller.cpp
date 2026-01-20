/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: ui_r_states_controller.cpp
    Desc:     implementation of UIRenderStatesController functional

    Created:  20.01.2026 by DimaSkup
\**********************************************************************************/
#include <CoreCommon/pch.h>
#include "ui_r_states_controller.h"
#include <imgui.h>
#include <UICommon/IFacadeEngineToUI.h>


namespace UI
{

//---------------------------------------------------------
// constructor
//---------------------------------------------------------
UIRenderStatesController::UIRenderStatesController(IFacadeEngineToUI* pFacade)
{
    assert(pFacade != nullptr);
    pFacade_ = pFacade;
}
    
//---------------------------------------------------------
// Desc:  draw a dropdown menu (ImGui::Combo)
// Args:  - label:      label for the menu
//        - comboId:    unique identifier for the menu
//        - currParam:  (in/out) current parameter (will be highlighted) and if we
//                      select any option it will be rewritten by this option value
//                      (we will just set a pointer to another value)
//        - items:      selectable options of menu
//        - numItems:   num of selectable options in menu
//
// Ret:   true if we selected any other option
//---------------------------------------------------------
bool DrawDropdown(
    const char* label,
    const char* comboId,
    const char** currParam,
    const char** items,
    const int numItems)
{
    // check input args
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
    if (!currParam || StrHelper::IsEmpty(*currParam))
    {
        LogErr(LOG, "empty current param");
        return false;
    }
    if (!items)
    {
        LogErr(LOG, "empty items arr");
        return false;
    }

    //---------------------------------

    bool changed = false;

    ImGui::TableNextColumn();
    ImGui::Text(label);

    ImGui::TableNextColumn();
    ImGui::PushItemWidth(-FLT_MIN);

    // dropdown menu
    if (ImGui::BeginCombo(comboId, *currParam))
    {
        for (int i = 0; i < numItems; ++i)
        {
            const bool isSelected = (strcmp(*currParam, items[i]) == 0);
            if (ImGui::Selectable(items[i], isSelected))
            {
                *currParam = items[i];
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

void UIRenderStatesController::DrawRasterParamsControl(const RsID rsId)
{
    // allow params modification only if id == 1 (raster state is "custom")
    if (rsId != 1)
    {
        ImGui::BeginDisabled(true);
    }

    //
    // get current params of a render state by id
    //
    bool bFrontCCW              = pFacade_->GetRsParamBool(rsId, UI_RS_FRONT_CCW);
    bool bDepthClip             = pFacade_->GetRsParamBool(rsId, UI_RS_DEPTH_CLIP_ENABLE);
    bool bScissor               = pFacade_->GetRsParamBool(rsId, UI_RS_SCISSOR_ENABLE);
    bool bMultisample           = pFacade_->GetRsParamBool(rsId, UI_RS_MULTISAMPLE_ENABLE);
    bool bAntiAliasedLine       = pFacade_->GetRsParamBool(rsId, UI_RS_ANTIALIASED_LINE_ENABLE);

    const char* fillMode        = pFacade_->GetRsParamStr(rsId, UI_RS_FILL);
    const char* cullMode        = pFacade_->GetRsParamStr(rsId, UI_RS_CULL);

    int   depthBias             = pFacade_->GetRsParamInt(rsId, UI_RS_DEPTH_BIAS);
    float depthBiasClamp        = pFacade_->GetRsParamFloat(rsId, UI_RS_DEPTH_BIAS_CLAMP);
    float slopeScaledDepthBias  = pFacade_->GetRsParamFloat(rsId, UI_RS_SLOPE_SCALED_DEPTH_BIAS);

    //
    // get arrays of available fill and cull modes options
    //
    const int numFillModes      = pFacade_->GetNumRsParams(UI_RS_FILL);
    const char** fillModes      = pFacade_->GetRsParamsNames(UI_RS_FILL);

    const int numCullModes      = pFacade_->GetNumRsParams(UI_RS_CULL);
    const char** cullModes      = pFacade_->GetRsParamsNames(UI_RS_CULL);

    //
    // draw control checkboxes
    //
    if (ImGui::Checkbox("Front counter clockwise", &bFrontCCW))
        pFacade_->UpdateCustomRsParam(UI_RS_FRONT_CCW, bFrontCCW);

    if (ImGui::Checkbox("Depth clip enable", &bDepthClip))
        pFacade_->UpdateCustomRsParam(UI_RS_DEPTH_CLIP_ENABLE, bDepthClip);

    if (ImGui::Checkbox("Scissor enable", &bScissor))
        pFacade_->UpdateCustomRsParam(UI_RS_SCISSOR_ENABLE, bScissor);

    if (ImGui::Checkbox("Multisample enable", &bMultisample))
        pFacade_->UpdateCustomRsParam(UI_RS_MULTISAMPLE_ENABLE, bMultisample);

    if (ImGui::Checkbox("AntiAliased line enable", &bAntiAliasedLine))
        pFacade_->UpdateCustomRsParam(UI_RS_ANTIALIASED_LINE_ENABLE, bAntiAliasedLine);

    //
    // Draw a table of dropdown menus to fill mode and cull mode
    //
    const int             numColumns = 2;
    const ImGuiTableFlags tableFlags = 0;

    if (ImGui::BeginTable("Setup raster state params", numColumns, tableFlags))
    {
        // 1st row
        if (DrawDropdown(
            "Fill",
            "##rs_fill",
            &fillMode,
            fillModes,
            numFillModes))
        {
            pFacade_->UpdateCustomRsParam(UI_RS_FILL, fillMode);
        }

        // 2nd row
        if (DrawDropdown(
            "Cull",
            "##rs_cull",
            &cullMode,
            cullModes,
            numCullModes))
        {
            pFacade_->UpdateCustomRsParam(UI_RS_CULL, cullMode);
        }


        // 3rd row 
        ImGui::TableNextColumn();
        ImGui::Text("Depth bias");

        ImGui::TableNextColumn();
        ImGui::PushItemWidth(-FLT_MIN);
        if (ImGui::InputInt("##depth_bias", &depthBias))
        {
            eRasterProp prop = UI_RS_DEPTH_BIAS;
            pFacade_->UpdateCustomRsParam(prop, depthBias);
        }
        ImGui::PopItemWidth();


        // 4th row
        ImGui::TableNextColumn();
        ImGui::Text("Depth bias clamp");

        ImGui::TableNextColumn();
        ImGui::PushItemWidth(-FLT_MIN);
        if (ImGui::InputFloat("##depth_bias_clamp", &depthBiasClamp))
        {
            eRasterProp prop = UI_RS_DEPTH_BIAS_CLAMP;
            pFacade_->UpdateCustomRsParam(prop, depthBiasClamp);
        }
        ImGui::PopItemWidth();


        // 5th row
        ImGui::TableNextColumn();
        ImGui::Text("Slope scaled depth bias");

        ImGui::TableNextColumn();
        ImGui::PushItemWidth(-FLT_MIN);
        if (ImGui::InputFloat("##slope_scaled_depth_bias", &slopeScaledDepthBias))
        {
            eRasterProp prop = UI_RS_SLOPE_SCALED_DEPTH_BIAS;
            pFacade_->UpdateCustomRsParam(prop, slopeScaledDepthBias);
        }
        ImGui::PopItemWidth();


        ImGui::EndTable();
    }
    

    // (for clearness: look at the same condition at the beginning of this method)
    if (rsId != 1)
    {
        ImGui::EndDisabled();
    }
}
 
//---------------------------------------------------------

void UIRenderStatesController::DrawBlendParamsControl(const BsID bsId)
{
    // allow params modification only if id == 1 (blend state is "custom")
    if (bsId != 1)
    {
        ImGui::BeginDisabled(true);
    }

    //
    // get current params of the blend state
    //
    bool alphaToCoverage          = pFacade_->GetBsParamBool(bsId, UI_BLEND_IS_ALPHA_TO_COVERAGE);
    bool independentBlend         = pFacade_->GetBsParamBool(bsId, UI_BLEND_IS_INDEPENDENT);
    bool blendEnabled             = pFacade_->GetBsParamBool(bsId, UI_BLEND_IS_ENABLED);

    // color blend (factor, factor, operation)
    const char* currSrcBlend      = pFacade_->GetBsParamStr(bsId, UI_BLEND_SRC_BLEND);
    const char* currDstBlend      = pFacade_->GetBsParamStr(bsId, UI_BLEND_DST_BLEND);
    const char* currBlendOp       = pFacade_->GetBsParamStr(bsId, UI_BLEND_OP);

    // alpha blend (factor, factor, operation)
    const char* currSrcBlendAlpha = pFacade_->GetBsParamStr(bsId, UI_BLEND_SRC_BLEND_ALPHA);
    const char* currDstBlendAlpha = pFacade_->GetBsParamStr(bsId, UI_BLEND_DST_BLEND_ALPHA);
    const char* currBlendOpAlpha  = pFacade_->GetBsParamStr(bsId, UI_BLEND_OP_ALPHA);

    // render target write mask
    const char* currWriteMask     = pFacade_->GetBsParamStr(bsId, UI_BLEND_RND_TARGET_WRITE_MASK);


    //
    // get available blend options to switch to
    //

    // get names arr of available blend factors and its count
    const int numBlendFactors     = pFacade_->GetNumBsParams(UI_BLEND_FACTOR);
    const char** blendFactors     = pFacade_->GetBsParamsNames(UI_BLEND_FACTOR);

    // get names arr of possible blend operations and its count
    const int numBlendOps         = pFacade_->GetNumBsParams(UI_BLEND_OPERATION);
    const char** blendOps         = pFacade_->GetBsParamsNames(UI_BLEND_OPERATION);

    // get names arr of available render target write masks (for blending) and its count
    const int numWriteMasks       = pFacade_->GetNumBsParams(UI_BLEND_RND_TARGET_WRITE_MASK);
    const char** writeMasks       = pFacade_->GetBsParamsNames(UI_BLEND_RND_TARGET_WRITE_MASK);


    //
    // draw control checkboxes
    //
    if (ImGui::Checkbox("Alpha to coverage", &alphaToCoverage))
    {
        pFacade_->UpdateCustomBsParam(UI_BLEND_IS_ALPHA_TO_COVERAGE, alphaToCoverage);
    }
    if (ImGui::Checkbox("Independent blend", &independentBlend))
    {
        pFacade_->UpdateCustomBsParam(UI_BLEND_IS_INDEPENDENT, independentBlend);
    }
    if (ImGui::Checkbox("Blend enabled", &blendEnabled))
    {
        pFacade_->UpdateCustomBsParam(UI_BLEND_IS_ENABLED, blendEnabled);
    }

    ImGui::NewLine();


    //
    // Draw a table of dropdown menus to control blend factors, blend operations, etc.
    //
    const int             numColumns = 2;
    const ImGuiTableFlags tableFlags = 0;

    if (ImGui::BeginTable("SetupBlendParams", numColumns, tableFlags))
    {
        // 1st row (souce color blend factor)
        if (DrawDropdown(
            "Src color blend factor",
            "##src_col_blend_factor",
            &currSrcBlend,
            blendFactors,
            numBlendFactors))
        {
            pFacade_->UpdateCustomBsParam(UI_BLEND_SRC_BLEND, currSrcBlend);
        }

        // 2nd row (destination color blend factor)
        if (DrawDropdown(
            "Dst color blend factor",
            "##dst_col_blend_factor",
            &currDstBlend,
            blendFactors,
            numBlendFactors))
        {
            pFacade_->UpdateCustomBsParam(UI_BLEND_DST_BLEND, currDstBlend);
        }

        // 3rd row (color blend operation)
        if (DrawDropdown(
            "Color blend op",
            "##col_blend_op",
            &currBlendOp,
            blendOps,
            numBlendOps))
        {
            pFacade_->UpdateCustomBsParam(UI_BLEND_OP, currBlendOp);
        }
        
        // 4th row (src alpha blend factor)
        if (DrawDropdown(
            "Src alpha blend factor",
            "##src_alpha_blend_factor",
            &currSrcBlendAlpha,
            blendFactors,
            numBlendFactors))
        {
            pFacade_->UpdateCustomBsParam(UI_BLEND_SRC_BLEND_ALPHA, currSrcBlendAlpha);
        }

        // 5th row (dst alpha blend factor)
        if (DrawDropdown(
            "Dst alpha blend factor",
            "##dst_alpha_blend_factor",
            &currDstBlendAlpha,
            blendFactors,
            numBlendFactors))
        {
            pFacade_->UpdateCustomBsParam(UI_BLEND_DST_BLEND_ALPHA, currDstBlendAlpha);
        }

        // 6th row (alpha blend operation)
        if (DrawDropdown(
            "Alpha blend op",
            "##alpha_blend_op",
            &currBlendOpAlpha,
            blendOps,
            numBlendOps))
        {
            pFacade_->UpdateCustomBsParam(UI_BLEND_OP_ALPHA, currBlendOpAlpha);
        }

        // 7th row (blend render target write mask)
        if (DrawDropdown(
            "Blend render target write mask",
            "##blend_write_mask",
            &currWriteMask,
            writeMasks,
            numWriteMasks))
        {
            pFacade_->UpdateCustomBsParam(UI_BLEND_RND_TARGET_WRITE_MASK, currWriteMask);
        }

        ImGui::EndTable();
    }

    // (for clearness: look at the same condition at the beginning of this method)
    if (bsId != 1)
    {
        ImGui::EndDisabled();
    }
}


//---------------------------------------------------------
//---------------------------------------------------------
void UIRenderStatesController::DrawDepthStencilParamsControl(const DssID dssId)
{
    // allow params modification only if id == 1 (depth-stencil state is "custom")
    if (dssId != 1)
    {
        ImGui::BeginDisabled(true);
    }

    //
    // get current params of the depth-stencil state
    //
    bool depthEnabled                = pFacade_->GetDssParamBool(dssId, UI_DSS_DEPTH_ENABLED);
    bool stencilEnabled              = pFacade_->GetDssParamBool(dssId, UI_DSS_STENCIL_ENABLED);

    const char* depthWriteMask       = pFacade_->GetDssParamStr(dssId, UI_DSS_DEPTH_WRITE_MASK);
    const char* depthFunc            = pFacade_->GetDssParamStr(dssId, UI_DSS_DEPTH_FUNC);

    // ff - front face
    const char* ffStencilFailOp      = pFacade_->GetDssParamStr(dssId, UI_DSS_FRONT_FACE_STENCIL_FAIL_OP);
    const char* ffStencilDepthFailOp = pFacade_->GetDssParamStr(dssId, UI_DSS_FRONT_FACE_STENCIL_DEPTH_FAIL_OP);
    const char* ffStencilPassOp      = pFacade_->GetDssParamStr(dssId, UI_DSS_FRONT_FACE_STENCIL_PASS_OP);
    const char* ffStencilFunc        = pFacade_->GetDssParamStr(dssId, UI_DSS_FRONT_FACE_STENCIL_FUNC);

    // bf - back face
    const char* bfStencilFailOp      = pFacade_->GetDssParamStr(dssId, UI_DSS_BACK_FACE_STENCIL_FAIL_OP);
    const char* bfStencilDepthFailOp = pFacade_->GetDssParamStr(dssId, UI_DSS_BACK_FACE_STENCIL_DEPTH_FAIL_OP);
    const char* bfStencilPassOp      = pFacade_->GetDssParamStr(dssId, UI_DSS_BACK_FACE_STENCIL_PASS_OP);
    const char* bfStencilFunc        = pFacade_->GetDssParamStr(dssId, UI_DSS_BACK_FACE_STENCIL_FUNC);


    //
    // get available dss params to switch to
    //
    const int numDepthWriteMasks     = pFacade_->GetNumDssParams(UI_DSS_DEPTH_WRITE_MASK);
    const char** depthWriteMasks     = pFacade_->GetDssParamsNames(UI_DSS_DEPTH_WRITE_MASK);

    // comparison func
    const int numCmpFunctions        = pFacade_->GetNumDssParams(UI_DSS_COMPARISON_FUNC);
    const char** cmpFunctions        = pFacade_->GetDssParamsNames(UI_DSS_COMPARISON_FUNC);

    // stencil operations
    const int  numStencilOps         = pFacade_->GetNumDssParams(UI_DSS_STENCIL_OP);
    const char**  stencilOps         = pFacade_->GetDssParamsNames(UI_DSS_STENCIL_OP);


    //
    // draw control checkboxes
    //
    if (ImGui::Checkbox("Depth enabled", &depthEnabled))
    {
        pFacade_->UpdateCustomDssParam(UI_DSS_DEPTH_ENABLED, depthEnabled);
    }
    if (ImGui::Checkbox("Stencil enabled", &stencilEnabled))
    {
        pFacade_->UpdateCustomDssParam(UI_DSS_STENCIL_ENABLED, stencilEnabled);
    }

    ImGui::NewLine();

    //
    // draw a table of dropdown menus to control depth func, stencil operations, etc.
    //
    const int             numColumns = 2;
    const ImGuiTableFlags tableFlags = 0;

    if (ImGui::BeginTable("SetupDepthStencilParams", numColumns, tableFlags))
    {
        // 1st row (depth write mask)
        if (DrawDropdown(
            "Write Mask",
            "##depth_write_mask",
            &depthWriteMask,
            depthWriteMasks,
            numDepthWriteMasks))
        {
            pFacade_->UpdateCustomDssParam(UI_DSS_DEPTH_WRITE_MASK, depthWriteMask);
        }

        // 2nd row (depth func)
        if (DrawDropdown(
            "Depth func",
            "##depth_func",
            &depthFunc,
            cmpFunctions,
            numCmpFunctions))
        {
            pFacade_->UpdateCustomDssParam(UI_DSS_DEPTH_FUNC, depthFunc);
        }

        // 3rd row (stencil read mask)
        ImGui::TableNextColumn();
        ImGui::Text("Stencil read mask");

        ImGui::TableNextColumn();
        ImGui::PushItemWidth(-FLT_MIN);
        ImGui::Text("0xFF");
        ImGui::PopItemWidth();

        // 4th row (stencil write mask)
        ImGui::TableNextColumn();
        ImGui::Text("Stencil write mask");

        ImGui::TableNextColumn();
        ImGui::PushItemWidth(-FLT_MIN);
        ImGui::Text("0xFF");
        ImGui::PopItemWidth();


        //-----------------------------
        // front face params:
        //-----------------------------

        // add empty line for visual sepration
        ImGui::TableNextColumn();
        ImGui::Text(" ");
        ImGui::TableNextRow();

        // a lable of the table block
        ImGui::TableNextColumn();
        ImGui::Text("Front face stencil:");
        ImGui::TableNextRow();

        // 5th row (ff stencil fail op)
        if (DrawDropdown(
            "fail operation",
            "##ff_stencil_fail_op",
            &ffStencilFailOp,
            stencilOps,
            numStencilOps))
        {
            eDepthStencilProp prop = UI_DSS_FRONT_FACE_STENCIL_FAIL_OP;
            pFacade_->UpdateCustomDssParam(prop, ffStencilFailOp);
        }

        // 6th row (ff stencil depth fail op)
        if (DrawDropdown(
            "depth fail op",
            "##ff_stencil_depth_fail_op",
            &ffStencilDepthFailOp,
            stencilOps,
            numStencilOps))
        {
            eDepthStencilProp prop = UI_DSS_FRONT_FACE_STENCIL_DEPTH_FAIL_OP;
            pFacade_->UpdateCustomDssParam(prop, ffStencilDepthFailOp);
        }

        // 7th row (ff stencil pass op)
        if (DrawDropdown(
            "pass op",
            "##ff_stencil_pass_op",
            &ffStencilPassOp,
            stencilOps,
            numStencilOps))
        {
            eDepthStencilProp prop = UI_DSS_FRONT_FACE_STENCIL_PASS_OP;
            pFacade_->UpdateCustomDssParam(prop, ffStencilPassOp);
        }

        // 8th row (ff stencil func)
        if (DrawDropdown(
            "stencil func",
            "#ff_stencil_func",
            &ffStencilFunc,
            cmpFunctions,
            numCmpFunctions))
        {
            eDepthStencilProp prop = UI_DSS_FRONT_FACE_STENCIL_FUNC;
            pFacade_->UpdateCustomDssParam(prop, ffStencilFunc);
        }


        //-----------------------------
        // back face params:
        //-----------------------------

        // add empty line for visual sepration
        ImGui::TableNextColumn();
        ImGui::Text(" ");
        ImGui::TableNextRow();

        // a lable of the table block
        ImGui::TableNextColumn();
        ImGui::Text("Front face stencil:");
        ImGui::TableNextRow();

        // 9th row (bf stencil fail op)
        if (DrawDropdown(
            "fail op",
            "#bf_stencil_fail_op",
            &bfStencilFailOp,
            stencilOps,
            numStencilOps))
        {
            eDepthStencilProp prop = UI_DSS_BACK_FACE_STENCIL_FAIL_OP;
            pFacade_->UpdateCustomDssParam(prop, bfStencilFailOp);
        }

        // 10th row (bf stencil depth fail op)
        if (DrawDropdown(
            "depth fail op",
            "##bf_stencil_depth_fail_op",
            &bfStencilDepthFailOp,
            stencilOps,
            numStencilOps))
        {
            eDepthStencilProp prop = UI_DSS_BACK_FACE_STENCIL_DEPTH_FAIL_OP;
            pFacade_->UpdateCustomDssParam(prop, bfStencilDepthFailOp);
        }

        // 11th row (bf stencil pass op)
        if (DrawDropdown(
            "pass op",
            "##bf_stencil_pass_op",
            &bfStencilPassOp,
            stencilOps,
            numStencilOps))
        {
            eDepthStencilProp prop = UI_DSS_BACK_FACE_STENCIL_PASS_OP;
            pFacade_->UpdateCustomDssParam(prop, bfStencilPassOp);
        }

        // 12th row (bf stencil func)
        if (DrawDropdown(
            "stencil func",
            "##bf_stencil_func",
            &bfStencilFunc,
            cmpFunctions,
            numCmpFunctions))
        {
            eDepthStencilProp prop = UI_DSS_BACK_FACE_STENCIL_FUNC;
            pFacade_->UpdateCustomDssParam(prop, bfStencilFunc);
        }

        ImGui::EndTable();
    }

    // (for clearness: look at the same condition at the beginning of this method)
    if (dssId != 1)
    {
        ImGui::EndDisabled();
    }
}

//---------------------------------------------------------

bool UIRenderStatesController::DrawRasterStatesSelectors(
    const RsID rsId,
    const MaterialID matId)
{
    bool changedRs = false;

    if (ImGui::TreeNode("Rasterizer states"))
    {
        DrawRasterParamsControl(rsId);

        ImGui::NewLine();
        ImGui::Separator();

        // print a selectable list of rasterizer states
        const cvector<RenderStateName>& statesNames = *pFacade_->GetRenderStateNames(RND_STATES_RASTER);

        for (index i = 0; i < statesNames.size(); ++i)
        {
            if (ImGui::Selectable(statesNames[i].name, i == rsId))
            {
                RenderStateSetup params;
                params.matId      = matId;
                params.rndState   = RND_STATES_RASTER;
                params.rndStateId = (int)i;

                pFacade_->SetMaterialRenderState(params);
                changedRs = true;
            }
        }
        ImGui::TreePop();
    }
    ImGui::Separator();

    return changedRs;
}

//---------------------------------------------------------

bool UIRenderStatesController::DrawBlendStatesSelectors(
    const BsID bsId,
    const MaterialID matId)
{
    bool changedBs = false;

    if (ImGui::TreeNode("Blend states"))
    {
        DrawBlendParamsControl(bsId);

        ImGui::NewLine();
        ImGui::Separator();

        //
        // draw a selectable list of predefined blend states
        //
        const cvector<RenderStateName>& statesNames = *pFacade_->GetRenderStateNames(RND_STATES_BLEND);

        for (index i = 0; i < statesNames.size(); ++i)
        {
            const bool isSelected = (i == bsId);
            if (ImGui::Selectable(statesNames[i].name, isSelected))
            {
                RenderStateSetup params;
                params.matId      = matId;
                params.rndState   = RND_STATES_BLEND;
                params.rndStateId = (int)i;

                pFacade_->SetMaterialRenderState(params);
                changedBs = true;
            }
        }
        ImGui::TreePop();
    }
    ImGui::Separator();

    return changedBs;
}

//---------------------------------------------------------
//---------------------------------------------------------
bool UIRenderStatesController::DrawDepthStencilStatesSelectors(
    const DssID dssId,
    const MaterialID matId)
{
    bool changedDss = false;

    if (ImGui::TreeNode("Depth-stencil states"))
    {
        DrawDepthStencilParamsControl(dssId);

        ImGui::NewLine();
        ImGui::Separator();

        //
        // draw a selectable list of predefined depth-stencil states
        //
        const cvector<RenderStateName>& statesNames = *pFacade_->GetRenderStateNames(RND_STATES_DEPTH_STENCIL);

        for (index i = 0; i < statesNames.size(); ++i)
        {
            if (ImGui::Selectable(statesNames[i].name, i == dssId))
            {
                RenderStateSetup params;
                params.matId      = matId;
                params.rndState   = RND_STATES_DEPTH_STENCIL;
                params.rndStateId = (int)i;

                pFacade_->SetMaterialRenderState(params);
                changedDss = true;
            }
        }
        ImGui::TreePop();
    }
    ImGui::Separator();

    return changedDss;
}


} // namespace
