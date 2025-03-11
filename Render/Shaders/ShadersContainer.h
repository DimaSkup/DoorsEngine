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
#include "shaderclass.h"                // shaders helper class

#include "LightShaderClass.h"           // for light effect on models
#include "DebugShader.h"

#include "colorshaderclass.h"           // for rendering models with only colour but not textures
#include "TextureShader.h"              // for texturing models
#include "fontshaderclass.h"            // for rendering text onto the screen
#include "SkyDomeShader.h"              // for rendering the sky
#include "OutlineShader.h"              // for rendering an outline around model
#include "BillboardShader.h"


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
	};

	struct ShadersContainer
	{
		ColorShaderClass   colorShader_;
		TextureShader      textureShader_;
		LightShaderClass   lightShader_;
		FontShaderClass    fontShader_;

		DebugShader        debugShader_;
		SkyDomeShader      skyDomeShader_;
		OutlineShader      outlineShader_;
		BillboardShader    billboardShader_;
	};
}