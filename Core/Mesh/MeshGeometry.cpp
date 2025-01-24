// *********************************************************************************
// Filename:    MeshGeometry.cpp
// 
// Created:     29.10.24
// *********************************************************************************
#include "MeshGeometry.h"
#include "../Common/MemHelpers.h"
#include <cassert>


MeshGeometry::MeshGeometry() 
{
}

MeshGeometry::~MeshGeometry() 
{
	// release memory
	SafeDeleteArr(subsets_);
	numSubsets_ = 0;

	vb_.~VertexBuffer();
	ib_.~IndexBuffer();
}

MeshGeometry::MeshGeometry(MeshGeometry&& rhs) noexcept :
	vb_(std::move(rhs.vb_)),
	ib_(std::move(rhs.ib_)),
	vertexStride_(std::exchange(rhs.vertexStride_, 0)),
	subsets_(std::exchange(rhs.subsets_, nullptr)),
	numSubsets_(std::exchange(rhs.numSubsets_, 0))
{
	// move constructor
}

MeshGeometry& MeshGeometry::operator=(MeshGeometry&& rhs) noexcept
{
	// move assignment
	if (this != &rhs)
	{
		this->~MeshGeometry();
		std::construct_at(this, std::move(rhs));
	}

	return *this;
}


// *********************************************************************************
//
//                              PUBLIC METHODS
// 
// *********************************************************************************

void MeshGeometry::Copy(
	ID3D11Device* pDevice,
	const Vertex3D* vertices,
	const UINT* indices,
	const int numVertices,
	const int numIndices,
	const MeshGeometry& mesh)
{
	// deep copy of data from the input mesh geometry obj

	InitVB(pDevice, vertices, numVertices);
	InitIB(pDevice, indices, numIndices);
	SetSubsets(mesh.subsets_, mesh.numSubsets_);
}

///////////////////////////////////////////////////////////

void MeshGeometry::AllocateSubsets(const int numSubsets)
{
	// allocate memory for this number of subsets;

	try
	{
		Assert::True(numSubsets > 0, "num of subsets must be > 0");

		// we already have enough memory; just go out
		if (this->numSubsets_ == numSubsets)
			return;

		this->~MeshGeometry();
		subsets_ = new Subset[numSubsets];
		numSubsets_ = numSubsets;

		// setup ID for each subset
		for (int i = 0; i < numSubsets_; ++i)
			subsets_[i].id_ = i;
	}
	catch (const std::bad_alloc& e)
	{
		const std::string errMsg = "can't allocate memory for subsets";
		Log::Error(e.what());
		Log::Error(errMsg);
		throw EngineException(errMsg);
	}
	catch (EngineException& e)
	{
		const std::string errMsg = "can't allocate memory for subsets";
		Log::Error(e);
		Log::Error(errMsg);
		throw EngineException(errMsg);
	}
}

///////////////////////////////////////////////////////////

void MeshGeometry::SetSubsets(const Subset* subsets, const int numSubsets)
{
	// set new subsets (meshes) data
	assert((subsets != nullptr) && (numSubsets > 0));

	AllocateSubsets(numSubsets);
	std::copy(subsets, subsets + numSubsets, this->subsets_);
}

///////////////////////////////////////////////////////////

void MeshGeometry::InitVB(
	ID3D11Device* pDevice,
	const Vertex3D* pVertices,
	int count)
{
	vb_.Initialize(pDevice, pVertices, count, false);
	vertexStride_ = sizeof(Vertex3D);
}

///////////////////////////////////////////////////////////

void MeshGeometry::InitIB(
	ID3D11Device* pDevice,
	const UINT* pIndices,
	int count)
{
	ib_.Initialize(pDevice, pIndices, count);
}

///////////////////////////////////////////////////////////