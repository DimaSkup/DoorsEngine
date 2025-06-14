// =================================================================================
// Filename:        ShadersContainer.h
// Description:     
//
// Created:         16.03.23
// =================================================================================
#pragma once


//
// INCLUDES
//
#include "LightShader.h"                // for light effect on models
#include "DebugShader.h"

#include "ColorShader.h"                // for rendering models with only colour but not textures
#include "TextureShader.h"              // for texturing models
#include "FontShader.h"                 // for rendering text onto the screen
#include "SkyDomeShader.h"              // for rendering the sky
#include "OutlineShader.h"              // for rendering an outline around model
#include "BillboardShader.h"
#include "MaterialIconShader.h"         // is used for rendering material icon (sphere + single material) which is showing in the editor's material browser
#include "TerrainShader.h"

namespace Render
{
	enum ShaderTypes
	{
		COLOR,
		TEXTURE,
		LIGHT,
		FONT,
		DEBUG,
		SKY_DOME,
		OUTLINE,
		BILLBOARD,
        MATERIAL_ICON,
        TERRAIN,
	};

	struct ShadersContainer
	{
		ColorShader         colorShader_;
		TextureShader       textureShader_;
		LightShader         lightShader_;
		FontShader          fontShader_;

		DebugShader         debugShader_;
		SkyDomeShader       skyDomeShader_;
		OutlineShader       outlineShader_;
		BillboardShader     billboardShader_;
        MaterialIconShader  materialIconShader_;

        TerrainShader       terrainShader_;
	};
}
