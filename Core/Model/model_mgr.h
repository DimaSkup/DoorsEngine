// ************************************************************************************
// Filename:      ModelMgr.h
// Description:   a main single storage for the models (BasicModel)
// 
// Created:       30.10.24
// ************************************************************************************
#pragma once

#include <types.h>

#include "sky_plane.h"
#include "sky_model.h"
#include "../Terrain/Terrain.h"
#include "model.h"

#include <math/vec2.h>
#include <math/vec3.h>
#include <cvector.h>

namespace Core
{

constexpr uint NUM_VERTS_PER_DECAL = 4;
constexpr uint MAX_NUM_DECALS = 128;

// 3D decal
struct Decal3D
{
    Vec3  pos[4];
    Vec2  tex[4];
    Vec3  normal;         // normal vector of this decal
    float age;            // current age of this decal
    float lifeTimeSec;    // lifespan of this decal (is used to make decals dissapear)
                          // if == 0, decal won't dissapear during time
};

//---------------------------------------------------------

class ModelMgr
{
public:
    ModelMgr();
    ~ModelMgr();

    // for debug
    void PrintDump(void) const;

    bool Init(void);
    void Update(const float deltaTime);
    void Shutdown(void);

    bool InitDebugLinesBuffers();
    void ShutdownDebugLinesBuffers();

    ModelID                         AddModel(Model&& model);
    Model&                          AddEmptyModel(void);

    int                             GetNumAssets(void) const;
    void                            SetModelName(const ModelID id, const char* newName);

    void GetModelsByIds(
        const ModelID* ids,
        const size numModels,
        cvector<const Model*>& outModels);

    Model&                          GetModelById    (const ModelID id);
    Model&                          GetModelByName  (const char* name);
    ModelID                         GetModelIdByName(const char* name);

    void                            GetModelsNamesList(cvector<ModelName>& names);
    const cvector<ModelName>*       GetModelsNamesArrPtr(void) const;
    

    Terrain&                        GetTerrain(void);
    SkyModel&                       GetSky(void);
    SkyPlane&                       GetSkyPlane(void);

    // get buffers...
    VertexBuffer<BillboardSprite>&  GetBillboardsBuffer(void);
    VertexBuffer<VertexDecal3D>&    GetDecalsVB(void);
    IndexBuffer<uint16>&            GetDecalsIB(void);
    VertexBuffer<VertexPosColor>&   GetDebugLinesVB(void);
    IndexBuffer<uint16>&            GetDebugLinesIB(void);

    uint32                          GetNumDecals(void) const;

    void AddDecal3D(
        const Vec3& center,
        const Vec3& dir,
        const Vec3& normal,
        const float width,
        const float height,
        const float lifeTimeSec = 0.0f);


private:
    bool InitBillboardsVB();
    bool InitDecalsBuffers();

    void UpdateDynamicDecals(const float dt);
    
private:
    // specific buffers
    VertexBuffer<BillboardSprite> billboardsVB_;      // billboards/sprites/particles
    VertexBuffer<VertexDecal3D>   decalsVB_;
    IndexBuffer<uint16>           decalsIB_;
    VertexBuffer<VertexPosColor>  debugLinesVB_;
    IndexBuffer<uint16>           debugLinesIB_;


    // models related stuff
    cvector<ModelID>    ids_;
    cvector<Model>      models_;
    cvector<ModelName>  names_;

    SkyModel            sky_;
    SkyPlane            skyPlane_;
    Terrain             terrainGeomip_;

    Decal3D             decalsRendList_[MAX_NUM_DECALS];
    uint32              numDecals_ = 0;
    bool                bNeedUpdateDecalsVB_ = false;

    static ModelID      lastModelID_;
};


//==================================================================================
// GLOBAL instance of the model manager
//==================================================================================
extern ModelMgr g_ModelMgr;


//==================================================================================
// INLINE methods
//==================================================================================
inline const cvector<ModelName>* ModelMgr::GetModelsNamesArrPtr() const
{
    return &names_;
}

inline Terrain& ModelMgr::GetTerrain(void)
{
    return terrainGeomip_;
}

inline SkyModel& ModelMgr::GetSky(void)
{
    return sky_;
}

inline SkyPlane& ModelMgr::GetSkyPlane(void)
{
    return skyPlane_;
}

inline VertexBuffer<BillboardSprite>& ModelMgr::GetBillboardsBuffer(void)
{
    return billboardsVB_;
}

inline VertexBuffer<VertexDecal3D>& ModelMgr::GetDecalsVB(void)
{
    return decalsVB_;
}

inline IndexBuffer<uint16>& ModelMgr::GetDecalsIB(void)
{
    return decalsIB_;
}

inline VertexBuffer<VertexPosColor>& ModelMgr::GetDebugLinesVB(void)
{
    return debugLinesVB_;
}

inline IndexBuffer<uint16>& ModelMgr::GetDebugLinesIB(void)
{
    return debugLinesIB_;
}

inline uint32 ModelMgr::GetNumDecals(void) const
{
    return numDecals_;
}

inline int ModelMgr::GetNumAssets(void) const
{
    return (int)std::ssize(ids_);
}

}
