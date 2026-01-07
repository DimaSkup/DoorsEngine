// ************************************************************************************
// Filename:      ModelMgr.cpp
// Created:       30.10.24
// ************************************************************************************
#include <CoreCommon/pch.h>
#include "model_mgr.h"

#include "model_creator.h"
#include "model_exporter.h"
#include <Render/d3dclass.h>

namespace fs = std::filesystem;


namespace Core
{

// static arr of indices (for internal purposes)
static cvector<index> s_Idxs;

// init a global instance of the model manager
ModelMgr g_ModelMgr;

// when we add some new model into a storage its ID == lastModelID
// and then we increase the lastModelID_ by 1, so the next model will have as ID
// this increased value, and so on
ModelID ModelMgr::lastModelID_ = 0;


//---------------------------------------------------------
// a little cringe helper to wrap getting of DX11 device and context
//---------------------------------------------------------
inline ID3D11Device* GetDevice()
{
    return Render::g_pDevice;
}

inline ID3D11DeviceContext* GetContext()
{
    return Render::g_pContext;
}

//---------------------------------------------------------
// default constructor and destructor
//---------------------------------------------------------
ModelMgr::ModelMgr()
{
    // reserve some memory ahead
    const size reserve = 128;
    ids_.reserve(reserve);
    models_.reserve(reserve);
    names_.reserve(reserve);
}

//---------------------------------------------------------

ModelMgr::~ModelMgr()
{
    Shutdown();
}

//---------------------------------------------------------
// Desc:   initialize some stuff 
//---------------------------------------------------------
bool ModelMgr::Init()
{
    // create and setup "invalid" model
    ModelsCreator creator;
    const ModelID cubeId = creator.CreateCube();

    // currently we expect "invalid" model to be only one and have ID == 0
    if ((cubeId != INVALID_MODEL_ID) || (ids_.size() != 1) || (models_.size() != 1))
    {
        LogErr(LOG, "something went wrong: there is already some other models");
        return false;
    }

    BasicModel& invalid = g_ModelMgr.GetModelById(cubeId);
    g_ModelMgr.SetModelName(invalid.id_, "invalid_model");
    invalid.SetMaterialForSubset(0, INVALID_MATERIAL_ID);

    //---------------------------------

    if (!InitBillboardBuffer())
    {
        LogErr(LOG, "can't init billboard/particles VB");
        return false;
    }

    if (!InitLineVertexBuffer())
    {
        LogErr(LOG, "can't init debug lines VB");
        return false;
    }

    //---------------------------------------------

    // init DECALS buffers
    const bool isDynamicVB = true;
    constexpr int numDecalsVertices = MAX_NUM_DECALS * NUM_VERTS_PER_DECAL;
    VertexPosTex decalsVertices[numDecalsVertices];

    memset(decalsVertices, 0, sizeof(decalsVertices));
    memset(decalsRenderList_, 0, sizeof(decalsRenderList_));

    if (!decalsVB_.Initialize(GetDevice(), decalsVertices, numDecalsVertices, isDynamicVB))
    {
        LogErr(LOG, "can't init decals VB");
        return false;
    }
    

    return true;
}

//---------------------------------------------------------
// Desc:   release memory
//---------------------------------------------------------
void ModelMgr::Shutdown()
{
    billboardsVB_.Shutdown();
    debugLinesVB_.Shutdown();
    debugLinesIB_.Shutdown();

    ids_.purge();
    models_.purge();
    names_.purge();

    sky_.Shutdown();
    terrainGeomip_.Shutdown();
}

//---------------------------------------------------------
// Desc:  init a billboards vertex buffer (mainly is used for particles)
//---------------------------------------------------------
bool ModelMgr::InitBillboardBuffer()
{
    constexpr int  maxNumBillboards = 30000;
    constexpr bool isDynamic = true;

    cvector<BillboardSprite> vertices(maxNumBillboards);

    if (!billboardsVB_.Initialize(
        GetDevice(),
        vertices.data(),
        maxNumBillboards,
        isDynamic))
    {
        LogErr(LOG, "can't create a vertex buffer for billboard sprites");
        return false;
    }

    return true;
}

//---------------------------------------------------------
// Desc:   initialize a vertex buffer for debug lines
//---------------------------------------------------------
bool ModelMgr::InitLineVertexBuffer()
{
    // TODO: I hate todo but fix it to prevent eating so much memory
    constexpr int  maxNumVertices = 120000;
    constexpr int  maxNumIndices = 256;
    constexpr bool isDynamic = true;

    cvector<VertexPosColor> vertices(maxNumVertices);
    cvector<uint16>         indices(maxNumIndices, 0);

    if (!debugLinesVB_.Initialize(
        GetDevice(),
        vertices.data(),
        maxNumVertices,
        isDynamic))
    {
        LogErr(LOG, "can't create a vertex buffer for debug lines");
        return false;
    }

    if (!debugLinesIB_.Initialize(
        GetDevice(),
        indices.data(),
        maxNumIndices,
        isDynamic))
    {
        LogErr(LOG, "can't create an index buffer for debug lines");
        return false;
    }

    return true;
}

//---------------------------------------------------------
// Desc:  push an input model into the storage
// Args:  - model:   a model which will be moved into the manager
// Ret:   identifier of added model
//---------------------------------------------------------
ModelID ModelMgr::AddModel(BasicModel&& model)
{
    bool isUnique = !ids_.binary_search(model.id_);
    if (!isUnique)
    {
        PrintDump();

        LogErr(LOG, "there is already a model by ID: %" PRIu32, model.id_);
        return INVALID_MODEL_ID;
    }

    const ModelID id = model.id_;
    const index idx = ids_.get_insert_idx(model.id_);

    names_.insert_before(idx, ModelName());
    strncpy(names_[idx].name, model.name_, MAX_LEN_MODEL_NAME);

    ids_.insert_before(idx, id);
    models_.insert_before(idx, std::move(model));
    

    if (id >= lastModelID_)
        lastModelID_ = id + 1;

    return id;
}

//---------------------------------------------------------
// Desc:   push a new empty model into the storage
// Ret:    a ref to this new model
//---------------------------------------------------------
BasicModel& ModelMgr::AddEmptyModel()
{
    const ModelID id = lastModelID_;
    ++lastModelID_;

    ids_.push_back(id);
    models_.push_back(BasicModel());

    // add an "invalid" name
    names_.push_back(ModelName());

    BasicModel& model = models_.back();
    ModelName& name = names_.back();

    strcpy(name.name, "invalid");
    strcpy(model.name_, "invalid");
    model.id_ = id;

    return model;
}

//---------------------------------------------------------
// Out:   array of pointers to models by input Ids
//---------------------------------------------------------
void ModelMgr::GetModelsByIDs(
    const ModelID* ids,
    const size numModels,
    cvector<const BasicModel*>& outModels)
{
    if (ids == nullptr || numModels == 0)
    {
        LogErr(LOG, "input args are invalid (ids arr == null OR num of models == 0)");
        return;
    }

    // get idxs by IDs
    ids_.get_idxs(ids, numModels, s_Idxs);

    // check idxs
    for (const index idx : s_Idxs)
        assert(IsIdxValid(idx));


    // get pointers by idxs
    outModels.resize(numModels);

    for (int i = 0; const index idx : s_Idxs)
        outModels[i++] = &models_[idx];
}

//---------------------------------------------------------
// return a model by ID, or invalid model (by idx == 0) if there is no such ID
//---------------------------------------------------------
BasicModel& ModelMgr::GetModelById(const ModelID id)
{
    const index idx = ids_.get_idx(id);

    if (IsIdxValid(idx))
        return models_[idx];

    return models_[INVALID_MODEL_ID];
}

//---------------------------------------------------------
// Desc:   get a model by input name
//---------------------------------------------------------
BasicModel& ModelMgr::GetModelByName(const char* name)
{
    if (StrHelper::IsEmpty(name))
    {
        LogErr(LOG, "input name is empty");
        return models_[INVALID_MODEL_ID];
    }

    for (index i = 0; i < names_.size(); ++i)
    {
        if (strcmp(names_[i].name, name) == 0)
            return models_[i];
    }

    // return an empty model if we didn't find any
    LogErr(LOG, "there is no model by name: %s", name);
    return models_[INVALID_MODEL_ID];
}
//---------------------------------------------------------
// Desc:   get a model ID by input name
//---------------------------------------------------------
ModelID ModelMgr::GetModelIdByName(const char* name)
{
    if (StrHelper::IsEmpty(name))
    {
        LogErr("input name is empty");
        return ids_[0];                     // return empty model (actually cube)
    }

    for (index i = 0; i < names_.size(); ++i)
    {
        if (strcmp(names_[i].name, name) == 0)
            return ids_[i];
    }

    // return an empty model ID if we didn't find any
    LogErr(LOG, "there is no model by name: %s", name);
    return ids_[0];
}

//---------------------------------------------------------
// Desc:   fill in the input array with names of the assets from the storage
//---------------------------------------------------------
void ModelMgr::GetModelsNamesList(cvector<ModelName>& names)
{
    const int numNames = GetNumAssets();
    names.resize(numNames);

    for (int i = 0; i < numNames; ++i)
        strcpy(names[i].name, models_[i].GetName());
}

//---------------------------------------------------------
// Desc:  we can change model's name properly only using this function
//        because we have also update the array of names with new value
//---------------------------------------------------------
void ModelMgr::SetModelName(const ModelID id, const char* name)
{
    if (StrHelper::IsEmpty(name))
    {
        LogErr(LOG, "input name is empty");
        return;
    }

    const index idx = ids_.get_idx(id);
    if (!IsIdxValid(idx))
    {
        LogErr(LOG, "there is no model by id: %" PRIu32, id);
        return;
    }

    // clamp length of the name
    size_t len = strlen(name);

    if (len > MAX_LEN_MODEL_NAME - 1)
        len = MAX_LEN_MODEL_NAME - 1;

    // update model's name
    BasicModel& model = models_[idx];
    strncpy(model.name_, name, len);
    model.name_[len] = '\0';

    // update arr of names
    strncpy(names_[idx].name, name, len);
    names_[idx].name[len] = '\0';
}

//---------------------------------------------------------
// Desc:  generate and push a new decal into the decals rendering list
// 
// Args:  - center:     the collision point, or the center of the decal
//        - direction:  direction for the decal
//        - normal:     normal vector of decal surface
//        - width:      width of the decal
//        - height:     height of the decal
//---------------------------------------------------------
void ModelMgr::PushDecalToRender(
    const Vec3& center,
    const Vec3& direction,
    const Vec3& normal,
    const float width,
    const float height)
{
    if (width <= 0 || height <= 0)
    {
        LogErr(LOG, "invalid decal dimensions: %.3f x %.3f", width, height);
        return;
    }

    Vec3 u = direction;
    Vec3Normalize(u);

    Vec3 n = normal;
    Vec3Normalize(n);

    // v (we don't have to normalize the v vector because n and u is already normalized)
    Vec3 v = Vec3Cross(u, n);
    Vec3Normalize(v);


    u = u * height * 0.5f;
    v = v * width  * 0.5f;

    /*
      1 *---------------* 0
        |             / |
        |           /   |
        |         /     |
        |       /       |
        |     /         |
        |   /           |
        | /             |
      2 *---------------* 3
   */

    // a little offset for decal along the normal vector
    Vec3 offset = n * 0.01f;

    Vec3 v0 = center + u + v + offset;  // UR
    Vec3 v1 = center + u - v + offset;  // UL
    Vec3 v2 = center - u - v + offset;  // BL
    Vec3 v3 = center - u + v + offset;  // BR

    Vec2 t0 = { 1, 0 };
    Vec2 t1 = { 0, 0 };
    Vec2 t2 = { 0, 1 };
    Vec2 t3 = { 1, 1 };


    // generate endpoints of the decal
    Decal d;

    // first triangle
    d.vertices[0].pos = { v0.x, v0.y, v0.z };
    d.vertices[1].pos = { v1.x, v1.y, v1.z };
    d.vertices[2].pos = { v2.x, v2.y, v2.z };

    d.vertices[0].tex = { t0.u, t0.v };
    d.vertices[1].tex = { t1.u, t1.v };
    d.vertices[2].tex = { t2.u, t2.v };

    // second triangle
    d.vertices[3].pos = { v0.x, v0.y, v0.z };
    d.vertices[4].pos = { v2.x, v2.y, v2.z };
    d.vertices[5].pos = { v3.x, v3.y, v3.z };

    d.vertices[3].tex = { t0.u, t0.v };
    d.vertices[4].tex = { t2.u, t2.v };
    d.vertices[5].tex = { t3.u, t3.v };
    

    // push decal into the render list (our render list is a ring buffer)
    decalsRenderList_[decalIdx_++] = d;
    decalIdx_ %= MAX_NUM_DECALS;


    // update decals vertex buffer
    VertexPosTex* vertices = (VertexPosTex*)decalsRenderList_;
    uint          numVerts = MAX_NUM_DECALS * NUM_VERTS_PER_DECAL;

    if (!decalsVB_.UpdateDynamic(GetContext(), vertices, numVerts))
    {
        LogErr(LOG, "can't update decals vertex buffer");
    }
}

//---------------------------------------------------------
// Desc:   print a dump of all the models (BasicModel) in the model manager
//---------------------------------------------------------
void ModelMgr::PrintDump() const
{
    printf("\nALL MODELS DUMP: \n");

    for (int i = 0; i < (int)models_.size(); ++i)
    {
        // [idx]: (id: model_id) Name: model_name   lod1: model_id_lod_1;   lod2: model_id_lod_2
        printf("[%d]: (id: %4" PRIu32 ") Name:%-32s  lod1: %" PRIu32 ";  lod2: %" PRIu32 "\n",
            i,
            models_[i].id_,
            names_[i].name,
            models_[i].lods_[LOD_1],
            models_[i].lods_[LOD_2]);
    }

    printf("\n");
}


} // namespace Core
