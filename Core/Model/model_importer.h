////////////////////////////////////////////////////////////////////
// Filename:      model_importer.h
// Description:   ASSIMP wrapper
//
//                imports a new model from the file of type:
//                .blend, .fbx, .3ds, .obj, etc.
// 
// Created:       05.07.23
////////////////////////////////////////////////////////////////////
#pragma once

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/material.h>

#include <d3d11.h>


namespace Core
{

// forward declaration
class BasicModel;

//---------------------------------------------------------

class ModelImporter
{
public:
    ModelImporter() {};
    
    bool LoadFromFile(ID3D11Device* pDevice, BasicModel* pModel, const char* filePath);
};

} // namespace Core
