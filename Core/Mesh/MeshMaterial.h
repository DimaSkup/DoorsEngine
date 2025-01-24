// ********************************************************************************
// Filename:      Material.h
// Description:   data structure of a mesh Material
// 
// Created:       28.10.24
// ********************************************************************************
#pragma once

#include <DirectXMath.h>


struct MeshMaterial
{
	DirectX::XMFLOAT4 ambient_ = { 1,1,1,1 };
	DirectX::XMFLOAT4 diffuse_ = { 1,1,1,1 };
	DirectX::XMFLOAT4 specular_ = { 0,0,0,1 };   // w = specPower (specular power)
	DirectX::XMFLOAT4 reflect_ = { .5f, .5f, .5f, 1 };


	MeshMaterial() {}

	MeshMaterial(
		const DirectX::XMFLOAT4& ambient,
		const DirectX::XMFLOAT4& diffuse,
		const DirectX::XMFLOAT4& specular,
		const DirectX::XMFLOAT4& reflect) :
		ambient_(ambient),
		diffuse_(diffuse),
		specular_(specular),
		reflect_(reflect) {}

	inline void SetAmbient(const DirectX::XMFLOAT4& newAmbient) { ambient_ = newAmbient; }
	inline void SetDiffuse(const DirectX::XMFLOAT4& newDiffuse) { diffuse_ = newDiffuse; }
	inline void SetSpecular(const DirectX::XMFLOAT4& newSpecular) { specular_ = newSpecular; }
	inline void SetSpecularPower(const float power) { specular_.w = power; }
};

