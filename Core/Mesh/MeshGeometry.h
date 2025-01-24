// ********************************************************************************
// Filename:      MeshGeometry.h
// Description:   a low-level class that encapsulates the vertex and index buffers,
//                as well as subsets (meshes) data
// 
// Created:       24.10.24
// ********************************************************************************
#pragma once

#include "../Common/Types.h"

#include "Vertex.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

#include "MeshMaterial.h"

#include <d3d11.h>
#include <DirectXCollision.h>

class MeshGeometry
{
public:
	struct Subset
	{
		Subset() {}

		MeshName name_ = "";      // each subset must have its own name
		bool alphaClip_ = false;  // apply alpha clipping to this subset or not
		int id_ = -1;
		int vertexStart_ = 0;     // start pos of vertex in the common buffer
		int vertexCount_ = 0;     // how many vertices this subset has
		int indexStart_ = 0;      // start pos of index in the common buffer
		int indexCount_ = 0;      // how many indices this subset has
	}; 


public:
	MeshGeometry();
	~MeshGeometry();

	MeshGeometry(const MeshGeometry& rhs) = delete;
	MeshGeometry& operator=(const MeshGeometry& rhs) = delete;

	// move constructor/assignment
	MeshGeometry(MeshGeometry&& rhs) noexcept;
	MeshGeometry& operator=(MeshGeometry&& rhs) noexcept;


	// deep copy
	void Copy(
		ID3D11Device* pDevice,
		const Vertex3D* vertices,
		const UINT* indices,
		const int numVertices,
		const int numIndices,
		const MeshGeometry& mesh);

	void AllocateSubsets(const int numSubsets);
	void SetSubsets(const Subset* subsets, const int numSubsets);

	void InitVB(ID3D11Device* pDevice, const Vertex3D* pVertices, int count);
	void InitIB(ID3D11Device* pDevice, const UINT* pIndices, int count);

public:
	VertexBuffer<Vertex3D> vb_;
	IndexBuffer<UINT>      ib_;
	UINT                   vertexStride_ = 0;

	MeshGeometry::Subset*  subsets_ = nullptr;       // data about each mesh of model
	int numSubsets_ = 0;
};
