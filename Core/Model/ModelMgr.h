// ************************************************************************************
// Filename:      ModelMgr.h
// Description:   a main single storage for the models (BasicModel)
// 
// Created:       30.10.24
// ************************************************************************************
#pragma once

#include <Types.h>
#include <cvector.h>
#include "SkyModel.h"
#include "../Terrain/TerrainGeomipmapped.h"
#include "../Terrain/TerrainQuadtree.h"
#include "BasicModel.h"

namespace Core
{

class ModelMgr
{
public:
    ModelMgr();

    void        Serialize  (ID3D11Device* pDevice);
    void        Deserialize(ID3D11Device* pDevice);

    ModelID     AddModel(BasicModel&& model);
    BasicModel& AddEmptyModel();

    void        GetModelsByIDs  (const ModelID* ids, const size numModels, cvector<const BasicModel*>& outModels);
    BasicModel& GetModelByID    (const ModelID id);
    BasicModel& GetModelByName  (const char* name);
    ModelID     GetModelIdByName(const char* name);

    void        GetModelsNamesList(cvector<ModelName>& names);

    //inline Terrain&             GetTerrain()         { return terrain_; }
    inline TerrainGeomipmapped& GetTerrainGeomip()   { return terrainGeomip_; }
    inline TerrainQuadtree&     GetTerrainQuadtree() { return terrainQuadtree_; }
    inline SkyModel&            GetSky()             { return sky_; }

    inline int                  GetNumAssets() const { return (int)std::ssize(ids_); }

    
private:
    cvector<ModelID>    ids_;
    cvector<BasicModel> models_;

    SkyModel            sky_;
    //Terrain             terrain_;
    TerrainGeomipmapped terrainGeomip_;
    TerrainQuadtree     terrainQuadtree_;

    static ModelMgr*    pInstance_;
    static ModelID      lastModelID_;
};


// =================================================================================
// Declare a global instance of the model manager
// =================================================================================
extern ModelMgr g_ModelMgr;

}
