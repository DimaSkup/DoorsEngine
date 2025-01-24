// *********************************************************************************
// Filename:     Light.h
// Description:  contains different data structures for lighting computations
//
// Created:      29.03.24 (moved into ECS at 22.08.24)
// *********************************************************************************
#pragma once

#include "../Common/Types.h"

#include <DirectXMath.h>
#include <algorithm>
#include <vector>




namespace ECS
{



// *********************************************************************************
//        STRUCTURES TO REPRESENT TYPES OF LIGHTS (DIRECTIONAL, POINT, SPOT)
// *********************************************************************************

enum LightTypes
{
	DIRECTIONAL,
	POINT,
	SPOT,
};

// Common description for members:
// 1. ambient:     the amount of ambient light emitted by the light source
// 2. diffuse:     the amount of diffuse light emitted by the light source
// 3. specular:    the amount of specular light emitted by the light source
// 4. direction:   the direction of the light
// 5. position:    the position of the light
// 6. range:       the range of the light. A point whose distance from the light source is 
//                 greater than the range is not lit.
// 7. attenuation: stores the three attenuation constant in the format (a0, a1, a2) that
//                 control how light intensity falls off with distance
// 8. spot:        the exponent used in the spotlight calculation to control the spotlight cone

enum LightProps
{
	AMBIENT,
	DIFFUSE,
	SPECULAR,
	POSITION,
	DIRECTION,
	RANGE,
	ATTENUATION,
	SPOT_EXP
};

///////////////////////////////////////////////////////////

struct DirLight
{
	DirLight() {}

	DirLight(const DirLight& rhs)
	{
		*this = rhs;
	}

	DirLight& operator=(const DirLight& rhs)
	{
		if (this == &rhs)
			return *this;

		ambient_   = rhs.ambient_;
		diffuse_   = rhs.diffuse_;
		specular_  = rhs.specular_;
		direction_ = rhs.direction_;

		return *this;
	}

	DirLight(
		const DirectX::XMFLOAT4& ambient,
		const DirectX::XMFLOAT4& diffuse,
		const DirectX::XMFLOAT4& specular,
		const DirectX::XMFLOAT3& direction)
		:
		ambient_(ambient),
		diffuse_(diffuse),
		specular_(specular),
		direction_(direction) {}
	

	DirectX::XMFLOAT4 ambient_;
	DirectX::XMFLOAT4 diffuse_;
	DirectX::XMFLOAT4 specular_;
	DirectX::XMFLOAT3 direction_;
	float pad = 0;                    // pad the last float so we can array of light if we wanted
};

///////////////////////////////////////////////////////////

struct PointLight
{
	PointLight() {}

	PointLight(const PointLight& rhs)
	{
		*this = rhs;
	}

	PointLight& operator=(const PointLight& rhs)
	{
		if (this == (&rhs))
			return *this;

		ambient_  = rhs.ambient_;
		diffuse_  = rhs.diffuse_;
		specular_ = rhs.specular_;
		position_ = rhs.position_;
		range_    = rhs.range_;
		att_      = rhs.att_;

		return *this;
	}

	PointLight(
		const XMFLOAT4& ambient,
		const XMFLOAT4& diffuse,
		const XMFLOAT4& specular,
		const XMFLOAT3& position,
		const float range,
		const XMFLOAT3& attenuation)
		:
		ambient_(ambient),
		diffuse_(diffuse),
		specular_(specular),
		position_(position),
		range_(range)
	{
		// invert attenuation params so we can multiply by these values 
		// in the HLSL shader instead of dividing by them
		att_.x = (attenuation.x) ? (1.0f / attenuation.x) : 0.0f;
		att_.y = (attenuation.y) ? (1.0f / attenuation.y) : 0.0f;
		att_.z = (attenuation.z) ? (1.0f / attenuation.z) : 0.0f;
	}


	DirectX::XMFLOAT4 ambient_;
	DirectX::XMFLOAT4 diffuse_;
	DirectX::XMFLOAT4 specular_;

	// packed into 4D vector: (position, range)
	DirectX::XMFLOAT3 position_;
	float range_;

	// packed into 4D vector: (1/att(A0,A1,A2), pad)
	DirectX::XMFLOAT3 att_;
	float pad_ = 0;                    // pad the last float so we can array of light if we wanted
};

///////////////////////////////////////////////////////////

struct SpotLight
{
	SpotLight() {}

	SpotLight(
		const XMFLOAT4& ambient,
		const XMFLOAT4& diffuse,
		const XMFLOAT4& specular,
		const XMFLOAT3& position,
		const float range,
		const XMFLOAT3& direction,
		const float spot,
		const XMFLOAT3& attenuation)
		:
		ambient_(ambient),
		diffuse_(diffuse),
		specular_(specular),
		position_(position),
		range_(range),
		direction_(direction),
		spot_(spot)
	{
		// invert attenuation params so we can multiply by these values 
		// in the HLSL shader instead of dividing by them
		att_.x = (attenuation.x) ? (1.0f / attenuation.x) : 0.0f;
		att_.y = (attenuation.y) ? (1.0f / attenuation.y) : 0.0f;
		att_.z = (attenuation.z) ? (1.0f / attenuation.z) : 0.0f;
	}


	DirectX::XMFLOAT4 ambient_;
	DirectX::XMFLOAT4 diffuse_;
	DirectX::XMFLOAT4 specular_;

	// packed into 4D vector: (position, range)
	DirectX::XMFLOAT3 position_;
	float range_;

	// packed into 4D vector: (direction, spot exponent)
	DirectX::XMFLOAT3 direction_;
	float spot_;

	// packed into 4D vector: (1/att(A0,A1,A2), pad)
	DirectX::XMFLOAT3 att_;
	float pad = 0;                    // pad the last float so we can array of light if we wanted
};


// *********************************************************************************
//        STRUCTURES TO REPRESENT CONTAINERS FOR INIT PARAMS FOR LIGHT SOURCES
// *********************************************************************************

struct DirLightsInitParams
{
	std::vector<XMFLOAT4> ambients;         // the amount of ambient light emitted by the light source
	std::vector<XMFLOAT4> diffuses;         // the amount of diffuse light emitted by the light source
	std::vector<XMFLOAT4> speculars;        // the amount of specular light emitted by the light source
	std::vector<XMFLOAT3> directions;       // the direction of the light
};

///////////////////////////////////////////////////////////

struct PointLightsInitParams
{
	std::vector<XMFLOAT4> ambients;         // the amount of ambient light emitted by the light source
	std::vector<XMFLOAT4> diffuses;         // the amount of diffuse light emitted by the light source
	std::vector<XMFLOAT4> speculars;        // the amount of specular light emitted by the light source
	std::vector<XMFLOAT3> positions;        // the position of the light
	std::vector<XMFLOAT3> attenuations;     // params for controlling how light intensity falls off with distance
	std::vector<float> ranges;              // the range of the light
};

///////////////////////////////////////////////////////////

struct SpotLightsInitParams
{
	std::vector<XMFLOAT4> ambients;         // the amount of ambient light emitted by the light source
	std::vector<XMFLOAT4> diffuses;         // the amount of diffuse light emitted by the light source
	std::vector<XMFLOAT4> speculars;        // the amount of specular light emitted by the light source
	std::vector<XMFLOAT3> positions;        // the position of the light
	std::vector<XMFLOAT3> directions;       // the direction of the light
	std::vector<XMFLOAT3> attenuations;     // params for controlling how light intensity falls off with distance
	std::vector<float> ranges;              // the range of the light
	std::vector<float> spotExponents;       // the exponent used in the spotlight calculation to control the spotlight cone
};


// *********************************************************************************
//           STRUCTURES TO REPRESENT CONTAINERS FOR LIGHT SOURCES
// *********************************************************************************

struct DirLights
{
	size GetCount() const { return std::ssize(data_); }

	std::vector<EntityID> ids_;
	std::vector<DirLight> data_;
};

struct PointLights
{
	size GetCount() const { return std::ssize(data_); }

	std::vector<EntityID> ids_;
	std::vector<PointLight> data_;
};

struct SpotLights
{
	size GetCount() const { return std::ssize(data_); }

	std::vector<EntityID> ids_;
	std::vector<SpotLight> data_;
};


// *********************************************************************************
//                              COMPONENT
// *********************************************************************************

struct Light
{
	ComponentType type_ = ComponentType::LightComponent;

	std::vector<EntityID> ids_;
	DirLights             dirLights_;
	PointLights           pointLights_;
	SpotLights            spotLights_;
};


} // namespace ECS