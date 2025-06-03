////////////////////////////////////////////////////////////////////
// Filename:     ModelMath.h
// Description:  constains functions for calculation model math
//               (normal, tangent, binormal, etc.)
//
// Created:      05.02.23
////////////////////////////////////////////////////////////////////
#pragma once

#include "../Mesh/Vertex.h"

namespace Core
{

class ModelMath
{
public:
	// function for calculating the tangent and binormal vectors for the model
	void CalculateModelVectors(
		Vertex3D*& vertices,
		int count,
		const bool calculateNormals = true);

	void CalculateTangentBinormal(
		const Vertex3D& vertex1,
		const Vertex3D& vertex2,
		const Vertex3D& vertex3,
		DirectX::XMVECTOR& tangent,
		DirectX::XMVECTOR& binormal);

	void CalculateNormal(
		const DirectX::XMVECTOR& tangent,
		const DirectX::XMVECTOR& binormal,
		DirectX::XMVECTOR& normal);
};

} // namespace Core

