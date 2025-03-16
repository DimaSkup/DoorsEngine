// =================================================================================
// Filename:    LightEditorModel.h
// Description: holds light source data for the editor view 
//              (a part of the MVC pattern)
// 
// Created:     30.01.25
// =================================================================================
#pragma once

#include <CoreCommon/MathHelper.h>
#include <UICommon/Color.h>
#include <UICommon/Vectors.h>


namespace UI
{

// =================================================================================
// Directed light data (for instance: sun)
// =================================================================================

struct DirectedLightData
{
    ColorRGBA ambient;
    ColorRGBA diffuse;
    ColorRGBA specular;
    Vec3      direction;
};

///////////////////////////////////////////////////////////

class ModelEntityDirLight 
{
public:
    DirectedLightData data_;

public:
    inline void SetData(
        const ColorRGBA& ambient,
        const ColorRGBA& diffuse,
        const ColorRGBA& specular,
        const Vec3& direction)
    {
        data_.ambient   = ambient;
        data_.diffuse   = diffuse;
        data_.specular  = specular;
        data_.direction = direction;
    }

    // ----------------------------------------------------

    inline void GetData(DirectedLightData& outData) const
    {
        outData = data_;
    }
};


// =================================================================================
// Point light data (for instance: light bulb, candle)
// =================================================================================

struct PointLightData
{
	ColorRGBA ambient;
	ColorRGBA diffuse;
	ColorRGBA specular;
	Vec3      position;
	float     range = 0;
	Vec3      attenuation;
};

///////////////////////////////////////////////////////////

class ModelEntityPointLight
{
public:
	PointLightData data_;

public:
	inline void SetData(
		const ColorRGBA& ambient,
		const ColorRGBA& diffuse,
		const ColorRGBA& specular,
		const Vec3& position,
		const float range,
		const Vec3& attenuation)
	{
		data_.ambient     = ambient;
		data_.diffuse     = diffuse;
		data_.specular    = specular;
		data_.position    = position;
		data_.range       = range;
		data_.attenuation = attenuation;
	}

	// ----------------------------------------------------

	inline void GetData(PointLightData& outData) const 
	{ 
		outData = data_;
	}
};


// =================================================================================
// Spotlight data (for instance: flashlight)
// =================================================================================

struct SpotLightData
{
	ColorRGBA ambient;
	ColorRGBA diffuse;
	ColorRGBA specular;

	Vec3 position;
	float range = 0.0f;

	Vec3 direction;
	float spotExp = 0.0f;    // spot exponent: light intensity fallof (for control the spotlight cone)

	Vec3 attenuation;
};

///////////////////////////////////////////////////////////

class ModelEntitySpotLight 
{
public:
	SpotLightData data_;

public:
	inline void SetData(
		const ColorRGBA& ambient,
		const ColorRGBA& diffuse,
		const ColorRGBA& specular,
		const Vec3& pos,
		const Vec3& direction,
		const Vec3& attenuation,
		const float range,
		const float spotExp)
	{
		data_.ambient     = ambient;
		data_.diffuse     = diffuse;
		data_.specular    = specular;
		data_.position    = pos;
        data_.direction   = direction;
		data_.attenuation = attenuation;
		data_.range       = range;
		data_.spotExp     = spotExp;
        

		//SetRotation(directionQuat);
	}

	// ----------------------------------------------------

	void SetRotation(const Vec4& directionQuat)
	{
		// convert input quaternion into roll/pitch/yaw in degrees
		DirectX::XMFLOAT3 pitchYawRoll = MathHelper::QuatToRollPitchYaw(directionQuat.ToXMVector());
	
		data_.direction.x = DirectX::XMConvertToDegrees(pitchYawRoll.x);
		data_.direction.y = DirectX::XMConvertToDegrees(pitchYawRoll.y);
		data_.direction.z = DirectX::XMConvertToDegrees(pitchYawRoll.z);
	}

	// ----------------------------------------------------

	inline void GetData(SpotLightData& outData) const
	{
		outData = data_;
	}
};

} // namespace UI
