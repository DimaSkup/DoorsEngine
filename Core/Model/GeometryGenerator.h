////////////////////////////////////////////////////////////////////////////////////////////
// Filename:    GeometryGenerator.h
// Description: this class is a utility class for generating simple geometric shapes
//              (for instance: grid, cylinder, sphere, box, etc.)
//
// Created:     13.03.24
////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "../Mesh/Vertex.h"
#include "../Model/SkyModel.h"
#include "../Mesh/MeshHelperTypes.h"
#include "../Mesh/Material.h"
#include "../Model/BasicModel.h"


namespace Core
{

class GeometryGenerator final
{
private:
	struct CylinderTempData
	{
		// transient data which is used during a cylinder mesh generation
		float* tu = nullptr;                        // texture X coords
		float* thetaSines = nullptr;              // precomputed sin/cos for each Theta value
		float* thetaCosines = nullptr;
		int lastVertexIdx = 0;
		int lastIndexIdx = 0;
	};



public:
	GeometryGenerator() {};

	void GenerateAxis(BasicModel& model);
	void GenerateCube(BasicModel& model);
	void GenerateLineBox(BasicModel& model);

	void GenerateSkyBoxForCubeMap(ID3D11Device* pDevice, SkyModel& sky, const float height);
	void GenerateSkyBoxForAtlasTex(BasicModel& model, const float height);

	void GenerateSkySphere(
		ID3D11Device* pDevice,
		SkyModel& sky,
		const float radius,
		const int sliceCount,
		const int stackCount);

	void GeneratePlane(
		const float width,
		const float height,
		BasicModel& model);

	void GenerateFlatGrid(
		const float widht,
		const float depth,
		const int m,
		const int n,
		BasicModel& model);

	void GeneratePyramid(
		const float height,
		const float baseWidth,
		const float baseDepth,
		BasicModel& model);

#if 0
	void GenerateWaves(
		const UINT numRows,
		const UINT numColumns,
		const float spatialStep,
		const float timeStep,
		const float speed,
		const float damping,
		Waves & waves,
		BasicModel& model);
#endif

	void GenerateCylinder(
		const MeshCylinderParams& params,
		BasicModel& model);

	void GenerateSphere(
		const MeshSphereParams& params,
		BasicModel& model);

	void GenerateSkyDome(
		const float radius,
		const int sliceCount,
		const int stackCount,
		BasicModel& model);

	void GenerateGeosphere(
		const float radius,
		int numSubdivisions,
		BasicModel& model);

	void ComputeAABB(
		const Vertex3D* vertices,
		const int numVertices,
		DirectX::BoundingBox& aabb);

private:
	// helper functions for a cube creation
	void SetupCubeVerticesPositions(DirectX::XMFLOAT3* positions);
	void SetupCubeFacesNormals(DirectX::XMFLOAT3* facesNormals);

	// cylinder private creation API 
	// (BuildCylinderStacks, BuildCylinderTopCap, BuildCylinderBottomCap)
	void BuildCylinderStacks(
		const MeshCylinderParams& params,
		CylinderTempData& tempData,
		BasicModel& model);

	void BuildCylinderTopCap(
		const MeshCylinderParams& params,
		CylinderTempData& tempData,
		BasicModel& model);

	void BuildCylinderBottomCap(
		const MeshCylinderParams& params,
		CylinderTempData& tempData,
		BasicModel& model);

	void BuildCylinderCapRingVertices(
		const MeshCylinderParams& params,
		const bool isTopCap,
		CylinderTempData& tempData,
		BasicModel& model);

	// helper function for a geosphere creation
	void Subdivide(BasicModel& model);
};

} // namespace Core
