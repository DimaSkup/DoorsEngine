// *********************************************************************************
// Filename:      BasicModel.h
// Description:   a middle-level model class; this contains a MeshGeometry instance
//                specifying the geometry of the model as well as the materials
//                and textures needed to draw the mesh.
//                Additionally, it keeps a system memory 
//                copy of the mesh. System memory copies are needed for when we
//                need to read the mesh data such as for computing bounding volumes,
//                picking, or collision detection
// 
// Created:       30.10.24
// *********************************************************************************
#pragma once


#include "../Model/Types.h"
#include "../Mesh/Vertex.h"
#include "../Mesh/MeshGeometry.h"
#include "../Common/MemHelpers.h"

#include "../Texture/TextureHelperTypes.h"

#include <string>
#include <d3d11.h>

class BasicModel
{
	using SRV = ID3D11ShaderResourceView;

public:
	BasicModel();
	~BasicModel();

	// move constructor/assignment
	BasicModel(BasicModel&& rhs) noexcept;
	BasicModel& operator=(BasicModel&& rhs) noexcept;

	// restrict shallow copying
	BasicModel(const BasicModel&) = delete;
	BasicModel& operator=(const BasicModel&) = delete;

	// deep copy
	void Copy(ID3D11Device* pDevice, const BasicModel& rhs);

	void InitializeBuffers(ID3D11Device* pDevice);
	void Clear();
	void ClearMemory();

	void CopyVertices(const Vertex3D* vertices, const int numVertices);
	void CopyIndices(const UINT* indices, const int numIndices);

	// memory allocation
	void AllocateMemory();
	void AllocateMemory(const int numVertices, const int numIndices, const int numSubsets);

	void AllocateVertices(const int size);
	void AllocateIndices(const int size);

	// query API
	inline ModelID GetID() const { return id_; }
	inline const ModelName& GetName() const { return name_; }
	inline const DirectX::BoundingBox& GetModelAABB()   const { return modelAABB_; }
	inline const DirectX::BoundingBox* GetSubsetsAABB() const { return subsetsAABB_; }
	inline MeshGeometry::Subset* GetSubsets() { return meshes_.subsets_; }

	inline int GetNumVertices() const { return numVertices_; }
	inline int GetNumIndices()  const { return numIndices_; }
	inline int GetNumSubsets()  const { return numSubsets_; }

	// update API
	void SetTexture(const int subsetID, const TexType type, const TexID texID);
	void SetTextures(
		const int subsetID,
		const TexType* types,
		const TexID* ids,
		const int count);

	void SetMaterialForSubset(const int subsetID, const MeshMaterial& material);
	void SetMaterialsForSubsets(
		const int* subsetIds,
		const MeshMaterial* materials,
		const int count);

	void SetModelAABB(const DirectX::BoundingBox& aabb);
	void SetSubsetAABB(const int subsetID, const DirectX::BoundingBox& aabb);
	void SetSubsetAABBs(
		const int* subsetsIDs,
		const DirectX::BoundingBox* AABBs,
		const int count);

	void ComputeModelAABB();
	void ComputeSubsetsAABB();

public:
	ModelID               id_ = 0;
	ModelName             name_ = "inv";
	ModelType             type_ = ModelType::Invalid;
	MeshGeometry          meshes_;                     // contains all the meshes data
	DirectX::BoundingBox  modelAABB_;                  // AABB of the whole model

	TexID*                texIDs_ = nullptr;           // each subset (mesh) has 22 textures
	MeshMaterial*         materials_ = nullptr;
	DirectX::BoundingBox* subsetsAABB_ = nullptr;      // AABB of each subset (mesh)

	// keep CPU copies of the meshes data to read from
	Vertex3D*             vertices_ = nullptr;
	UINT*                 indices_ = nullptr;

	int numVertices_ = 0;
	int numIndices_ = 0;
	int numMats_ = 0;
	int numSubsets_ = 0;
	int numTextures_ = 0;                              // numSubsets * 22
	int numBones_ = 0;
	int numAnimClips_ = 0;                             // animation clips              
};