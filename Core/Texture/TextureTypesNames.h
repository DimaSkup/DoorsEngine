// =================================================================================
#pragma once


#include <CoreCommon/Types.h>


static const std::string namesOfTexTypes[NUM_TEXTURE_TYPES] =
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

///////////////////////////////////////////////////////////

static const std::string namesOfTexTypes2[NUM_TEXTURE_TYPES] =
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

