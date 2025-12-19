////////////////////////////////////////////////////////////////////////////////////////////
// Filename:    GeometryGenerator.h
// Description: this class is a utility class for generating simple geometric shapes
//              (for instance: grid, cylinder, sphere, box, etc.)
//
// Created:     13.03.24
////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "../Mesh/vertex.h"
#include "../Model/sky_model.h"
#include "../Mesh/mesh_gen_helper_types.h"
#include "../Mesh/material.h"
#include "../Model/basic_model.h"
#include "../Terrain/TerrainGeomipmapped.h"


namespace Core
{

class GeometryGenerator
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

    void GenerateTreeLod1(
        BasicModel& model,
        const float planeWidth,
        const float planeHeight,
        const bool originAtBottom,
        const float rotateAroundX);

    void GenerateCube(BasicModel& model);

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

    void GenerateTerrainFlatGrid(
        const int widht,
        const int depth,
        const int m,
        const int n,
        Vertex3dTerrain** outVertices,
        UINT** outIndices,
        int& outNumVertices,
        int& outNumIndices);

    void GeneratePyramid(
        const float height,
        const float baseWidth,
        const float baseDepth,
        BasicModel& model);

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

    bool GenSkyPlane(
        const int skyPlaneResolution,
        const float skyPlaneWidth,
        const float skyPlaneTop,
        const float skyPlaneBottom,
        const int texRepeat,
        VertexPosTex** outVertices,
        uint16** outIndices,
        int& outNumVertices,
        int& outNumIndices);

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
