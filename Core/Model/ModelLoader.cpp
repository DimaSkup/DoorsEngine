// =================================================================================
// Filename:      ModelLoader.cpp
// 
// Created:       11.11.24
// =================================================================================
#include "ModelLoader.h"

#include <CoreCommon/FileSystemPaths.h>
#include <CoreCommon/log.h>
#include "../Texture/TextureMgr.h"

#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;


namespace Core
{

// =================================================================================
// Public API
// =================================================================================

bool ModelLoader::Load(const char* modelFilePath, BasicModel& outModel)
{
	// read in all data from the .de3d file and fill in the input model with this data;
    //
    // input:  modelFilePath -- path to model relatively to the "assets" folder
    // output: outModel      -- a model object filled with loaded data

    char modelPath[256]{ '\0' };
    sprintf(modelPath, "%s%s", g_RelPathAssetsDir, modelFilePath);

	
	std::ifstream fin(modelPath, std::ios::in | std::ios::binary);
    if (!fin)
    {
        sprintf(g_String, "can't open .de3d file for loading: %s", modelPath);
        LogErr(g_String);
        return false;
    }

	// ---------------------------------------------

	M3dMaterial* matParams = nullptr;
	
	try
	{
		ReadHeader(fin, outModel);

		// allocate memory for the model and some temp data
        outModel.AllocateMemory(outModel.numVertices_, outModel.numIndices_, outModel.numSubsets_);

		matParams = new M3dMaterial[outModel.numSubsets_];

		//ReadMaterials(fin, model.numSubsets_, model.materials_, matParams);
		ReadSubsetTable(fin, outModel.numSubsets_, outModel.GetSubsets());
		ReadModelSubsetsAABB(fin, outModel.numSubsets_, outModel.modelAABB_, outModel.subsetsAABB_);

		ReadVertices(fin, outModel.numVertices_, outModel.vertices_);
		ReadIndices(fin, outModel.numIndices_, outModel.indices_);

		// bind textures to the model, etc.
        const std::string modelDir = fs::path(modelPath).parent_path().string();
		SetupSubsets(outModel, matParams, modelDir.c_str());

		SafeDeleteArr(matParams);

        return true;
	}
	catch (std::bad_alloc& e)
	{
        fin.close();
		LogErr(e.what());
		LogErr("can't allocate memory for model's data");
		outModel.Shutdown();
		SafeDeleteArr(matParams);
        return false;
	}
}


// =================================================================================
// Private API
// =================================================================================
void ModelLoader::ReadHeader(std::ifstream& fin, BasicModel& model)
{
	std::string ignore;
	int modelType = 0;

	fin >> ignore;                          // file header text
	fin >> ignore >> model.id_;
	fin >> ignore >> model.name_;
	fin >> ignore >> modelType;

	fin >> ignore >> model.numSubsets_;
	fin >> ignore >> model.numVertices_;
	fin >> ignore >> model.numIndices_;
	fin >> ignore >> model.numBones_;
	fin >> ignore >> model.numAnimClips_;

	model.type_ = eModelType(modelType);
}

///////////////////////////////////////////////////////////

void ModelLoader::ReadMaterials(
	std::ifstream& fin,
	int numMaterials,
	Material* materials,
	M3dMaterial* materialsParams)
{
	std::string ignore;

	// materials header text
	fin >> ignore;

	// read in each material params
	for (int i = 0; i < numMaterials; ++i)
	{
		M3dMaterial& matParams = materialsParams[i];
		Material& meshMat = materials[i];

		fin >> ignore
			>> meshMat.ambient.x
			>> meshMat.ambient.y
			>> meshMat.ambient.z;

		fin >> ignore
			>> meshMat.diffuse.x
			>> meshMat.diffuse.y
			>> meshMat.diffuse.z;

		fin >> ignore
			>> meshMat.specular.x
			>> meshMat.specular.y
			>> meshMat.specular.z;

		// read in a specular power
		fin >> ignore >> meshMat.specular.w;

		fin >> ignore
			>> meshMat.reflect.x
			>> meshMat.reflect.y
			>> meshMat.reflect.z;

		fin >> ignore >> matParams.alphaClip_;
		fin >> ignore >> matParams.effectTypeName_;
		fin >> ignore >> matParams.numTextures_;

		// read each pair 'texture_type' => 'tex_path' for this mesh
		for (int i = 0; i < matParams.numTextures_; ++i)
		{
			fin >> ignore 
				>> matParams.texTypes[i]   // read in a type of texture
				>> ignore 
				>> matParams.texPaths[i];  // read in a path to texture
		}
	}
}

///////////////////////////////////////////////////////////

void ModelLoader::SetupSubsets(
	BasicModel& model,
	const M3dMaterial* matParams,
	const char* modelDirPath)
{
	// load textures for each subset (mesh) and make some other setup

	// load textures for each subset (mesh)
	for (int i = 0; i < model.numSubsets_; ++i)
	{
		const ModelLoader::M3dMaterial& params = matParams[i];

		for (int j = 0; j < params.numTextures_; ++j)
		{
            // generate full path to texture...
            sprintf(g_String, "%s%s%s", modelDirPath, "/textures/", params.texPaths[j].c_str());

            // ...and load it
			const TexID texID = g_TextureMgr.LoadFromFile(g_String);
		}
	}
}

///////////////////////////////////////////////////////////

void ModelLoader::ReadSubsetTable(
	std::ifstream& fin,
	int numSubsets,
	MeshGeometry::Subset* subsets)
{
	std::string ignore;

	// skip subset header text
	fin >> ignore;

	// read in each subset data
	for (int i = 0; i < numSubsets; ++i)
	{
		fin >> ignore >> subsets[i].id;
		fin >> ignore >> subsets[i].vertexStart;
		fin >> ignore >> subsets[i].vertexCount;
		fin >> ignore >> subsets[i].indexStart;
		fin >> ignore >> subsets[i].indexCount;
	}
}

///////////////////////////////////////////////////////////

void ModelLoader::ReadModelSubsetsAABB(
	std::ifstream& fin,
	const int numSubsets,
	DirectX::BoundingBox& modelAABB,
	DirectX::BoundingBox* subsetsAABBs)
{
	// write data about AABB of the whole model and each model's subset

	std::string ignore;

	// skip header text
	fin >> ignore;

	// read in model's AABB
	fin >> ignore;         
	ReadAABB(fin, modelAABB);

	// read in AABB of each subset
	for (int i = 0; i < numSubsets; ++i)
	{
		fin >> ignore;
		ReadAABB(fin, subsetsAABBs[i]);
	}
}

///////////////////////////////////////////////////////////

void ModelLoader::ReadVertices(
	std::ifstream& fin,
	int numVertices,
	Vertex3D* vertices)
{
	std::string ignore;

	// vertices header text
	fin >> ignore;
	fin.get();

	// read in data of each vertex
	fin.read((char*)vertices, sizeof(Vertex3D) * numVertices);
}

///////////////////////////////////////////////////////////

void ModelLoader::ReadIndices(
	std::ifstream& fin,
	int numTriangles,
	UINT* indices)
{
	int faceIdx = 0;
	std::string ignore;

	// triangles header text
	fin >> ignore;
	fin.get();

	// read in indices data of each triangle face
	fin.read((char*)indices, sizeof(UINT) * numTriangles * 3);
}

///////////////////////////////////////////////////////////

void ModelLoader::ReadAABB(std::ifstream& fin, DirectX::BoundingBox& aabb)
{
	// read in data of bounding box

	fin >> aabb.Center.x  >> aabb.Center.y  >> aabb.Center.z;
	fin >> aabb.Extents.x >> aabb.Extents.y >> aabb.Extents.z;
}

} // namespace Core
