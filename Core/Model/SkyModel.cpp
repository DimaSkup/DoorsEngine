// =================================================================================
// Filename:      SkyModel.cpp
// Description:   a class for the sky model (because it is a specific model)
// 
// Created:       23.12.24
// =================================================================================
#include "SkyModel.h"


namespace Core
{

SkyModel::SkyModel()
{
}

SkyModel::~SkyModel()
{
}

///////////////////////////////////////////////////////////

void SkyModel::InitializeBuffers(
	ID3D11Device* pDevice,
	const Vertex3DPos* vertices,
	const USHORT* indices,
	const int numVertices,
	const int numIndices)
{
	// initialize vertex and index buffers with input data

	Assert::True((vertices != nullptr) && (indices != nullptr), "wrong pointers");
	Assert::True((numVertices > 0) && (numIndices > 0), "wrong number of array elemets");

	vb_.Initialize(pDevice, vertices, numVertices, false);
	ib_.Initialize(pDevice, indices, numIndices);
}

///////////////////////////////////////////////////////////

void SkyModel::SetName(const std::string& newName)
{
	// set new name if it is valid or don't change in another case
	name_ = (!newName.empty()) ? newName : name_;
}

///////////////////////////////////////////////////////////

void SkyModel::SetTexture(const int idx, const TexID texID)
{
	Assert::True((idx > -1) && (idx < maxTexNum_), "wrong data to set texture");
	texIDs_[idx] = texID;
}

} // namespace Core