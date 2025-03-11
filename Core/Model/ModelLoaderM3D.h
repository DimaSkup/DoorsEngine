// *********************************************************************************
// Filename:      ModelLoaderM3D.h
// Description:   functional to load models from the .m3d model format
// 
// Created:       24.10.24
// *********************************************************************************
#pragma once

#include "BasicModel.h"
#include <string>
#include <fstream>


namespace Core
{

class ModelLoaderM3D
{
public:
	struct M3dMaterial
	{
		MeshMaterial mat_;
		bool alphaClip_;
		std::string effectTypeName_;
		std::string diffuseMapName_;
		std::string normalMapName_;
	};

public:
	void LoadM3d(const std::string& filename, BasicModel& model);

	void ReadMaterials(
		std::ifstream& fin,
		int numMaterials,
		MeshMaterial* materials,
		M3dMaterial* materialsParams);

	void SetupMaterials(
		BasicModel& model,
		const int numMaterials,
		const M3dMaterial* m3dMats);

	void ReadSubsetTable(
		std::ifstream& fin,
		int numSubsets,
		MeshGeometry::Subset* subsets);

	void ReadVertices(
		std::ifstream& fin,
		int numVertices,
		Vertex3D* vertices);

	void ReadTriangles(
		std::ifstream& fin,
		int numTriangles,
		UINT* indices);
};

} // namespace Core