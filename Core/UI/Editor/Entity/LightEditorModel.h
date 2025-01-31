// =================================================================================
// Filename:    LightEditorModel.h
// Description: holds light source data for the editor view 
//              (a part of the MVC pattern)
// 
// Created:     30.01.25
// =================================================================================
#pragma once


#include "../../UICommon/Color.h"
#include "../../UICommon/Vectors.h"


namespace Model
{

class EntityDirLight 
{
};

// =================================================================================

class EntityPointLight
{
public:
	ColorRGBA ambient_;
	ColorRGBA diffuse_;
	ColorRGBA specular_;
	Vec3      position_;
	Vec3      attenuation_;
	float     range_ = 0;


public:
	inline void SetData(
		const ColorRGBA& ambient,
		const ColorRGBA& diffuse,
		const ColorRGBA& specular,
		const Vec3& position,
		const Vec3& attenuation,
		const float range)
	{
		ambient_     = ambient;
		diffuse_     = diffuse;
		specular_    = specular;
		position_    = position;
		attenuation_ = attenuation;
		range_       = range;
	}

	inline void GetData(
		ColorRGBA& ambient,
		ColorRGBA& diffuse,
		ColorRGBA& specular,
		Vec3& position,
		Vec3& attenuation,
		float& range) const
	{
		ambient      = ambient_;
		diffuse      = diffuse_;
		specular     = specular_;
		position     = position_;
		attenuation  = attenuation_;
		range        = range_;
	}
};

// =================================================================================

class EntitySpotLight 
{
};
}
