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
#include "BasicModel.h"

namespace Core
{

class ModelMgr
{
public:
    ModelMgr();

    bool        Init();

    bool        InitBillboardBuffer();
    bool        InitGrassBuffer();

    void        Serialize  (ID3D11Device* pDevice);
    void        Deserialize(ID3D11Device* pDevice);

    ModelID     AddModel(BasicModel&& model);
    BasicModel& AddEmptyModel();

    void        GetModelsByIDs  (const ModelID* ids, const size numModels, cvector<const BasicModel*>& outModels);
    BasicModel& GetModelById    (const ModelID id);
    BasicModel& GetModelByName  (const char* name);
    ModelID     GetModelIdByName(const char* name);

    void        GetModelsNamesList(cvector<ModelName>& names);
    

    //inline Terrain&             GetTerrain()         { return terrain_; }
    inline TerrainGeomip&   GetTerrainGeomip()   { return terrainGeomip_; }
    inline SkyModel&        GetSky()             { return sky_; }

    inline VertexBuffer<BillboardSprite>& GetBillboardsBuffer() { return billboardsVB_; }
    inline VertexBuffer<VertexGrass>&     GetGrassVB() { return grassVB_; }
    inline cvector<VertexGrass>&          GetGrassVertices() { return grassVertices_; }

    inline int                  GetNumAssets() const { return (int)std::ssize(ids_); }


    
private:
    VertexBuffer<BillboardSprite> billboardsVB_;
    VertexBuffer<VertexGrass>     grassVB_;
    cvector<VertexGrass>          grassVertices_;

    cvector<ModelID>    ids_;
    cvector<BasicModel> models_;

    SkyModel            sky_;
    //Terrain             terrain_;
    TerrainGeomip       terrainGeomip_;

    static ModelMgr*    pInstance_;
    static ModelID      lastModelID_;

};


// =================================================================================
// Declare a global instance of the model manager
// =================================================================================
extern ModelMgr g_ModelMgr;

}
