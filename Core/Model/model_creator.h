////////////////////////////////////////////////////////////////////////////////////////////
// Filename:        ModelsCreator.h
// Description:     a functional for mesh/models creation
//                  (plane, cube, sphere, imported models, etc.)
//
// Created:         12.02.24
////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "../Model/basic_model.h"
#include "../Mesh/mesh_gen_helper_types.h"
#include <Types.h>


namespace Core
{

class ModelsCreator
{
public:
    ModelsCreator();

    ModelID CreateFromDE3D(ID3D11Device* pDevice, const char* modelPath);
    ModelID ImportFromM3D (ID3D11Device* pDevice, const char* modelPath);
    ModelID ImportFromFile(ID3D11Device* pDevice, const char* modelPath);
    

    // create a model according to its type and with default params
    ModelID Create(ID3D11Device* pDevice, const eModelType type);

    ModelID CreateSkyDome(ID3D11Device* pDevice, const float radius, const int sliceCount, const int stackCount);
    ModelID CreatePlane(ID3D11Device* pDevice, const float width = 1.0f, const float height = 1.0f);

    ModelID CreateTreeLod1(
        ID3D11Device* pDevice,
        const float planeWidth,
        const float planeHeight,
        const bool originAtBottom,
        const float rotateAroundX);

    ModelID CreateCube(ID3D11Device* pDevice);
    
    ModelID CreateSphere(ID3D11Device* pDevice, const MeshSphereParams& params);
    ModelID CreateGeoSphere(ID3D11Device* pDevice, const MeshGeosphereParams& params);

    ModelID CreateCylinder(ID3D11Device* pDevice, const MeshCylinderParams& params);
    ModelID CreateSkull(ID3D11Device* pDevice);
    ModelID CreatePyramid(ID3D11Device* pDevice, const MeshPyramidParams& params = NULL);

    // creators for the specific types of models
    void CreateSkyCube            (ID3D11Device* pDevice, const float height);
    void CreateSkySphere          (ID3D11Device* pDevice, const float radius, const int sliceCount, const int stackCount);
    bool CreateTerrain            (const char* configFilename);

private:
    void ReadSkullMeshFromFile(BasicModel& model, const char* filepath);

#if 0
    

    void PaintGridWithRainbow(Mesh::MeshData & grid,
        const UINT verticesCountByX,
        const UINT verticesCountByZ);

    void PaintGridAccordingToHeights(Mesh::MeshData & grid);
#endif
};

} // namespace Core
