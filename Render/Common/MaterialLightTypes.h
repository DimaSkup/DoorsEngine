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
		const DirectX::XMFLOAT4& ambient,
		const DirectX::XMFLOAT4& diffuse,
		const DirectX::XMFLOAT4& specular,
		const DirectX::XMFLOAT3& position,
		const float range,
		const DirectX::XMFLOAT3& direction,
		const float spot,
		const DirectX::XMFLOAT3& attenuation)
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


} // namespace Render