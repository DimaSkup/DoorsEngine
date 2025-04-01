// *********************************************************************************
// Filename:      ModelLoader.h
// Description:   loader for models data of the internal format .de3d
// 
// Created:       11.11.24
// *********************************************************************************
#pragma once

#include "BasicModel.h"
#include <fstream>
#include <filesystem>

namespace Core
{

class ModelLoader
{
public:
	struct M3dMaterial
	{
		Material mat_;
		bool alphaClip_;
		int numTextures_ = 0;
		std::string effectTypeName_;
		std::string texTypes[NUM_TEXTURE_TYPES];
		std::string texPaths[NUM_TEXTURE_TYPES];           // each mesh (subset) can have 22 texture types
	};

public:
	void Load(
		const std::string& assetFilepath,   // path to model relatively to the "assets" folder
		BasicModel& model);

private:
	void ReadHeader(std::ifstream& fin, BasicModel& model);

	void ReadMaterials(
		std::ifstream& fin,
		int numMaterials,
		Material* materials,
		M3dMaterial* materialsParams);

	void SetupSubsets(
		BasicModel& model,
		const M3dMaterial* m3dMats,
		const std::string& modelDirPath);

	void ReadSubsetTable(
		std::ifstream& fin,
		int numSubsets,
		MeshGeometry::Subset* subsets);

	void ReadModelSubsetsAABB(
		std::ifstream& fin,
		const int numSubsets,
		DirectX::BoundingBox& modelAABB,
		DirectX::BoundingBox* subsetsAABBs);

	void ReadVertices(
		std::ifstream& fin,
		int numVertices,
		Vertex3D* vertices);

	void ReadIndices(
		std::ifstream& fin,
		int numTriangles,
		UINT* indices);

	void ReadAABB(std::ifstream& fin, DirectX::BoundingBox& aabb);
};

} // namespace Core
