////////////////////////////////////////////////////////////////////
// Filename:      ModelImporter.h
// Description:   imports a new model from the file of type:
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
	
	bool LoadFromFile(ID3D11Device* pDevice, BasicModel& model, const char* filePath);


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
		const DirectX::XMMATRIX& parentTrasformMatrix,
		const char* filePath);

	void ProcessMesh(
		ID3D11Device* pDevice, 
		BasicModel& model,
		int& subsetIdx,
		const aiMesh* pMesh, 
		const aiScene* pScene, 
		const DirectX::XMMATRIX & transformMatrix,
		const char* filePath);

	void LoadMaterialColorsData(
        aiMaterial* pMaterial,
        Material& mat);

	void LoadMaterialTextures(
		ID3D11Device* pDevice,
		TexID* texIDs,
		const aiMaterial* pMaterial,
		const MeshGeometry::Subset& subset,
		const aiScene* pScene,
		const char* filePath);

	void GetVerticesIndicesOfMesh(
		const aiMesh* pMesh,
		BasicModel& model,
		MeshGeometry::Subset& subset,
		const int subsetIdx);

	void ExecuteModelMathCalculations(Vertex3D* vertices, const int numVertices);

};

} // namespace Core
