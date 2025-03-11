// *********************************************************************************
// Filename:     ModelExporter.h
// Description:  exports models which were imported or manually generated into
//               the .de3d format
// 
// Created:      11.11.24
// *********************************************************************************
#pragma once

#include "BasicModel.h"
#include <fstream>


namespace Core
{

class ModelExporter
{
public:
	ModelExporter();

	void ExportIntoDE3D(
		ID3D11Device* pDevice,
		const BasicModel& model, 
		const std::string& path);

private:
	void WriteHeader(std::ofstream& fout, const BasicModel& model);

	void WriteMaterials(
		ID3D11Device* pDevice,
		std::ofstream& fout, 
		const BasicModel& model,
		const std::string& targetDirFullPath);

	void WriteSubsetTable(
		std::ofstream& fout, 
		const MeshGeometry::Subset* subsets,
		const int numSubsets);

	void WriteModelSubsetsAABB(
		std::ofstream& fout,
		const DirectX::BoundingBox& modelAABB,
		const DirectX::BoundingBox* subsetsAABBs,
		const int numAABB);

	void WriteVertices(
		std::ofstream& fout, 
		const Vertex3D* vertices,
		const int numVertices);

	void WriteIndices(
		std::ofstream& fout,
		const UINT* indices,
		const int numIndices);

	void WriteAABB(std::ofstream& fout, const DirectX::BoundingBox& aabb);
	void WriteFLOAT3(std::ofstream& fout, const DirectX::XMFLOAT3& data);
	void WriteMaterialProps(std::ofstream& fout, const MeshMaterial& mat);
};

} // namespace Core