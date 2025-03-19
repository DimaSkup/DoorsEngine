// *********************************************************************************
// Filename:      BasicModel.cpp
// 
// Created:       30.10.24
// *********************************************************************************
#include "BasicModel.h"
#include <CoreCommon/MemHelpers.h>

namespace Core
{

BasicModel::BasicModel()
{
}

BasicModel::~BasicModel()
{
	Clear();
}

///////////////////////////////////////////////////////////

BasicModel::BasicModel(BasicModel&& rhs) noexcept :
	id_(rhs.id_),
	name_(std::exchange(rhs.name_, "")),
	type_(std::exchange(rhs.type_, ModelType::Invalid)),
	meshes_(std::move(rhs.meshes_)),
	modelAABB_(std::exchange(rhs.modelAABB_, {})),
	
	texIDs_(std::exchange(rhs.texIDs_, nullptr)),
	materials_(std::exchange(rhs.materials_, nullptr)),
	subsetsAABB_(std::exchange(rhs.subsetsAABB_, nullptr)),
	vertices_(std::exchange(rhs.vertices_, nullptr)),
	indices_(std::exchange(rhs.indices_, nullptr)),

	numVertices_(rhs.numVertices_),
	numIndices_(rhs.numIndices_),
	//numMats_(rhs.numMats_),
	numSubsets_(rhs.numSubsets_),
	numTextures_(rhs.numTextures_),
	numBones_(rhs.numBones_),
	numAnimClips_(rhs.numAnimClips_)
{
	// move constructor
}

///////////////////////////////////////////////////////////

BasicModel& BasicModel::operator=(BasicModel&& rhs) noexcept
{
	// move assignment
	if (this != &rhs)
	{
		this->~BasicModel();                    // lifetime of *this ends
		std::construct_at(this, std::move(rhs));
	}

	return *this;
}

///////////////////////////////////////////////////////////

void BasicModel::Copy(ID3D11Device* pDevice, const BasicModel& rhs)
{
	// deep copy of data

	assert(0 && "TODO: FIX IT");
#if 0
	Clear();
	AllocateMemory(rhs.numVertices_, rhs.numIndices_, rhs.numSubsets_);

	type_ = rhs.type_;
	name_ = "copy_of_" + rhs.name_;

	modelAABB_ = rhs.modelAABB_;

	// copy textures / materials / AABB of each subset
	std::copy(rhs.texIDs_, rhs.texIDs_ + numTextures_, texIDs_);
	std::copy(rhs.materials_, rhs.materials_ + numMats_, materials_);
	std::copy(rhs.subsetsAABB_, rhs.subsetsAABB_ + numSubsets_, subsetsAABB_);

	CopyVertices(rhs.vertices_, rhs.numVertices_);
	CopyIndices(rhs.indices_, rhs.numIndices_);

	// copy VB/IB, and subsets data
	meshes_.Copy(pDevice, vertices_, indices_, numVertices_, numIndices_, rhs.meshes_);

	numVertices_ = rhs.numVertices_;
	numIndices_  = rhs.numIndices_;
	numMats_     = rhs.numMats_;
	numSubsets_  = rhs.numSubsets_;
	numTextures_ = rhs.numTextures_;
#endif
}

///////////////////////////////////////////////////////////

void BasicModel::InitializeBuffers(ID3D11Device* pDevice)
{
	meshes_.InitVB(pDevice, vertices_, numVertices_);
	meshes_.InitIB(pDevice, indices_, numIndices_);
}

///////////////////////////////////////////////////////////

void BasicModel::Clear()
{
	id_ = 0;
	name_ = "";
	type_ = ModelType::Invalid;

	meshes_.~MeshGeometry();

	SafeDeleteArr(texIDs_);
	SafeDeleteArr(materials_);
	SafeDeleteArr(subsetsAABB_);
	SafeDeleteArr(vertices_);
	SafeDeleteArr(indices_);

	numVertices_  = 0;
	numIndices_   = 0;
	//numMats_      = 0;
	numSubsets_   = 0;
	numTextures_  = 0;
	numBones_     = 0;
	numAnimClips_ = 0;
}

///////////////////////////////////////////////////////////

void BasicModel::ClearMemory()
{
	meshes_.~MeshGeometry();

	SafeDeleteArr(texIDs_);
	SafeDeleteArr(materials_);
	SafeDeleteArr(subsetsAABB_);
	SafeDeleteArr(vertices_);
	SafeDeleteArr(indices_);
}



// *****************************************************************************
// 
//                       PUBLIC MEMORY ALLOCATION API
// 
// *****************************************************************************

void BasicModel::AllocateMemory()
{
	try
	{
		assert(numVertices_ > 0);
		assert(numIndices_ > 0);
		assert(numSubsets_ > 0);

		// each subset (mesh) has only one material
		//numMats_ = numSubsets_;

		//
		// prepare memory
		//
		ClearMemory();
		meshes_.AllocateSubsets(numSubsets_);

		vertices_    = new Vertex3D[numVertices_]{};
		indices_     = new UINT[numIndices_]{ 0 };
		subsetsAABB_ = new DirectX::BoundingBox[numSubsets_];

		// each subset (mesh) has its own material 
		// so the num of subsets == num of materials
		materials_ = new MeshMaterial[numSubsets_];

		// each subset (mesh) has 22 kinds of textures (diffuse, normal, etc)
		numTextures_ = numSubsets_ * 22;
		texIDs_ = new TexID[numTextures_]{ 0 };

		// if we want to create just only one subset (mesh) 
		// we setup this subset's data right here
		if (numSubsets_ == 1)
		{
			MeshGeometry::Subset& subset = meshes_.subsets_[0];

			subset.vertexCount_ = numVertices_;
			subset.indexCount_ = numIndices_;
		}
	}
	catch (const std::bad_alloc& e)
	{
		Clear();

		Log::Error(e.what());
		Log::Error("can't allocate memory for some data of the model");
		return;
	}
}

///////////////////////////////////////////////////////////

void BasicModel::AllocateMemory(
	const int numVertices,
	const int numIndices,
	const int numSubsets)
{
	try
	{
		assert(numVertices > 0);
		assert(numIndices > 0);
		assert(numSubsets > 0);


		//
		// prepare memory
		//
		ClearMemory();
		meshes_.AllocateSubsets(numSubsets);

		numVertices_ = numVertices;
		numIndices_ = numIndices;
		//numMats_ = numSubsets;
		numSubsets_ = numSubsets;

		vertices_    = new Vertex3D[numVertices_]{};
		indices_     = new UINT[numIndices_]{ 0 };
		subsetsAABB_ = new DirectX::BoundingBox[numSubsets];

		// each subset (mesh) has its own material 
		// so the num of subsets == num of materials
		materials_ = new MeshMaterial[numSubsets_];

		// each subset (mesh) has 22 kinds of textures (diffuse, normal, etc)
		numTextures_ = numSubsets_ * 22;
		texIDs_ = new TexID[numTextures_]{ 0 };

		// if we want to create just only one subset (mesh) 
		// we setup this subset's data right here
		if (numSubsets == 1)
		{
			MeshGeometry::Subset& subset = meshes_.subsets_[0];

			subset.vertexCount_ = numVertices;
			subset.indexCount_ = numIndices;
		}
	}
	catch (const std::bad_alloc& e)
	{
		Clear();

		Log::Error(e.what());
		Log::Error("can't allocate memory for some data of the model");
		return;
	}
}

///////////////////////////////////////////////////////////

void BasicModel::AllocateVertices(const int size)
{
	assert(size > 0);

	SafeDeleteArr(vertices_);
	vertices_ = new Vertex3D[size]{};
	numVertices_ = size;
}

///////////////////////////////////////////////////////////

void BasicModel::AllocateIndices(const int size)
{
	assert(size > 0);

	SafeDeleteArr(indices_);
	indices_ = new UINT[size]{ 0 };
	numIndices_ = size;
}

///////////////////////////////////////////////////////////

void BasicModel::CopyVertices(const Vertex3D* vertices, const int numVertices)
{
	// check input data and check if we have enough allocated memory
	assert((vertices != nullptr) && (numVertices > 0));
	assert((vertices_ != nullptr) && (numVertices_ >= (u32)numVertices));

	std::copy(vertices, vertices + numVertices, vertices_);
}

///////////////////////////////////////////////////////////

void BasicModel::CopyIndices(const UINT* indices, const int numIndices)
{
	// check input data and check if we have enough allocated memory
	assert((indices != nullptr) && (numIndices > 0));
	assert((indices_ != nullptr) && (numIndices_ >= (u32)numIndices));

	std::copy(indices, indices + numIndices, indices_);
}


// *****************************************************************************
// 
//                           PUBLIC UPDATING API
// 
// *****************************************************************************

void BasicModel::SetTexture(const int subsetID, const TexType type, const TexID id)
{
	// set a texture for particular subset (mesh) of the model
	Assert::True(subsetID > -1, "wrong ID of subset");
	SetTextures(subsetID, &type, &id, 1);
}

///////////////////////////////////////////////////////////

void BasicModel::SetTextures(
	const int subsetID,
	const TexType* types,
	const TexID* ids,
	const int count)
{
	// setup multiple textures for particular subset (mesh) of the model

	Assert::True(subsetID > -1, "wrong ID of subset");
	Assert::True(count > 0, "wrong num of input elems");
	Assert::NotNullptr(texIDs_, "you didn't allocate memory for data");

	int offset = subsetID * 22;  // 22 textures types per subset (mesh)

	// setup textures by types
	for (int i = 0; i < count; ++i)
		texIDs_[offset + types[i]] = ids[i];
}

///////////////////////////////////////////////////////////

void BasicModel::SetMaterialForSubset(
	const int subsetID,
	const MeshMaterial& material)
{
	Assert::True(subsetID > -1, "wrong ID of subset");
	SetMaterialsForSubsets(&subsetID, &material, 1);
}

///////////////////////////////////////////////////////////

void BasicModel::SetMaterialsForSubsets(
	const int* subsetIds,
	const MeshMaterial* materials,
	const int count)
{
	Assert::NotNullptr(subsetIds, "arr of subset ids == nullptr");
	Assert::NotNullptr(materials, "arr of materials == nullptr");
	Assert::NotNullptr(materials_, "you didn't allocate memory for data");
	Assert::True(count > 0, "wrong num of input data elements");
	//Assert::True(count <= numMats_, "you haven't enought memory for input data");

	// subset (mesh) ID is the same as its idx in the BasicModel class;
	// so we can access material by the same idx as well
	for (int i = 0; i < count; ++i)
	{
		const int matIdx = subsetIds[i];
		materials_[matIdx] = materials[i];
	}
}

///////////////////////////////////////////////////////////

void BasicModel::SetModelAABB(const DirectX::BoundingBox& aabb)
{
	// set AABB for the whole model
	modelAABB_ = aabb;
}

///////////////////////////////////////////////////////////

void BasicModel::SetSubsetAABB(const int subsetID, const DirectX::BoundingBox& aabb)
{
	Assert::True(subsetID > -1, "wrong ID of subset");
	SetSubsetAABBs(&subsetID, &aabb, 1);
}

///////////////////////////////////////////////////////////

void BasicModel::SetSubsetAABBs(
	const int* subsetsIDs,
	const DirectX::BoundingBox* AABBs,
	const int count)
{
	Assert::NotNullptr(subsetsIDs, "ptr to subsets IDs == nullptr");
	Assert::NotNullptr(AABBs, "ptr to AABBs == nullptr");
	Assert::NotNullptr(materials_, "you didn't allocate memory for data");
	Assert::True(count > 0, "wrong num of input elems");
	//Assert::True(count <= numMats_, "you haven't enought memory for input data");

	// subset (mesh) ID is the same as its idx in the BasicModel class;
	// so we can access AABB by the same idx as well
	for (int i = 0; i < count; ++i)
	{
		const int idx = subsetsIDs[i];
		subsetsAABB_[idx] = AABBs[i];
	}
}

///////////////////////////////////////////////////////////

void BasicModel::ComputeModelAABB()
{
	// compute a bounding box of the whole model by AABB of each subset (mesh)
	// NOTE: subsets AABBs must be already computed before

	using namespace DirectX;

	XMVECTOR vMin{ FLT_MAX, FLT_MAX, FLT_MAX };
	XMVECTOR vMax{ FLT_MIN, FLT_MIN, FLT_MIN };

	// go through each subset (mesh)
	for (int i = 0; i < numSubsets_; ++i)
	{
		const DirectX::BoundingBox& subsetAABB = subsetsAABB_[i];

		// define min/max point of this mesh
		const XMVECTOR meshVecCenter  = XMLoadFloat3(&subsetAABB.Center);
		const XMVECTOR meshVecExtents = XMLoadFloat3(&subsetAABB.Extents);
		const XMVECTOR meshVecMax = meshVecCenter + meshVecExtents;
		const XMVECTOR meshVecMin = meshVecCenter - meshVecExtents;

		vMin = XMVectorMin(vMin, meshVecMin);
		vMax = XMVectorMax(vMax, meshVecMax);
	}

	// compute a model's AABB
	XMStoreFloat3(&modelAABB_.Center,  0.5f * (vMin + vMax));
	XMStoreFloat3(&modelAABB_.Extents, 0.5f * (vMax - vMin));
}

///////////////////////////////////////////////////////////

void BasicModel::ComputeSubsetsAABB()
{
	// compute a bounding box of each subset (mesh) of the model by its vertices
	// NOTE: there must be already data of vertices and subsets

	using namespace DirectX;

	
	// go through each subset (mesh)
	for (int i = 0; i < numSubsets_; ++i)
	{
		MeshGeometry::Subset& subset = meshes_.subsets_[i];
		Vertex3D* vertices = vertices_ + subset.vertexStart_;

		XMVECTOR vMin{ FLT_MAX, FLT_MAX, FLT_MAX };
		XMVECTOR vMax{ FLT_MIN, FLT_MIN, FLT_MIN };
		
		// go through each vertex of this subset (mesh)
		for (int vIdx = 0; vIdx < (int)subset.vertexCount_; ++vIdx)
		{
			XMVECTOR P = XMLoadFloat3(&vertices[vIdx].position);
			vMin = DirectX::XMVectorMin(vMin, P);
			vMax = DirectX::XMVectorMax(vMax, P);
		}

		// convert min/max representation to center and extents representation
		DirectX::XMStoreFloat3(&subsetsAABB_[i].Center,  0.5f * (vMin + vMax));
		DirectX::XMStoreFloat3(&subsetsAABB_[i].Extents, 0.5f * (vMax - vMin));
	}
}

} // namespace Core
