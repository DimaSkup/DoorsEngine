// ********************************************************************************
// Filename:    TextureHelperTypes.h
// Description: contains some common types and data 
//              for functional related to textures 
// 
// Created:     05.06.24
// ********************************************************************************
#pragma once

#include <cstdint>
#include <string>
#include <d3d11.h>

using SRV = ID3D11ShaderResourceView;

const uint32_t TEXTURE_TYPE_COUNT = 22;            // the number of aiTextureType elements
//const TexID    INVALID_TEXTURE_ID = 0;           


static const std::string namesOfTexTypes[TEXTURE_TYPE_COUNT] =
{
	// to understand wtf you need to look at the enum aiTextureType 
	// inside of material.h of the ASSIMP
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
	"TRANSMISSION",
};

static const std::string namesOfTexTypes2[TEXTURE_TYPE_COUNT] =
{
	// to understand wtf you need to look at the enum aiTextureType 
	// inside of material.h of the ASSIMP
	"none",
	"DiffuseMap",
	"SpecularMap",
	"AmbientMap",
	"EmissiveMap",
	"HeightMap",
	"NormalMap",
	"ShininessMap",
	"OpacityMap",
	"DisplacementMap",
	"LightMap",
	"ReflectionMap",
	"BaseColorMap",
	"NormalCameraMap",
	"EmissionColorMap",
	"MetalnessMap",
	"DiffuseRoughnessMap",
	"AmbientOcclusionMap",
	"Unknown",
	"SheenMap",
	"ClearCoatMap",
	"TransmissionMap",
};


// enumaration of textures types
enum TexType
{
	// to understand wtf you need to look at the enum aiTextureType 
	// inside of material.h of the ASSIMP
	NONE,
	DIFFUSE,
	SPECULAR,
	AMBIENT,
	EMISSIVE,
	HEIGHT,
	NORMALS,
	SHININESS,
	OPACITY,
	DISPLACEMENT,
	LIGHTMAP,
	REFLECTION,
	BASE_COLOR,
	NORMAL_CAMERA,
	EMISSION_COLOR,
	METALNESS,
	DIFFUSE_ROUGHNESS,
	AMBIENT_OCCLUSION,
	UNKNOWN,
	SHEEN,
	CLEARCOAT,
	TRANSMISSION,
	LAST
};