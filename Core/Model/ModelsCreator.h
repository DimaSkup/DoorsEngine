////////////////////////////////////////////////////////////////////////////////////////////
// Filename:        ModelsCreator.h
// Description:     a functional for mesh/models creation
//                  (plane, cube, sphere, imported models, etc.)
//
// Created:         12.02.24
////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
   

#include "../Common/Types.h"
#include "../Model/BasicModel.h"
#include "../Mesh/MeshHelperTypes.h"

#include "../Texture/TextureMgr.h"


class ModelsCreator
{
public:
	ModelsCreator();

	ModelID CreateFromDE3D(ID3D11Device* pDevice, const std::string& path);
	ModelID ImportFromFile(ID3D11Device* pDevice, const std::string& filepath);

	// create a model according to its type and with default params
	ModelID Create(ID3D11Device* pDevice, const ModelType type);

	ModelID CreateSkyDome(ID3D11Device* pDevice, const float radius, const int sliceCount, const int stackCount);
	ModelID CreatePlane(ID3D11Device* pDevice, const float width = 1.0f, const float height = 1.0f);
	ModelID CreateBoundingLineBox(ID3D11Device* pDevice);
	ModelID CreateCube(ID3D11Device* pDevice);
	void CreateSkyCube(ID3D11Device* pDevice, const float height);
	void CreateSkySphere(ID3D11Device* pDevice, const float radius, const int sliceCount, const int stackCount);

	ModelID CreateSphere(ID3D11Device* pDevice, const MeshSphereParams& params);
	ModelID CreateGeoSphere(ID3D11Device* pDevice, const MeshGeosphereParams& params);

	ModelID CreateCylinder(ID3D11Device* pDevice, const MeshCylinderParams& params);
	ModelID CreateSkull(ID3D11Device* pDevice);
	ModelID CreatePyramid(ID3D11Device* pDevice, const MeshPyramidParams& params = NULL);
	ModelID CreateGrid(ID3D11Device* pDevice, const u32 width, const u32 depth);
	ModelID CreateWater(ID3D11Device* pDevice, const float width, const float depth);

	ModelID CreateGeneratedTerrain(
		ID3D11Device* pDevice,
		const float terrainWidth,
		const float terrainDepth,
		const int verticesCountByX,
		const int verticesCountByZ);

	
private:
	void ReadSkullMeshFromFile(BasicModel& model, const std::string& filepath);

#if 0
	

	void PaintGridWithRainbow(Mesh::MeshData & grid,
		const UINT verticesCountByX,
		const UINT verticesCountByZ);

	void PaintGridAccordingToHeights(Mesh::MeshData & grid);
#endif
};



