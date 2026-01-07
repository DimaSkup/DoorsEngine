// ************************************************************************************
// Filename:      ModelMgr.h
// Description:   a main single storage for the models (BasicModel)
// 
// Created:       30.10.24
// ************************************************************************************
#pragma once

#include "sky_plane.h"
#include "sky_model.h"
#include "../Terrain/TerrainGeomipmapped.h"
#include "basic_model.h"

#include <Types.h>
#include <cvector.h>

namespace Core
{
constexpr uint NUM_VERTS_PER_DECAL = 6;
constexpr uint MAX_NUM_DECALS = 128;

// 3D decal
struct Decal
{
    VertexPosTex vertices[NUM_VERTS_PER_DECAL];    // endpoints of the decal surface
};

//---------------------------------------------------------

class ModelMgr
{
public:
    ModelMgr();
    ~ModelMgr();

    // for debug
    void PrintDump() const;

    bool Init();
    void Shutdown();
   

    ModelID     AddModel(BasicModel&& model);
    BasicModel& AddEmptyModel();

    void        GetModelsByIDs  (const ModelID* ids, const size numModels, cvector<const BasicModel*>& outModels);
    BasicModel& GetModelById    (const ModelID id);
    BasicModel& GetModelByName  (const char* name);
    ModelID     GetModelIdByName(const char* name);

    void                      GetModelsNamesList(cvector<ModelName>& names);
    const cvector<ModelName>* GetModelsNamesArrPtr() const { return &names_; }
    

    inline TerrainGeomip&                 GetTerrainGeomip()    { return terrainGeomip_; }
    inline SkyModel&                      GetSky()              { return sky_; }
    inline SkyPlane&                      GetSkyPlane()         { return skyPlane_; }

    inline VertexBuffer<BillboardSprite>& GetBillboardsBuffer() { return billboardsVB_; }

    inline VertexBuffer<VertexPosTex>&    GetDecalsVB()         { return decalsVB_; }

    inline VertexBuffer<VertexPosColor>&  GetDebugLinesVB()     { return debugLinesVB_;}
    inline IndexBuffer<uint16>&           GetDebugLinesIB()     { return debugLinesIB_;}

    inline int                            GetNumAssets() const  { return (int)std::ssize(ids_); }

    void SetModelName(const ModelID id, const char* newName);

    void PushDecalToRender(
        const Vec3& center,
        const Vec3& direction,
        const Vec3& normal,
        const float width,
        const float height);

private:
    bool InitBillboardBuffer();
    bool InitLineVertexBuffer();

    bool IsIdxValid(const index idx) const;
    
private:
    // specific buffers
    VertexBuffer<BillboardSprite> billboardsVB_;      // billboards/sprites/particles
    VertexBuffer<VertexPosTex>    decalsVB_;
    VertexBuffer<VertexPosColor>  debugLinesVB_;
    IndexBuffer<uint16>           debugLinesIB_;

    

    // models related stuff
    cvector<ModelID>    ids_;
    cvector<BasicModel> models_;
    cvector<ModelName>  names_;

    SkyModel            sky_;
    SkyPlane            skyPlane_;
    TerrainGeomip       terrainGeomip_;

    Decal               decalsRenderList_[MAX_NUM_DECALS];
    uint                decalIdx_ = 0;

    static ModelID      lastModelID_;
};


// =================================================================================
// Declare a global instance of the model manager
// =================================================================================
extern ModelMgr g_ModelMgr;


//-----------------------------------------------------
// Desc:  check if input index of model is in proper range
//-----------------------------------------------------
inline bool ModelMgr::IsIdxValid(const index idx) const
{
    return (idx >= 0 && idx < ids_.size());
}

}
