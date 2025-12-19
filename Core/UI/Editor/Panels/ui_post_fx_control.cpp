/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: ui_post_fx_control.cpp

    Created:  07.11.2025 by DimaSkup
\**********************************************************************************/
#include "ui_post_fx_control.h"
#include <UICommon/IFacadeEngineToUI.h>
#include <post_fx_enum.h>
#include <imgui.h>

namespace UI
{

//---------------------------------------------------------
// flags to define if we need to draw post effects control fields
//---------------------------------------------------------
bool s_ShowControl[NUM_POST_EFFECTS]{ 0 };

//---------------------------------------------------------
// Arr of pointers to post effect params controller [post_fx => func]
//---------------------------------------------------------
void (*FuncDrawControlPostFx[NUM_POST_EFFECTS])(IFacadeEngineToUI*) { nullptr };


//---------------------------------------------------------
// Desc:  change a single post effect's parameter
//---------------------------------------------------------
inline void DragFloat(
    IFacadeEngineToUI* pFacade,
    const char* text,
    const ePostFxParam param)
{
    float value = pFacade->GetPostFxParam(param);

    if (ImGui::DragFloat(text, &value, 0.01f))
        pFacade->SetPostFxParam(param, value);
}

//---------------------------------------------------------
// Desc:  change some post effect's color parameter (3 floats)
//---------------------------------------------------------
inline void DragColor(
    IFacadeEngineToUI* pFacade,
    const char* text,
    const ePostFxParam rParam,  // channel R
    const ePostFxParam gParam,  // channel G
    const ePostFxParam bParam)  // channel B
{
    float rgb[3] = {
        pFacade->GetPostFxParam(rParam),
        pFacade->GetPostFxParam(gParam),
        pFacade->GetPostFxParam(bParam),
    };

    if (ImGui::ColorEdit3(text, rgb))
    {
        pFacade->SetPostFxParam(rParam, rgb[0]);
        pFacade->SetPostFxParam(gParam, rgb[1]);
        pFacade->SetPostFxParam(bParam, rgb[2]);
    }
}

//===================================================================
// DRAW CONTROL FIELDS FOR POST EFFECTS
//===================================================================

void DrawControlPostFX_ShockwaveDistortion(IFacadeEngineToUI* pFacade)
{
    DragFloat(pFacade, "centerX (0..1)", POST_FX_PARAM_SHOCKWAVE_DISTORT_CX);
    DragFloat(pFacade, "centerY (0..1)", POST_FX_PARAM_SHOCKWAVE_DISTORT_CY);
    DragFloat(pFacade, "speed",          POST_FX_PARAM_SHOCKWAVE_DISTORT_SPEED);
    DragFloat(pFacade, "thickness",      POST_FX_PARAM_SHOCKWAVE_DISTORT_THICKNESS);
    DragFloat(pFacade, "amplitude",      POST_FX_PARAM_SHOCKWAVE_DISTORT_AMPLITUDE);
    DragFloat(pFacade, "sharpness",      POST_FX_PARAM_SHOCKWAVE_DISTORT_SHARPNESS);
}

//---------------------------------------------------------

void DrawControlPostFx_Glitch(IFacadeEngineToUI* pFacade)
{
    DragFloat(pFacade, "intensity (0..1)",    POST_FX_PARAM_GLITCH_INTENSITY);
    DragFloat(pFacade, "color split",         POST_FX_PARAM_GLITCH_COLOR_SPLIT);
    DragFloat(pFacade, "block size",          POST_FX_PARAM_GLITCH_BLOCK_SIZE);
    DragFloat(pFacade, "speed",               POST_FX_PARAM_GLITCH_SPEED);
    DragFloat(pFacade, "scanline (0..1)",     POST_FX_PARAM_GLITCH_SCANLINE);
    DragFloat(pFacade, "noise amount (0..1)", POST_FX_PARAM_GLITCH_NOISE_AMOUNT);
}

//---------------------------------------------------------

void DrawControlPostFx_Posterization(IFacadeEngineToUI* pFacade)
{
    DragFloat(pFacade, "levels", POST_FX_PARAM_POSTERIZATION_LEVELS);
}

//---------------------------------------------------------

void DrawControlPostFx_BloomExtract(IFacadeEngineToUI* pFacade)
{
    DragFloat(pFacade, "threshold", POST_FX_PARAM_BLOOM_THRESHOLD);
}

//---------------------------------------------------------

void DrawControlPostFx_BrightContrastAdj(IFacadeEngineToUI* pFacade)
{
    DragFloat(pFacade, "brightness", POST_FX_PARAM_BRIGHTNESS);
    DragFloat(pFacade, "contrast",   POST_FX_PARAM_CONTRAST);
}

//---------------------------------------------------------

void DrawControlPostFx_SimpleChromaticAberraion(IFacadeEngineToUI* pFacade)
{
    DragFloat(pFacade, "strength", POST_FX_PARAM_CHROMATIC_ABERRATION_STRENGTH);
}

//---------------------------------------------------------

void DrawControlPostFx_ColorShift(IFacadeEngineToUI* pFacade)
{
    DragFloat(pFacade, "hue", POST_FX_PARAM_COLOR_SHIFT_HUE);
}

//---------------------------------------------------------

void DrawControlPostFx_ColorSplit(IFacadeEngineToUI* pFacade)
{
    DragFloat(pFacade, "centerX (0..1)", POST_FX_PARAM_COLOR_SPLIT_CX);
    DragFloat(pFacade, "centerY (0..1)", POST_FX_PARAM_COLOR_SPLIT_CY);
    DragFloat(pFacade, "intensity",      POST_FX_PARAM_COLOR_SPLIT_INTENSITY);
    DragFloat(pFacade, "radial power",   POST_FX_PARAM_COLOR_SPLIT_RADIAL_POWER);

    DragColor(
        pFacade,
        "chromatic mul",
        POST_FX_PARAM_COLOR_SPLIT_CHROMA_MUL_R,
        POST_FX_PARAM_COLOR_SPLIT_CHROMA_MUL_G,
        POST_FX_PARAM_COLOR_SPLIT_CHROMA_MUL_B);

    DragFloat(pFacade, "samples",        POST_FX_PARAM_COLOR_SPLIT_SAMPLES);
}

//---------------------------------------------------------

void DrawControlPostFx_ColorTint(IFacadeEngineToUI* pFacade)
{
    DragColor(
        pFacade,
        "color",
        POST_FX_PARAM_COLOR_TINT_R,
        POST_FX_PARAM_COLOR_TINT_G,
        POST_FX_PARAM_COLOR_TINT_B);

    DragFloat(pFacade, "intensity", POST_FX_PARAM_COLOR_TINT_INTENSITY);
}

//---------------------------------------------------------

void DrawControlPostFx_CrtCurvature(IFacadeEngineToUI* pFacade)
{
    DragFloat(pFacade, "curvature", POST_FX_PARAM_CRT_CURVATURE);
}

//---------------------------------------------------------

void DrawControlPostFx_FilmGrain(IFacadeEngineToUI* pFacade)
{
    DragFloat(pFacade, "strength", POST_FX_PARAM_FILM_GRAIN_STRENGTH);
}

//---------------------------------------------------------

void DrawControlPostFx_FrostGlassBlur(IFacadeEngineToUI* pFacade)
{
    DragFloat(pFacade, "strength", POST_FX_PARAM_FROST_GLASS_BLUR_STRENGTH);
}

//---------------------------------------------------------

void DrawControlPostFx_HeatDistortion(IFacadeEngineToUI* pFacade)
{
    DragFloat(pFacade, "strength", POST_FX_PARAM_HEAT_DISTORT_STRENGTH);
}

//---------------------------------------------------------

void DrawControlPostFx_NegativeGlow(IFacadeEngineToUI* pFacade)
{
    DragFloat(pFacade, "strength",  POST_FX_PARAM_NEGATIVE_GLOW_STRENGTH);
    DragFloat(pFacade, "threshold", POST_FX_PARAM_NEGATIVE_GLOW_THRESHOLD);
}

//---------------------------------------------------------

void DrawControlPostFx_Pixelation(IFacadeEngineToUI* pFacade)
{
    DragFloat(pFacade, "pixel size", POST_FX_PARAM_PIXELATION_PIXEL_SIZE);
}

//---------------------------------------------------------

void DrawControlPostFx_RadialBlur(IFacadeEngineToUI* pFacade)
{
    DragFloat(pFacade, "centerX (0..1)", POST_FX_PARAM_RADIAL_BLUR_CX);
    DragFloat(pFacade, "centerY (0..1)", POST_FX_PARAM_RADIAL_BLUR_CY);
    DragFloat(pFacade, "samples",        POST_FX_PARAM_RADIAL_BLUR_SAMPLES);
    DragFloat(pFacade, "strength",       POST_FX_PARAM_RADIAL_BLUR_STRENGTH);
}

//---------------------------------------------------------

void DrawControlPostFx_SwirlDistortion(IFacadeEngineToUI* pFacade)
{
    DragFloat(pFacade, "centerX (0..1)",  POST_FX_PARAM_SWIRL_DISTORT_CX);
    DragFloat(pFacade, "centerY (0..1)",  POST_FX_PARAM_SWIRL_DISTORT_CY);
    DragFloat(pFacade, "radius (in UV)",  POST_FX_PARAM_SWIRL_DISTORT_RADIUS);
    DragFloat(pFacade, "angle (radians)", POST_FX_PARAM_SWIRL_DISTORT_ANGLE);
}

//---------------------------------------------------------

void DrawControlPostFx_OldTvDistortion(IFacadeEngineToUI* pFacade)
{
    DragFloat(pFacade, "strength", POST_FX_PARAM_OLD_TV_DISTORT_STRENGTH);
}

//---------------------------------------------------------

void DrawControlPostFx_Vignette(IFacadeEngineToUI* pFacade)
{
    DragFloat(pFacade, "strength", POST_FX_PARAM_VIGNETTE_STRENGTH);
}

//---------------------------------------------------------
// Desc:  render info about currenly used post effects
//---------------------------------------------------------
void DrawPostFxsQueueInfo(
    IFacadeEngineToUI* pFacade,
    const ePostFxType* fxsQueue,
    const int numPostFxsInQueue)
{
    ImGui::Text("POST FX QUEUE (%d / %d):", numPostFxsInQueue, MAX_NUM_POST_EFFECTS);

    for (int i = 0; i < numPostFxsInQueue; ++i)
    {
        const ePostFxType type = fxsQueue[i];

        // remove this post fx from rendering queue
        ImGui::PushID(i);
        if (ImGui::Button("Remove"))
            pFacade->RemovePostFx(i);

        ImGui::SameLine();

        // show/hide control fields using an arrow
        ImGuiDir arrowDir = (!s_ShowControl[type]) ? ImGuiDir_Down : ImGuiDir_Up;

        if (ImGui::ArrowButton("", arrowDir))
            s_ShowControl[type] = !s_ShowControl[type];

        ImGui::SameLine();
        ImGui::Text("%d: %s", i, g_PostFxsNames[type]);

        // draw control fields
        if (s_ShowControl[type])
        {
            if (FuncDrawControlPostFx[type] != nullptr)
                FuncDrawControlPostFx[type](pFacade);
            else
                ImGui::Text("IMPLEMENT FUNC FOR: %s", g_PostFxsNames[type]);
        }
        ImGui::PopID();
    }
}

//---------------------------------------------------------
// Desc:  render a list of available post effects to push into the render queue
//---------------------------------------------------------
void DrawListOfAvailablePostFxs(
    IFacadeEngineToUI* pFacade,
    const ePostFxType* fxsQueue,
    const int numPostFxsInQueue)
{
    int availablePostFxsIdxs[NUM_POST_EFFECTS];

    for (int i = 0; i < (int)NUM_POST_EFFECTS; ++i)
        availablePostFxsIdxs[i] = i;

    // remove used post fxs from the list of available ones
    for (int i = 0; i < numPostFxsInQueue; ++i)
    {
        ePostFxType type = fxsQueue[i];
        availablePostFxsIdxs[type] = -1;
    }

    // render list
    for (int i = 0; i < NUM_POST_EFFECTS; ++i)
    {
        if (availablePostFxsIdxs[i] == -1)
            continue;

        ImGui::PushID(i);
        if (ImGui::Button("Add"))
            pFacade->PushPostFx(ePostFxType(i));

        ImGui::PopID();
        ImGui::SameLine();
        ImGui::Text("%s", g_PostFxsNames[i]);
    }
}

//---------------------------------------------------------
// Desc:  default constructor
//---------------------------------------------------------
UI_PostFxControl::UI_PostFxControl()
{
    // bind effects control handlers
    FuncDrawControlPostFx[POST_FX_SHOCKWAVE_DISTORTION] = DrawControlPostFX_ShockwaveDistortion;
    FuncDrawControlPostFx[POST_FX_POSTERIZATION]        = DrawControlPostFx_Posterization;
    FuncDrawControlPostFx[POST_FX_BRIGHT_CONTRAST_ADJ]  = DrawControlPostFx_BrightContrastAdj;
    FuncDrawControlPostFx[POST_FX_BLOOM_BRIGHT_EXTRACT] = DrawControlPostFx_BloomExtract;
    FuncDrawControlPostFx[POST_FX_CHROMATIC_ABERRATION] = DrawControlPostFx_SimpleChromaticAberraion;
    FuncDrawControlPostFx[POST_FX_COLOR_SHIFT]          = DrawControlPostFx_ColorShift;
    FuncDrawControlPostFx[POST_FX_COLOR_SPLIT]          = DrawControlPostFx_ColorSplit;
    FuncDrawControlPostFx[POST_FX_COLOR_TINT]           = DrawControlPostFx_ColorTint;
    FuncDrawControlPostFx[POST_FX_CRT_SCANLINES]        = DrawControlPostFx_CrtCurvature;

    FuncDrawControlPostFx[POST_FX_FILM_GRAIN]           = DrawControlPostFx_FilmGrain;
    FuncDrawControlPostFx[POST_FX_FROST_GLASS_BLUR]     = DrawControlPostFx_FrostGlassBlur;
    FuncDrawControlPostFx[POST_FX_GLITCH]               = DrawControlPostFx_Glitch;
    FuncDrawControlPostFx[POST_FX_HEAT_DISTORTION]      = DrawControlPostFx_HeatDistortion;
    FuncDrawControlPostFx[POST_FX_NEGATIVE_GLOW]        = DrawControlPostFx_NegativeGlow;
    FuncDrawControlPostFx[POST_FX_PIXELATION]           = DrawControlPostFx_Pixelation;

    FuncDrawControlPostFx[POST_FX_RADIAL_BLUR]          = DrawControlPostFx_RadialBlur;
    FuncDrawControlPostFx[POST_FX_SWIRL_DISTORTION]     = DrawControlPostFx_SwirlDistortion;
    FuncDrawControlPostFx[POST_FX_OLD_TV_DISTORTION]    = DrawControlPostFx_OldTvDistortion;
    FuncDrawControlPostFx[POST_FX_VIGNETTE_EFFECT]      = DrawControlPostFx_Vignette;
}

//---------------------------------------------------------
// Desc:  draw GUI for work with post effects
//---------------------------------------------------------
void UI_PostFxControl::Draw(IFacadeEngineToUI* pFacade)
{
    assert(pFacade && "input ptr to UI facade == nullptr");


    // enable/disable depth visualization
    bool enableDepthVis = pFacade->IsEnabledDepthVisualize();

    if (ImGui::Checkbox("Enable depth visualization", &enableDepthVis))
    {
        pFacade->EnablePostFxs(false);
        pFacade->EnableDepthVisualize(enableDepthVis);
    }


    // enable/disable using of post effects
    bool enablePostFXs = pFacade->IsPostFxsEnabled();

    if (ImGui::Checkbox("Enable PostFX", &enablePostFXs))
    {
        pFacade->EnablePostFxs(enablePostFXs);
        pFacade->EnableDepthVisualize(false);
    }
    // show a list of enabled post effects from the rendering queue
    const ePostFxType* fxsQueue = nullptr;
    const int numUsedPostFxs = (int)pFacade->GetNumUsedPostFxs();

    pFacade->GetPostFxsQueue((const void**)&fxsQueue);
    ImGui::Separator();

    DrawPostFxsQueueInfo(pFacade, fxsQueue, numUsedPostFxs);
    ImGui::Separator();

    if (ImGui::TreeNode("Available PostFx:"))
    {
        DrawListOfAvailablePostFxs(pFacade, fxsQueue, numUsedPostFxs);
        ImGui::TreePop();
    }
}

} // namespace
