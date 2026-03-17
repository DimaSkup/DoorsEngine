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

namespace Core
{

// forward declaration (pointer use only)
class Model;

//---------------------------------------------------------

class ModelImporter
{
public:
    ModelImporter() {};
    
    bool LoadFromFile(Model* pModel, const char* filePath);
};

} // namespace Core
