/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: post_fx_enum.h
    Desc:     enum of available post effects

    why is it here? in Shared? because we need this enum
    both for editor's GUI and for rendering post effects

    Created:  05.11.2025 by DimaSkup
\**********************************************************************************/
#pragma once
#include "post_fx_params_enum.h"

//---------------------------------------------------------
// post effects names
//---------------------------------------------------------
static const char* g_PostFxsNames[] =
{
    "Bloom bright extract",     
    "Brightnes/contrast adj",   
    "Chromatic aberration",     
    "Color shift",              
    "Color split",

    "Color tint",               
    "CRT curve + scanlines",
    "Dithering: ordered",
    "Edge detection",           
    "Film grain",               
    "Frosted glass blur",

    "Gaussian blur",            
    "Glitch",         
    "Grayscale",                
    "Heat distortion",         
    "Invert colors",

    "Negative glow",           
    "Night vision",             
    "Old TV distortion",
    "Pixelation",               
    "Posterization",

    "Radial blur",              
    "Sepia Tone",               
    "Shockwave distortion",     
    "Swirl distortion",         
    "Thermal vision",

    "Vignette effect",          
    "Visualize depth",          
};

//---------------------------------------------------------
// post effects types
//---------------------------------------------------------
enum ePostFxType
{
    POST_FX_BLOOM_BRIGHT_EXTRACT,
    POST_FX_BRIGHT_CONTRAST_ADJ,

    POST_FX_CHROMATIC_ABERRATION,
    POST_FX_COLOR_SHIFT,           // hue rotation
    POST_FX_COLOR_SPLIT,           // chromatic aberration - strong
    POST_FX_COLOR_TINT,
    POST_FX_CRT_SCANLINES,         // CRT curvature + Scanlines: retro CRT display simulation.

    POST_FX_DITHERING_ORDERED,
    POST_FX_EDGE_DETECTION,        // sobel filter
    POST_FX_FILM_GRAIN,            // Adds subtle noise for a cinematic feel
    POST_FX_FROST_GLASS_BLUR,
    POST_FX_GAUSSIAN_BLUR,
    POST_FX_GLITCH,

    POST_FX_GRAYSCALE,
    POST_FX_HEAT_DISTORTION,       // mirage
    POST_FX_INVERT_COLORS,
    POST_FX_NEGATIVE_GLOW,         // inverted bloom

    POST_FX_NIGHT_VISION,          // green filter + noise
    POST_FX_OLD_TV_DISTORTION,     // wobble + noise
    POST_FX_PIXELATION,            // low-resolution look
    POST_FX_POSTERIZATION,         // Reduces the number of color levels to create a stylized “cartoon” look

    POST_FX_RADIAL_BLUR,
    POST_FX_SEPIA,
    POST_FX_SHOCKWAVE_DISTORTION,
    POST_FX_SWIRL_DISTORTION,
    POST_FX_THERMAL_VISION,
    POST_FX_VIGNETTE_EFFECT,
    POST_FX_VISUALIZE_DEPTH,

    NUM_POST_EFFECTS
};

//---------------------------------------------------------
// post fx to shader:
// enum value from ePostFxType serves as index to responsible shader name
//---------------------------------------------------------
static const char* g_PostFxShaderName[NUM_POST_EFFECTS] =
{
    "postFX_BloomBrightExtract",
    "postFX_BrightContrastAdjust",
    "postFX_ChromaticAberration",
    "postFX_ColorShift",
    "postFX_ColorSplit",

    "postFX_ColorTint",
    "postFX_CRT_and_Scanlines",
    "postFX_DitheringOrdered",
    "postFX_EdgeDetection",
    "postFX_FilmGrain",
    "postFX_FrostGlassBlur",

    "postFX_GaussianBlur",
    "postFX_Glitch",
    "postFX_Grayscale",
    "postFX_HeatDistortion",
    "postFX_InvertColors",

    "postFX_NegativeGlow",
    "postFX_NightVision",
    "postFX_old_TV_distortion",
    "postFX_Pixelation",
    "postFX_Posterization",

    "postFX_RadialBlur",
    "postFX_Sepia",
    "postFX_ShockwaveDistortion",
    "postFX_SwirlDistortion",
    "postFX_ThermalVision",

    "postFX_VignetteEffect",
    "VisualizeDepthShader",
};
