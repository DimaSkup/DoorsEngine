/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: enum_debug_type.h
    Desc:     enumeration of render debug types and other related stuff
              (is used for some editor's tools and for debugging of rendering stuff)

    Created:  20.11.2025  by DimaSkup
\**********************************************************************************/
#pragma once


enum eRndDbgType
{
    DBG_TURN_OFF,               // turn off the debug shader and use the default shader
    DBG_SHOW_NORMALS_0_TO_1,    // normals are packed into range [0,1]
    DBG_SHOW_NORMALS,
    DBG_SHOW_NORMALS_CREATED_FROM_NORMAL_MAP,
    DBG_SHOW_TANGENTS,
    DBG_SHOW_BINORMALS,
    DBG_SHOW_BUMPED_NORMALS,

    DBG_SHOW_ONLY_LIGTHING,
    DBG_SHOW_ONLY_DIRECTED_LIGHTING,
    DBG_SHOW_ONLY_POINT_LIGHTING,
    DBG_SHOW_ONLY_SPOT_LIGHTING,
    DBG_SHOW_ONLY_DIFFUSE_MAP,
    DBG_SHOW_ONLY_NORMAL_MAP,

    DBG_WIREFRAME,

    DBG_SHOW_MATERIAL_AMBIENT,
    DBG_SHOW_MATERIAL_DIFFUSE,
    DBG_SHOW_MATERIAL_SPECULAR,
    DBG_SHOW_MATERIAL_REFLECTION,

    DBG_SHOW_ONLY_AS_ALPHA_CHANNEL,

    NUM_RENDER_DEBUG_TYPES,
};

//---------------------------------------------------------

static const char* g_RndDebugTypeNames[NUM_RENDER_DEBUG_TYPES] =
{
    "default",
    "normals [0,1]",
    "normals [-1,1]",
    "normals created from normal map",
    "tangents",
    "binormals",
    "bumped normals",

    "only lighting",
    "only directed lighting",   // for instance: sun
    "only point lighting",      // for instance: light bulb, candle
    "only spot lighting",       // for instance: flashlight
    "only diffuse map",
    "only normal map",

    "wireframe",

    "material ambient",
    "material diffuse",
    "material specular",
    "material reflection",

    "as alpha channel",
};
