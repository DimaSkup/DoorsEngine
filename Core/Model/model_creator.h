////////////////////////////////////////////////////////////////////////////////////////////
// Filename:        ModelsCreator.h
// Description:     a functional for mesh/models creation
//                  (plane, cube, sphere, imported models, etc.)
//
// Created:         12.02.24
////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <Types.h>
#include "../Mesh/mesh_gen_helper_types.h"


namespace Core
{

class ModelsCreator
{
public:
    ModelsCreator();

    ModelID CreateFromDE3D  (const char* modelPath);
    ModelID ImportFromM3D   (const char* modelPath);
    ModelID ImportFromFile  (const char* modelPath);

    ModelID CreateSkyDome   (const float radius, const int sliceCount, const int stackCount);
    ModelID CreatePlane     (const float width = 1.0f, const float height = 1.0f);

    ModelID CreateTreeLod1(
        const float planeWidth,
        const float planeHeight,
        const bool originAtBottom,
        const float rotateAroundX);

    ModelID CreateCube      (void);
    ModelID CreateSphere    (const MeshSphereParams& params);
    ModelID CreateGeoSphere (const MeshGeosphereParams& params);

    ModelID CreateCylinder  (const MeshCylinderParams& params);
    ModelID CreateSkull     (void);
    ModelID CreatePyramid   (const MeshPyramidParams& params = NULL);

    // creators for the specific types of models
    void    CreateSkyCube   (const float height);
    void    CreateSkySphere (const float radius, const int sliceCount, const int stackCount);
    bool    CreateTerrain   (const char* configFilename);

private:
    //void    ReadSkullMeshFromFile(BasicModel& model, const char* filepath);
};

} // namespace Core
