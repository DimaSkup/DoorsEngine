////////////////////////////////////////////////////////////////////
// Filename:     ModelMath.cpp
// Created:      06.02.23
////////////////////////////////////////////////////////////////////
#include <CoreCommon/pch.h>
#include "ModelMath.h"

using namespace DirectX;

namespace Core
{

void ModelMath::CalculateModelVectors(
	Vertex3D*& vertices,
	int count,
	const bool calculateNormals)
{
	// CalculateModelVectors() generates the tangent and binormal for the model as well as 
	// a recalculated normal vector. To start it calculates how many faces (triangles) are
	// in the model. Then for each of those triangles it gets the three vertices and uses
	// that to calculate the tangent, binormal, and normal. After calculating those three
	// normal vectors it then saves them back into the model structure.

	// Input:
	// 1. An array of vertices (vertices).
	// 2. A boolean flag (isCalcNormals) which defined either we compute normal vectors or not

	assert(vertices != nullptr);
	assert(count > 2);

	DirectX::XMVECTOR tangent;
	DirectX::XMVECTOR binormal;
	

	// calculate the number of faces in the model
	const size_t facesCount = count / 3;

	// go throught all the faces and calculate the tangent, binormal, and normal vectors
	for (size_t faceIdx = 0, dataIdx = 0; faceIdx < facesCount; ++faceIdx)
	{
		
		// calculate the tangent and binormal of that face
		CalculateTangentBinormal(
			vertices[dataIdx++],
			vertices[dataIdx++],
			vertices[dataIdx++],
			tangent,
			binormal);

		// if we want to compute normals as well
		if (calculateNormals)
		{
			DirectX::XMVECTOR normalVec;
	
			CalculateNormal(tangent, binormal, normalVec);
	
			// store these normal vectors into the vertices of the face
			XMStoreFloat3(&vertices[dataIdx-3].normal, normalVec);
			XMStoreFloat3(&vertices[dataIdx-2].normal, normalVec);
			XMStoreFloat3(&vertices[dataIdx-1].normal, normalVec);
		}
	

		// store the tangent, and binormal for this face back in the vertex structure
		for (int backIndex = 3; backIndex > 0; backIndex--)
		{
			const int vIdx = (int)dataIdx - backIndex;  // an index of the vertex
			XMStoreFloat3(&vertices[vIdx].tangent, tangent);
		}
	}
}

///////////////////////////////////////////////////////////

void ModelMath::CalculateTangentBinormal(
	const Vertex3D & vertex1,
	const Vertex3D & vertex2,
	const Vertex3D & vertex3,
	_Out_ DirectX::XMVECTOR & tangent,
	_Out_ DirectX::XMVECTOR & binormal)
{
	// the CalculateTangentBinormal() takes in three vertices and then
	// calculates and returns the tangent and binormal of those three vertices

	float den = 0.0f;    // denominator of the tangent/binormal equation
	float tuVector[2]{ 0.0f };
	float tvVector[2]{ 0.0f };
	
	const DirectX::XMVECTOR vertex1_pos = DirectX::XMLoadFloat3(&vertex1.position);
	const DirectX::XMVECTOR vertex2_pos = DirectX::XMLoadFloat3(&vertex2.position);
	const DirectX::XMVECTOR vertex3_pos = DirectX::XMLoadFloat3(&vertex3.position);

	// calculate the two vectors of edges for this face
	DirectX::XMFLOAT3 edge1;
	DirectX::XMFLOAT3 edge2;
	DirectX::XMStoreFloat3(&edge1, DirectX::XMVectorSubtract(vertex2_pos, vertex1_pos));
	DirectX::XMStoreFloat3(&edge2, DirectX::XMVectorSubtract(vertex3_pos, vertex1_pos));
	
	// calculate the tu and tv texture space vectors
	tuVector[0] = vertex2.texture.x - vertex1.texture.x; 
	tvVector[0] = vertex2.texture.y - vertex1.texture.y;

	tuVector[1] = vertex3.texture.x - vertex1.texture.x;
	tvVector[1] = vertex3.texture.y - vertex1.texture.y;

	// calculate the denominator of the tangent/binormal equation
	den = 1.0f / (tuVector[0] * tvVector[1] - tuVector[1] * tvVector[0]);

#if 0
	if (isinf(den))
	{
		throw std::runtime_error("denominator of the tangent/binormal equation is infinite");
	}
#endif

	// calculate the cross products and multiply by the coefficient to get 
	// the tangent and binormal;
	const float tangent_x = (tvVector[1] * edge1.x - tvVector[0] * edge2.x) * den;
	const float tangent_y = (tvVector[1] * edge1.y - tvVector[0] * edge2.y) * den;
	const float tangent_z = (tvVector[1] * edge1.z - tvVector[0] * edge2.z) * den;

	const float binormal_x = (tuVector[0] * edge2.x - tuVector[1] * edge1.x) * den;
	const float binormal_y = (tuVector[0] * edge2.y - tuVector[1] * edge1.y) * den;
	const float binormal_z = (tuVector[0] * edge2.z - tuVector[1] * edge1.z) * den;

	// normalize the tangent components and then store it
	tangent = DirectX::XMVector3Normalize({ tangent_x, tangent_y, tangent_z });

	// normalize the binormal components and then store it
	binormal = DirectX::XMVector3Normalize({ binormal_x, binormal_y, binormal_z });
}

///////////////////////////////////////////////////////////

void ModelMath::CalculateNormal(
	const DirectX::XMVECTOR& tangent,
	const DirectX::XMVECTOR& binormal,
	DirectX::XMVECTOR& normal)
{
	// the CalculateNormal() takes in the tangent and binormal and does a cross product
	// to give back the normalized normal vector

	normal = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(tangent, binormal));
}

} // namespace Core
