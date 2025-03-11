// *********************************************************************************
// Filename:      ModelLoaderM3D.cpp
//
// Created:       24.10.24
// *********************************************************************************
#include "ModelLoaderM3D.h"
#include <CoreCommon/FileSystemPaths.h>
#include <CoreCommon/log.h>
#include "../Texture/TextureMgr.h"


namespace Core
{

void ModelLoaderM3D::LoadM3d(const std::string& filename, BasicModel& model)
{
	// read in all data from the .m3d file and
	// fill in the input model with this data

	Log::Debug("load a model from: " + filename);

	std::ifstream fin(filename, std::ios::in | std::ios::binary);
	if (!fin.is_open())
	{
		Log::Error("can't open .m3d file: " + filename);
		return;
	}

	M3dMaterial* matParams = nullptr;
	int numMaterials = 0;
	int numVertices = 0;
	int numTriangles = 0;
	int numBones = 0;
	int numAnimationClips = 0;
	std::string ignore;

	fin >> ignore;                        // file header text
	fin >> ignore >> numMaterials;        // the same as the numSubsets
	fin >> ignore >> numVertices;
	fin >> ignore >> numTriangles;
	fin >> ignore >> numBones;
	fin >> ignore >> numAnimationClips;

	try 
	{
		// allocate memory for the model and some temp data
		model.AllocateMemory(numVertices, numTriangles * 3, numMaterials);
		matParams = new M3dMaterial[numMaterials];

		ReadMaterials(fin, numMaterials, model.materials_, matParams);
		ReadSubsetTable(fin, numMaterials, model.meshes_.subsets_);
		ReadVertices(fin, numVertices, model.vertices_);
		ReadTriangles(fin, numTriangles, model.indices_);

		// bind textures to the model, etc.
		SetupMaterials(model, numMaterials, matParams);

		// clear transient data
		SafeDeleteArr(matParams);
	}
	catch (std::bad_alloc& e)
	{
		Log::Error(e.what());
		Log::Error("can't allocate memory for model's data");
		model.~BasicModel();
		SafeDeleteArr(matParams);
	}
}

///////////////////////////////////////////////////////////

void ModelLoaderM3D::ReadMaterials(
	std::ifstream& fin,
	int numMaterials,
	MeshMaterial* materials,
	M3dMaterial* materialsParams)
{
	std::string ignore;

	// materials header text
	fin >> ignore;  

	// read in each material params
	for (int i = 0; i < numMaterials; ++i)
	{
		M3dMaterial& matParams = materialsParams[i];
		MeshMaterial& meshMat = materials[i];

		fin >> ignore
			>> meshMat.ambient_.x
			>> meshMat.ambient_.y
			>> meshMat.ambient_.z;

		fin >> ignore
			>> meshMat.diffuse_.x
			>> meshMat.diffuse_.y
			>> meshMat.diffuse_.z;

		fin >> ignore
			>> meshMat.specular_.x
			>> meshMat.specular_.y
			>> meshMat.specular_.z;

		// read in a specular power
		fin >> ignore >> meshMat.specular_.w;

		fin >> ignore
			>> meshMat.reflect_.x
			>> meshMat.reflect_.y
			>> meshMat.reflect_.z;

		fin >> ignore >> matParams.alphaClip_;
		fin >> ignore >> matParams.effectTypeName_;
		fin >> ignore >> matParams.diffuseMapName_;
		fin >> ignore >> matParams.normalMapName_;
	}
}

///////////////////////////////////////////////////////////

void ModelLoaderM3D::SetupMaterials(
	BasicModel& model,
	const int numMaterials,
	const M3dMaterial* matParams)
{
	TextureMgr& texMgr = *TextureMgr::Get();
	const std::string texDirPath = g_RelPathTexDir + "TexturesM3D/";

	// setup alpha clipping for each subset
	for (int i = 0; i < model.numSubsets_; ++i)
	{
		model.meshes_.subsets_[i].alphaClip_ |= matParams[i].alphaClip_;
	}

	// setup textures for each subset (mesh)
	for (int i = 0; i < numMaterials; ++i)
	{
		const ModelLoaderM3D::M3dMaterial& params = matParams[i];

		const TexID texDiff = texMgr.LoadFromFile(texDirPath + params.diffuseMapName_);
		const TexID texNorm = texMgr.LoadFromFile(texDirPath + params.normalMapName_);

		model.SetTexture(i, TexType::DIFFUSE, texDiff);
		model.SetTexture(i, TexType::NORMALS, texNorm);
	}
}

///////////////////////////////////////////////////////////

void ModelLoaderM3D::ReadSubsetTable(
	std::ifstream& fin,
	int numSubsets,
	MeshGeometry::Subset* subsets)
{
	std::string ignore;

	// subset header text
	fin >> ignore;

	// read in each subset data
	for (int i = 0; i < numSubsets; ++i)
	{
		MeshGeometry::Subset& subset = subsets[i];

		fin >> ignore >> subset.id_;
		fin >> ignore >> subset.vertexStart_;
		fin >> ignore >> subset.vertexCount_;
		fin >> ignore >> subset.indexStart_;
		fin >> ignore >> subset.indexCount_;     

		// read in a num of triangle but we need the num of indices
		subset.indexStart_ *= 3;
		subset.indexCount_ *= 3;
	}
}

///////////////////////////////////////////////////////////

void ModelLoaderM3D::ReadVertices(
	std::ifstream& fin,
	int numVertices,
	Vertex3D* vertices)
{
	std::string ignore;

	// vertices header text
	fin >> ignore;

	// read in data of each vertex
	for (int i = 0; i < numVertices; ++i)
	{
		Vertex3D& vertex = vertices[i];

		fin >> ignore
			>> vertex.position.x
			>> vertex.position.y
			>> vertex.position.z;

		fin >> ignore
			>> vertex.tangent.x
			>> vertex.tangent.y
			>> vertex.tangent.z;
		fin >> ignore;           // tan.w == -1

		fin >> ignore
			>> vertex.normal.x
			>> vertex.normal.y
			>> vertex.normal.z;

		fin >> ignore
			>> vertex.texture.x
			>> vertex.texture.y;
	}
}

///////////////////////////////////////////////////////////

void ModelLoaderM3D::ReadTriangles(
	std::ifstream& fin,
	int numTriangles,
	UINT* indices)
{
	int faceIdx = 0;
	std::string ignore;

	// triangles header text
	fin >> ignore;

	// read in indices data of each triangle face
	for (int i = 0; i < numTriangles; ++i)
	{
		fin >> indices[i * 3 + 0]
			>> indices[i * 3 + 1]
			>> indices[i * 3 + 2];
	}
}

} // namespace Core
