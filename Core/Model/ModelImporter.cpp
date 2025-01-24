////////////////////////////////////////////////////////////////////
// Filename:      ModelImporter.cpp
// 
// Created:       07.07.23
////////////////////////////////////////////////////////////////////
#include "ModelImporter.h"
#include "ModelMath.h"
#include "../Common/log.h"
#include "../Common/Assert.h"

#include "../Model/ModelLoaderM3D.h"

#include "../Texture/TextureMgr.h"
#include "../Model/ModelImporterHelpers.h"
#include "../Common/Utils.h"

#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

using namespace DirectX;

// global default materials for this .cpp
const DirectX::XMFLOAT4 g_DefaultMatAmbient(0.3f, 0.3f, 0.3f, 1);
const DirectX::XMFLOAT4 g_DefaultMatDiffuse(0.8f, 0.8f, 0.8f, 1);
const DirectX::XMFLOAT4 g_DefaultMatSpecular(0, 0, 0, 1);  // w == spec power
const DirectX::XMFLOAT4 g_DefaultMatReflect(0.5f, 0.5f, 0.5f, 1);



// ********************************************************************************
// 
//                              PUBLIC METHODS
// 
// ********************************************************************************

void ModelImporter::LoadFromFile(
	ID3D11Device* pDevice,
	BasicModel& model,
	const std::string& filePath)
{
	// this function initializes a new model from the file 
	// of type .blend, .fbx, .3ds, .obj, .m3d, etc.

	fs::path path = filePath;

	Assert::True(fs::exists(path), "there is no file by path: " + filePath);

	try
	{
		fs::path ext = path.extension();

		// define if we want to load a .m3d file
		if (ext == L".m3d")
		{
			ModelLoaderM3D loader;
			loader.LoadM3d(filePath, model);
			return;
		}

		// --------------------------------------

		// else we want to import a model of some another type
		Assimp::Importer importer;

		const aiScene* pScene = importer.ReadFile(
			filePath, 
			aiProcess_GenNormals |
			aiProcess_CalcTangentSpace |
			//aiProcess_ImproveCacheLocality |
			aiProcess_Triangulate | 
			aiProcess_ConvertToLeftHanded);

		// assert that we successfully read the data file 
		Assert::NotNullptr(pScene, "can't read a model's data file: " + filePath);

		// compute how many vertices/indices/materials/subsets this model has
		ComputeNumOfData(model, pScene->mRootNode, pScene);

		// allocate memory for vertices/indices/etc.
		model.AllocateMemory(model.numVertices_, model.numIndices_, model.numSubsets_);

		int subsetIdx = 0;

		// load all the meshes/materials/textures/etc.
		ProcessNode(
			pDevice, 
			model,
			subsetIdx,
			pScene->mRootNode, 
			pScene, 
			DirectX::XMMatrixIdentity(), 
			filePath);

		importer.FreeScene();
	}
	catch (EngineException & e)
	{
		Log::Error(e, true);
		throw EngineException("can't initialize a model from the file: " + filePath);
	}
}




// ********************************************************************************
// 
//                              PRIVATE METHODS
// 
// ********************************************************************************

void ModelImporter::ComputeNumOfData(
	BasicModel& model,
	const aiNode* pNode, 
	const aiScene* pScene)
{
	// go through all the meshes in the current model's node
	for (UINT i = 0; i < pNode->mNumMeshes; i++)
	{
		aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[i]];

		// accumulate the amount of vertices/indices/subsets in this model
		model.numVertices_ += pMesh->mNumVertices;
		model.numIndices_ += pMesh->mNumFaces * 3;
		model.numSubsets_++;
	}

	// go through all the child nodes of the current node and handle it
	for (UINT i = 0; i < pNode->mNumChildren; i++)
	{
		ComputeNumOfData(model, pNode->mChildren[i], pScene);
	}
}

///////////////////////////////////////////////////////////

void ModelImporter::ProcessNode(
	ID3D11Device* pDevice,
	BasicModel& model,
	int& subsetIdx,
	const aiNode* pNode, 
	const aiScene* pScene,
	const DirectX::XMMATRIX & parentTransformMatrix,  // a matrix which is used to transform position of this mesh to the proper location
	const std::string & filePath)                     // full path to the model
{
	//
	// this function goes through each node of the scene's tree structure
	// starting from the root node and initializes a mesh using data of this each node;
	//
	// created mesh is pushed into the input meshes array
	//


	const XMMATRIX nodeTransformMatrix = XMMATRIX(&pNode->mTransformation.a1) * parentTransformMatrix;

	// go through all the meshes in the current model's node
	for (UINT i = 0; i < pNode->mNumMeshes; i++)
	{
		aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[i]];

		// handle this mesh and push it into the model's meshes array
		ProcessMesh(
			pDevice,
			model,
			subsetIdx,
			pMesh, 
			pScene, 
			nodeTransformMatrix, 
			filePath);

		++subsetIdx;
	}

	// go through all the child nodes of the current node and handle it
	for (UINT i = 0; i < pNode->mNumChildren; i++)
	{
		ProcessNode(
			pDevice, 
			model,
			subsetIdx,
			pNode->mChildren[i], 
			pScene, 
			nodeTransformMatrix, 
			filePath);
	}
}

///////////////////////////////////////////////////////////

void ModelImporter::ProcessMesh(
	ID3D11Device* pDevice,
	BasicModel& model,
	int& subsetIdx,
	const aiMesh* pMesh,                                    // the current mesh of the model
	const aiScene* pScene,                            // a ptr to the scene of this model type
	const DirectX::XMMATRIX & transformMatrix,        // a matrix which is used to transform position of this mesh to the proper location
	const std::string& filePath)                      // full path to the model
{
	// arrays to fill with data

	MeshGeometry::Subset& subset = model.meshes_.subsets_[subsetIdx];
	MeshMaterial& meshMat = model.materials_[subsetIdx];
	
	try
	{	
		SetMeshName(pMesh, subset);

		// load vertices/indices/subsets data
		GetVerticesIndicesOfMesh(pMesh, model, subset, subsetIdx);

		// do some math calculations with these vertices (for instance: computation of tangents/bitangents)
		//ExecuteModelMathCalculations(model.vertices_ + subset.vertexStart_, subset.vertexCount_);

		// get material data of this mesh
		aiMaterial* pAiMat = pScene->mMaterials[pMesh->mMaterialIndex];
		LoadMaterialColors(pAiMat, meshMat);

		// load available textures for this mesh
		TexID* texIDs = model.texIDs_ + subsetIdx * 22;
		LoadMaterialTextures(pDevice, texIDs, pAiMat, subset, pScene, filePath);
		
	}
	catch (std::bad_alloc & e)
	{
		Log::Error(e.what());
		throw EngineException("can't create a mesh of model by path: " + filePath);
	}
	catch (EngineException& e)
	{
		Log::Error(e);
		Log::Error("\n can't import a mesh: " + subset.name_ +
			       ";\n from file: " + filePath);
	}
}

///////////////////////////////////////////////////////////

void ModelImporter::SetMeshName(const aiMesh* pMesh, MeshGeometry::Subset& subset)
{
	// setup a name for the last added mesh
	MeshName name = pMesh->mName.C_Str();

	// check for emptiness (if empty we generate a name for this mesh)
	subset.name_ = (!name.empty()) ? name : "mesh_without_name";
}

///////////////////////////////////////////////////////////

void ModelImporter::LoadMaterialColors(aiMaterial* pMaterial, MeshMaterial& mat)
{
	// read material colors for this mesh

	aiColor4D ambient;
	aiColor4D diffuse;
	aiColor4D specular;
	float shininess;

	aiReturn ret[4];

	ret[0] = pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
	ret[1] = pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
	ret[2] = pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, specular);
	ret[3] = pMaterial->Get(AI_MATKEY_SHININESS, shininess);

	// aiReturn_SUCCESS == 0; so if for instance (ret[0] == 0) we use loaded color 
	// or default one in another case
	mat.ambient_  = (!ret[0]) ? XMFLOAT4(ambient.r, ambient.g, ambient.b, ambient.a) : g_DefaultMatAmbient;
	mat.diffuse_  = (!ret[1]) ? XMFLOAT4(diffuse.r, diffuse.g, diffuse.b, diffuse.a) : g_DefaultMatDiffuse;
	mat.specular_ = ((!ret[2]) & (!ret[3])) ? XMFLOAT4(specular.r, specular.g, specular.b, shininess) : g_DefaultMatSpecular;
}

///////////////////////////////////////////////////////////

void ModelImporter::LoadMaterialTextures(
	ID3D11Device* pDevice,
	TexID* texIDs,
	const aiMaterial* pMaterial,
	const MeshGeometry::Subset& subset,
	const aiScene* pScene,
	const std::string& filePath)
{
	//
	// load all the available textures for this mesh by its material data
	//
	
	TextureMgr* pTexMgr = TextureMgr::Get();
	std::vector<aiTextureType> texTypesToLoad;
	std::vector<UINT> texCounts;

	// define what texture types to load
	for (int i = 1; i < (int)Texture::TEXTURE_TYPE_COUNT; ++i)
	{
		aiTextureType type = (aiTextureType)i;

		// if there are some textures by this type
		if (UINT texCount = pMaterial->GetTextureCount(type))
		{
			texTypesToLoad.push_back(type);
			texCounts.push_back(texCount);
		}
	}
	
	// get path to the directory which contains a model's data file
	fs::path fullPath = filePath;
	fs::path modelDirPath = fullPath.parent_path();

	// go through available texture type and load responsible texture
	for (size idx = 0; idx < std::ssize(texTypesToLoad); ++idx)
	{
		aiTextureType type = texTypesToLoad[idx];
		const UINT texCount = texCounts[idx];
		TexStoreType storeType = TexStoreType::Invalid;

		// go through each texture of this aiTextureType for this aiMaterial
		for (UINT i = 0; i < texCount; i++)
		{
			// get path to the texture file
			aiString path;
			pMaterial->GetTexture(type, i, &path);
			fs::path texPath = path.C_Str();;

			// determine what the texture storage type is
			const TexStoreType storeType = DetermineTextureStorageType(
				pScene, 
				pMaterial,
				i, type);

			type = (type == aiTextureType_HEIGHT) ? aiTextureType_NORMALS : type;

			switch (storeType)
			{

			// load a tex which is located on the disk
			case TexStoreType::Disk:
			{
				// load tex by path and store its ID into the arr
				const std::string texPath = modelDirPath.string() + '/' + path.C_Str();
				texIDs[type] = pTexMgr->LoadFromFile(texPath);

				break;
			}

			// load an embedded compressed texture
			case TexStoreType::EmbeddedCompressed:
			{
				const aiTexture* pAiTexture = pScene->GetEmbeddedTexture(path.C_Str());
				const std::string texName = texPath.stem().string();
		
				// add into the tex mgr a new texture
				const TexID id = pTexMgr->Add(texName, Texture(
					pDevice,
					texName,
					(uint8_t*)(pAiTexture->pcData),         // data of texture
					pAiTexture->mWidth));                  // size of texture);

				texIDs[type] = id;

				break;
			}

			// load an embedded indexed compressed texture
			case TexStoreType::EmbeddedIndexCompressed:
			{
				const UINT index = GetIndexOfEmbeddedCompressedTexture(&path);
				const std::string texName = texPath.stem().string();

				// add into the tex mgr a new texture
				const TexID id = pTexMgr->Add(texName, Texture(
					pDevice,
					texName,
					(uint8_t*)(pScene->mTextures[index]->pcData),   // data of texture
					pScene->mTextures[index]->mWidth));             // size of texture

				texIDs[type] = id;

				break;

			} // end switch/case
			}
		}
	}
}

///////////////////////////////////////////////////////////

void ComputeMeshVertices(
	Vertex3D* vertices,
	const aiMesh* pMesh,
	const int vertexStart,
	const int numVertices)
{
	// get vertices of this mesh
	for (int i = 0, vIdx = vertexStart; i < numVertices; i++, ++vIdx)
	{
		aiVector3D& pos = pMesh->mVertices[i];
		aiVector3D& tex = pMesh->mTextureCoords[0][i];
		aiVector3D& norm = pMesh->mNormals[i];
		aiVector3D& tang = pMesh->mTangents[i];
		aiVector3D& bitang = pMesh->mBitangents[i];

		vertices[vIdx] = std::move(Vertex3D(
			XMFLOAT3(pos.x, pos.y, pos.z),
			XMFLOAT2(tex.x, tex.y),
			XMFLOAT3(norm.x, norm.y, norm.z),
			XMFLOAT3(tang.x, tang.y, tang.z),                    // tangent
			XMFLOAT3(bitang.x, bitang.y, bitang.z),                    // binormal
			PackedVector::XMCOLOR(1, 1, 1, 1)));    // ARGB color
	}
}

///////////////////////////////////////////////////////////

void ComputeMeshIndices(
	UINT* indices,
	aiFace* faces,
	const int indexStart,
	const int numFaces)
{
	// compute value for each index of all the faces in the mesh

	for (int faceIdx = 0, idx = indexStart; faceIdx < numFaces; ++faceIdx)
	{
		indices[idx + 0] = faces[faceIdx].mIndices[0];
		indices[idx + 1] = faces[faceIdx].mIndices[1];
		indices[idx + 2] = faces[faceIdx].mIndices[2];
		idx += 3;
	}
}

///////////////////////////////////////////////////////////

void ComputeMeshAABB(
	DirectX::BoundingBox& aabb, 
	aiVector3D* verticesPos, 
	const int numVertices)
{
	// compute the bounding box of the mesh
	XMVECTOR vMin{ FLT_MAX, FLT_MAX, FLT_MAX };
	XMVECTOR vMax{ FLT_MIN, FLT_MIN, FLT_MIN };

	for (int i = 0; i < numVertices; ++i)
	{
		XMVECTOR P{ verticesPos[i].x, verticesPos[i].y, verticesPos[i].z };
		vMin = XMVectorMin(vMin, P);
		vMax = XMVectorMax(vMax, P);
	}

	// convert min/max representation to center and extents representation
	XMStoreFloat3(&aabb.Center, 0.5f * (vMin + vMax));
	XMStoreFloat3(&aabb.Extents, 0.5f * (vMax - vMin));
}

///////////////////////////////////////////////////////////

void ModelImporter::GetVerticesIndicesOfMesh(
	const aiMesh* pMesh,
	BasicModel& model,
	MeshGeometry::Subset& curSubset,
	const int subsetIdx)
{
	// fill in the arrays with vertices/indices/subset data of the input mesh

	// compute the number of previous vertices/indices so we will know a starting
	// position to continue writing data
	for (int i = 0; i < subsetIdx; ++i)
	{
		const MeshGeometry::Subset& subset = model.meshes_.subsets_[i];
		curSubset.vertexStart_ += subset.vertexCount_;
		curSubset.indexStart_ += subset.indexCount_;
	}

	// define how many vertices/indices this subset (mesh) has
	curSubset.vertexCount_ = pMesh->mNumVertices;
	curSubset.indexCount_ = pMesh->mNumFaces * 3;

	ComputeMeshVertices(
		model.vertices_, 
		pMesh, 
		curSubset.vertexStart_, 
		curSubset.vertexCount_);

	ComputeMeshIndices(
		model.indices_,
		pMesh->mFaces,
		curSubset.indexStart_, 
		(int)pMesh->mNumFaces);

	ComputeMeshAABB(
		model.subsetsAABB_[subsetIdx],
		pMesh->mVertices, 
		curSubset.vertexCount_);
}