////////////////////////////////////////////////////////////////////
// Filename:      ModelImporter.h
// Description:   loads a new model from the file of type:
//                .blend, .fbx, .3ds, .obj, etc.
// 
// Created:       05.07.23
////////////////////////////////////////////////////////////////////
#pragma once

#include "../Model/BasicModel.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/material.h>

#include <DirectXMath.h>
#include <d3d11.h>


namespace Core
{

class ModelImporter final
{
public:
	ModelImporter() {};

	
	void LoadFromFile(
		ID3D11Device* pDevice,
		BasicModel& model,
		const std::string & filePath);


    static double s_ImportDuration_;
    static double s_TexLoadingDuration_;
    static double s_VerticesLoading_;
    static double s_NodesLoading_;
    static double s_SceneLoading_;

private:
	void ComputeNumOfData(
		BasicModel& model,
		const aiNode* pNode,
		const aiScene* pScene);

	void ProcessNode(
		ID3D11Device* pDevice,
		BasicModel& model,
		int& subsetIdx,
		const aiNode* pNode, 
		const aiScene* pScene, 
		const DirectX::XMMATRIX & parentTrasformMatrix,
		const std::string & filePath);

	void ProcessMesh(
		ID3D11Device* pDevice, 
		BasicModel& model,
		int& subsetIdx,
		const aiMesh* pMesh, 
		const aiScene* pScene, 
		const DirectX::XMMATRIX & transformMatrix,
		const std::string & filePath);

	void SetMeshName(const aiMesh* pMesh, MeshGeometry::Subset& subset);

	void LoadMaterialColors(aiMaterial* pMaterial, MeshMaterial& mat);

	void LoadMaterialTextures(
		ID3D11Device* pDevice,
		TexID* texIDs,
		const aiMaterial* pMaterial,
		const MeshGeometry::Subset& subset,
		const aiScene* pScene,
		const std::string& filePath);

	void GetVerticesIndicesOfMesh(
		const aiMesh* pMesh,
		BasicModel& model,
		MeshGeometry::Subset& subset,
		const int subsetIdx);

	void ExecuteModelMathCalculations(Vertex3D* vertices, const int numVertices);

};

} // namespace Core
