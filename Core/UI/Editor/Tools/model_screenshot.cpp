/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: model_screenshot.cpp
    Created:  04.11.2025 by DimaSkup
\**********************************************************************************/
#include <CoreCommon/pch.h>
#include "model_screenshot.h"
#include <UICommon/IFacadeEngineToUI.h>
#include "../Debug/rnd_dbg_type_switcher.h"
#include <imgui.h>

namespace UI
{

// static ptr to the models names array
static cvector<ModelName>* s_ModelsNamesArr = nullptr;

//---------------------------------------------------------
// Desc:  do some initialization
//---------------------------------------------------------
void ModelScreenshot::Init(IFacadeEngineToUI* pFacade)
{
    if (!pFacade)
    {
        LogErr(LOG, "can't init model screeshot tool: input ptr to UI facade == nullptr");
        return;
    }

    pFacade_ = pFacade;

    // init a frame buffer into which we will render a chosen model
    pFacade_->SetModelPreviewParam(FRAME_BUF_WIDTH,  480.0f);
    pFacade_->SetModelPreviewParam(FRAME_BUF_HEIGHT, 640.0f);

    const uint width  = (uint)pFacade_->GetModelPreviewParam(FRAME_BUF_WIDTH);
    const uint height = (uint)pFacade_->GetModelPreviewParam(FRAME_BUF_HEIGHT);

    pFacade_->InitModelFrameBuf(width, height);
}

//---------------------------------------------------------

void Checkbox(
    IFacadeEngineToUI* pFacade,
    const char* text,
    const eModelPreviewParams param)
{
    if (!text || text[0] == '\0')
    {
        LogErr(LOG, "can't draw checkbox: input text is empty!");
        return;
    }

    bool flag = (bool)pFacade->GetModelPreviewParam(param);

    if (ImGui::Checkbox(text, &flag))
        pFacade->SetModelPreviewParam(param, (float)flag);
}

//---------------------------------------------------------

void DragFloat3(
    IFacadeEngineToUI* pFacade,
    const char* text,
    const eModelPreviewParams param0,
    const eModelPreviewParams param1,
    const eModelPreviewParams param2,
    const float speed = 0.01f)
{
    if (!text || text[0] == '\0')
    {
        LogErr(LOG, "can't draw float3: input text is empty!");
        return;
    }

    Vec3 vec;
    vec.x = pFacade->GetModelPreviewParam(param0);
    vec.y = pFacade->GetModelPreviewParam(param1);
    vec.z = pFacade->GetModelPreviewParam(param2);

    if (ImGui::DragFloat3(text, vec.xyz, speed))
    {
        pFacade->SetModelPreviewParam(param0, vec.x);
        pFacade->SetModelPreviewParam(param1, vec.y);
        pFacade->SetModelPreviewParam(param2, vec.z);
    }


    // do some stuff according to specific value of input param
    switch (param0)
    {
        // prevent crash when camera pos == Vec3(0,0,0)
        case CAMERA_POS_X:
        {
            if (vec.x == 0 && vec.y == 0 && vec.z == 0)
                pFacade->SetModelPreviewParam(param2, vec.z + 0.01f);
            break;
        }

        // draw helper buttons to rotate model/camera exactly by 45 degrees
        case MODEL_ROT_X:
        case CAMERA_ROT_X:
        {
            ImGui::PushID(text);

            if (ImGui::Button("rX -45"))
                pFacade->SetModelPreviewParam(param0, vec.x - DEG_TO_RAD(45));
            ImGui::SameLine();

            if (ImGui::Button("rX +45"))
                pFacade->SetModelPreviewParam(param0, vec.x + DEG_TO_RAD(45));
            ImGui::SameLine();

            if (ImGui::Button("rY -45"))
                pFacade->SetModelPreviewParam(param1, vec.y - DEG_TO_RAD(45));
            ImGui::SameLine();

            if (ImGui::Button("rY +45"))
                pFacade->SetModelPreviewParam(param1, vec.y + DEG_TO_RAD(45));

            ImGui::PopID();

            break;
        }
    }
}

//---------------------------------------------------------

void DragFloat(
    IFacadeEngineToUI* pFacade,
    const char* text,
    const eModelPreviewParams param,
    const float speed = 0.01f,
    const float minVal = EPSILON_E6)
{
    if (!text || text[0] == '\0')
    {
        LogErr(LOG, "can't draw float: input text is empty!");
        return;
    }

    float val = pFacade->GetModelPreviewParam(param);

    if (ImGui::DragFloat(text, &val, speed, minVal))
        pFacade->SetModelPreviewParam(param, val);
}

//---------------------------------------------------------

inline void DragColor(
    IFacadeEngineToUI* pFacade,
    const char* text,
    const eModelPreviewParams rParam,  // channel R
    const eModelPreviewParams gParam,  // channel G
    const eModelPreviewParams bParam)  // channel B
{
    float rgb[3] = {
        pFacade->GetModelPreviewParam(rParam),
        pFacade->GetModelPreviewParam(gParam),
        pFacade->GetModelPreviewParam(bParam),
    };

    if (ImGui::ColorEdit3(text, rgb))
    {
        pFacade->SetModelPreviewParam(rParam, rgb[0]);
        pFacade->SetModelPreviewParam(gParam, rgb[1]);
        pFacade->SetModelPreviewParam(bParam, rgb[2]);
    }
}

//---------------------------------------------------------

bool DragUint(
    IFacadeEngineToUI* pFacade,
    const char* text,
    const eModelPreviewParams param,
    const float speed = 0.01f)
{
    if (!text || text[0] == '\0')
    {
        LogErr(LOG, "can't draw UINT: input text is empty!");
        return false;
    }

    int val = (int)pFacade->GetModelPreviewParam(param);

    if (ImGui::DragInt(text, &val, speed))
    {
        pFacade->SetModelPreviewParam(param, (float)val);
        return true;
    }

    return false;
}

//---------------------------------------------------------

void RenderModelPreview(
    IFacadeEngineToUI* pFacade,
    const float width,
    const float height)
{
    pFacade->RenderModelFrameBuf();

    ID3D11ShaderResourceView* pSRV = pFacade->GetModelFrameBufView();

    if (pSRV)
        ImGui::Image((ImTextureID)pSRV, { width, height });
}

//---------------------------------------------------------
// Desc:  resize a preview frame buffer (where we see a model)
//---------------------------------------------------------
void ResizePreviewWnd(IFacadeEngineToUI* pFacade)
{
    const uint width  = (uint)pFacade->GetModelPreviewParam(FRAME_BUF_WIDTH);
    const uint height = (uint)pFacade->GetModelPreviewParam(FRAME_BUF_HEIGHT);

    // reinit
    pFacade->InitModelFrameBuf(width, height);
}

//---------------------------------------------------------
// Desc:  make a screenshot of a model from different views (or from only one)
//        and create a texture atlas of these screenshots
//
// Args:  - filename:  where to save a texture atlas
//---------------------------------------------------------
void MakeScreenshotAtlas(const char* filename, IFacadeEngineToUI* pFacade)
{
    const uint width  = (uint)pFacade->GetModelPreviewParam(FRAME_BUF_WIDTH);
    const uint height = (uint)pFacade->GetModelPreviewParam(FRAME_BUF_HEIGHT);

    // save for restoring after screenshot creation
    float prevRotation = pFacade->GetModelPreviewParam(CAMERA_ROT_Y);

    const uint frames = 8;
    const float angleStep = 360.0f / frames;

    pFacade->CreateEmptyTexAtlas(width, height, frames);

    // render model from different views and push each "screenshot" into atlas
    for (uint i = 0; i < frames; ++i)
    {
        const float angle = (i * angleStep);
                
        pFacade->SetModelPreviewParam(CAMERA_ROT_Y, DEG_TO_RAD(angle));
        pFacade->RenderModelFrameBuf();
        pFacade->PushTexIntoAtlas(pFacade->GetModelFrameBufView());
    }

    pFacade->SaveTexAtlasToFile(filename);
    pFacade->ClearTexAtlasMemory();

    // restore rotation
    pFacade->SetModelPreviewParam(CAMERA_ROT_Y, prevRotation);
}

//---------------------------------------------------------

void RenderEditingFields(IFacadeEngineToUI* pFacade)
{
    // control fields
    Checkbox(pFacade, "Use ortho matrix", USE_ORTHO_MATRIX);

    ImGui::Text("Model");
    DragFloat3(pFacade, "pos",       MODEL_POS_X, MODEL_POS_Y, MODEL_POS_Z);
    DragFloat3(pFacade, "rot",       MODEL_ROT_X, MODEL_ROT_Y, MODEL_ROT_Z, DEG_TO_RAD(5));

    DragFloat (pFacade, "scale",     MODEL_SCALE, 0.001f, 0.0001f);

    ImGui::Text("Camera");
    DragFloat3(pFacade, "cam pos",   CAMERA_POS_X, CAMERA_POS_Y, CAMERA_POS_Z, 0.05f);
    DragFloat3(pFacade, "cam rot",   CAMERA_ROT_X, CAMERA_ROT_Y, CAMERA_ROT_Z, DEG_TO_RAD(5));

    ImGui::Text("Frame buffer (and screenshot)");

    bool resize = false;
    resize |= DragUint (pFacade, "width",  FRAME_BUF_WIDTH);
    resize |= DragUint (pFacade, "height", FRAME_BUF_HEIGHT);

    DragColor(pFacade,  "bg color",  BG_COLOR_R, BG_COLOR_G, BG_COLOR_B);
    DragFloat(pFacade,  "ortho view height", ORTHO_MATRIX_VIEW_HEIGHT, 0.001f, 0.001f);


    // handle some stuff
    if (resize)
        ResizePreviewWnd(pFacade);
}

//---------------------------------------------------------
// Desc:  render a selectable list of models names
//---------------------------------------------------------
void RenderModelsNamesList(IFacadeEngineToUI* pFacade, int& selectedIdx)
{
    if (ImGui::TreeNode("Select model"))
    {
        constexpr ImGuiSelectableFlags_ flags = ImGuiSelectableFlags_DontClosePopups;
        const cvector<ModelName>&       names = *pFacade->GetModelsNamesArrPtr();

        for (int i = 0; const ModelName& name : names)
        {
            if (ImGui::Selectable(name.name, i == selectedIdx, flags))
            {
                selectedIdx = i;
                const ModelID modelId = pFacade->GetModelIdByName(name.name);

                pFacade->SetModelPreviewParam(MODEL_ID, (float)modelId);
            }

            ++i;
        }
        ImGui::TreePop();
    }
}

//---------------------------------------------------------
// Desc:  draw a list for rendering debug types selection
//        (for instance: show only diffuse tex, or normal vectors as colors, etc.)
//---------------------------------------------------------
inline void RenderDebugTypesList(IFacadeEngineToUI* pFacade)
{
    if (ImGui::TreeNode("Select rendering debug"))
    {
        // switch btw different rendering states
        DrawRndDebugTypesSwitcher(pFacade);
        ImGui::TreePop();
    }
}

//---------------------------------------------------------
// Desc:  render a window for making model's screenshots
//---------------------------------------------------------
void ModelScreenshot::Render(bool* pOpen)
{
    if (!pFacade_)
    {
        LogErr(LOG, "dude, you forgot to init ptr to facade interface!");
        return;
    }

    // always center this window when appearing
    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    const float sizeX   = ImGui::GetMainViewport()->Size.x * 0.5f;
    const float sizeY   = ImGui::GetMainViewport()->Size.y * 0.66f;
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(sizeX, sizeY), ImGuiCond_Appearing);


    if (ImGui::Begin("Model screenshot", pOpen, ImGuiWindowFlags_NoScrollbar))
    {
        const float imgWidth     = pFacade_->GetModelPreviewParam(FRAME_BUF_WIDTH);
        const float imgHeight    = pFacade_->GetModelPreviewParam(FRAME_BUF_HEIGHT);

        const ImVec2 wndPos      = ImGui::GetWindowPos();
        const ImVec2 availReg    = ImGui::GetContentRegionAvail();
        const ImVec2 padding     = { imgWidth + 20.0f, 40.0f };

        // upper left corner and size for both sides
        const ImVec2 leftPos     = wndPos + ImVec2(0.0f, padding.y);
        const ImVec2 rightPos    = wndPos + padding;

        const ImVec2 leftSideSz  = { imgWidth, imgHeight };
        const ImVec2 rightSideSz = { availReg.x - padding.x, availReg.y };


        // LEFT side: model preview
        ImGui::SetNextWindowPos(leftPos);
        if (ImGui::BeginChild("Preview", leftSideSz))
        {
            RenderModelPreview(pFacade_, imgWidth, imgHeight);
        }
        ImGui::EndChild();


        // RIGHT side: control fields
        ImGui::SetNextWindowPos(rightPos);
        if (ImGui::BeginChild("Control fields", rightSideSz))
        {
            if (ImGui::Button("Make screenshot"))
                MakeScreenshotAtlas("screenshot.dds", pFacade_);

            RenderEditingFields(pFacade_);
            RenderModelsNamesList(pFacade_, selectedModelIdx_);
            RenderDebugTypesList(pFacade_);
        }
        ImGui::EndChild();
    }
    ImGui::End();
}


} // namespace
