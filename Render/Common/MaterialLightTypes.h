// =================================================================================
// Filename:      MaterialLightTypes.h
// Description:   contains declarations of Material and different light src types
// 
// Created:       30.08.24
// =================================================================================
#pragma once

#include <DirectXMath.h>

namespace Render
{


struct Material
{
	DirectX::XMFLOAT4 ambient_  = { 1,1,1,1 };
	DirectX::XMFLOAT4 diffuse_  = { 1,1,1,1 };
	DirectX::XMFLOAT4 specular_ = { 0,0,0,1 };             // w = specPower (specular power)
	DirectX::XMFLOAT4 reflect_  = { 0.5f, 0.5f, 0.5f, 1 };

	Material() {}

	Material(
		const DirectX::XMFLOAT4& ambient,
		const DirectX::XMFLOAT4& diffuse,
		const DirectX::XMFLOAT4& specular,
		const DirectX::XMFLOAT4& reflect) :
		ambient_(ambient),
		diffuse_(diffuse),
		specular_(specular),
		reflect_(reflect) {}
};

///////////////////////////////////////////////////////////

struct DirLight
{
	DirLight() {}

	DirLight(
		DirectX::XMFLOAT4& ambient,
		DirectX::XMFLOAT4& diffuse,
		DirectX::XMFLOAT4& specular,
		DirectX::XMFLOAT3& direction)
		:
		ambient(ambient),
		diffuse(diffuse),
		specular(specular),
		direction(direction) {}


	DirectX::XMFLOAT4 ambient   = { 0,0,0,1 };
	DirectX::XMFLOAT4 diffuse   = { 0,0,0,1 };
	DirectX::XMFLOAT4 specular  = { 0,0,0,1 };
	DirectX::XMFLOAT3 direction = { 0,0,1 };
	float pad = 0;                             // pad the last float so we can array of light if we wanted
};

///////////////////////////////////////////////////////////

struct PointLight
{
	PointLight() {}

	DirectX::XMFLOAT4 ambient   = { 0,0,0,1 };
	DirectX::XMFLOAT4 diffuse   = { 0,0,0,1 };
	DirectX::XMFLOAT4 specular  = { 0,0,0,1 };

	// packed into 4D vector: (position, range)
	DirectX::XMFLOAT3 position = { 0,0,0 };
	float range = 1.0f;

	// packed into 4D vector: (1/att(A0,A1,A2), pad)
	DirectX::XMFLOAT3 att = { 1,1,1 };
	float pad = 0;                    // pad the last float so we can array of light if we wanted
};

///////////////////////////////////////////////////////////

struct SpotLight
{
	SpotLight() {}

#if 0
	SpotLight(
		const DirectX::XMFLOAT4& ambient,
		const DirectX::XMFLOAT4& diffuse,
		const DirectX::XMFLOAT4& specular,
		const DirectX::XMFLOAT3& position,
		const float range,
		const DirectX::XMFLOAT3& direction,
		const float spot,
		const DirectX::XMFLOAT3& attenuation)
		:
		ambient(ambient),
		diffuse(diffuse),
		specular(specular),
		position(position),
		range(range),
		direction(direction),
		spot(spot)
	{
		// invert attenuation params so we can multiply by these values 
		// in the HLSL shader instead of dividing by them
		att_.x = (attenuation.x) ? (1.0f / attenuation.x) : 0.0f;
		att_.y = (attenuation.y) ? (1.0f / attenuation.y) : 0.0f;
		att_.z = (attenuation.z) ? (1.0f / attenuation.z) : 0.0f;
	}
#endif


	DirectX::XMFLOAT4 ambient   = { 0,0,0,1 };
	DirectX::XMFLOAT4 diffuse   = { 0,0,0,1 };
	DirectX::XMFLOAT4 specular  = { 0,0,0,1 };

	// packed into 4D vector: (position, range)
	DirectX::XMFLOAT3 position  = { 0,0,0 };
	float range = 1.0f;

	// packed into 4D vector: (direction, spot exponent)
	DirectX::XMFLOAT3 direction = { 0,0,1 };
	float spot = 1.0f;

	// packed into 4D vector: (1/att(A0,A1,A2), pad)
	DirectX::XMFLOAT3 att = { 1,1,1 };
	float pad = 0;                    // pad the last float so we can array of light if we wanted
};


} // namespace Render
