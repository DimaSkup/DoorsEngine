// *********************************************************************************
// Filename:      HelperTypes.h
// Description:   contains some data types which are helpful for ECS testing
// *********************************************************************************
#pragma once
#include <vector>
#include <DirectXMath.h>

using u32 = uint32_t;

struct TransformData
{
	std::vector<DirectX::XMFLOAT3> positions;
	std::vector<DirectX::XMVECTOR> dirQuats;      // direction quaternions
	std::vector<float> uniformScales;
};

struct MoveData
{
	std::vector<DirectX::XMFLOAT3> translations;
	std::vector<DirectX::XMVECTOR> rotQuats;      // rotation quaternion
	std::vector<float> uniformScales;
};

struct RenderedData
{
	std::vector<ECS::RENDERING_SHADERS> shaderTypes;
	std::vector<D3D11_PRIMITIVE_TOPOLOGY> primTopologyTypes;
};