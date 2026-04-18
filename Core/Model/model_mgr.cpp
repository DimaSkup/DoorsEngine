// ************************************************************************************
// Filename:      ModelMgr.cpp
// Created:       30.10.24
// ************************************************************************************
#include <CoreCommon/pch.h>
#include "model_mgr.h"

#include "model_creator.h"
#include "model_exporter.h"
#include <Render/d3dclass.h>


namespace Core
{

// some constants
#define NUM_BILLBOARDS         5000       // size of billboards/particles VB
#define NUM_VERTS_DBG_LINES_VB 30000
#define NUM_INDEX_DBG_LINES_IB 256


// static arrays for internal purposes
static cvector<index>         s_Idxs;
static cvector<VertexDecal3D> s_VertsDecals;

// init a global instance of the model manager
ModelMgr g_ModelMgr;

// value is used to get an ID for a new model
ModelID ModelMgr::lastModelID_ = 0;


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
    if ((cubeId != INVALID_MODEL_ID) ||
        (ids_.size() != 1) ||
        (models_.size() != 1))
    {
        LogErr(LOG, "there is already some other models");
        return false;
    }

    Model& invalidModel = g_ModelMgr.GetModelById(cubeId);
    
    g_ModelMgr.SetModelName(invalidModel.GetId(), "invalid_model");
    invalidModel.SetMaterialForSubset(0, INVALID_MAT_ID);

    //---------------------------------

    if (!InitBillboardsVB())
    {
        LogErr(LOG, "can't init VB for billboard/particles");
        return false;
    }

    if (!InitDecalsBuffers())
    {
        LogErr(LOG, "can't init buffers for decals");
        return false;
    }

    return true;
}

//---------------------------------------------------------
// Desc:  update some buffers (vb/ib) if necessary
//---------------------------------------------------------
void ModelMgr::Update(const float deltaTime)
{
    UpdateDynamicDecals(deltaTime);
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
bool ModelMgr::InitBillboardsVB()
{
    constexpr bool bIsDynamicVB = true;
    cvector<BillboardSprite> vertices(NUM_BILLBOARDS);

    if (!billboardsVB_.Init(vertices.data(), NUM_BILLBOARDS, bIsDynamicVB))
    {
        LogErr(LOG, "can't create a VB for billboard sprites");
        return false;
    }

    return true;
}

//---------------------------------------------------------
// Desc:   initialize a vertex/index buffer for debug lines
//---------------------------------------------------------
bool ModelMgr::InitDebugLinesBuffers()
{
    constexpr bool bDynamicBuf = true;

    // TODO: I hate todo but fix it to prevent eating so much memory
    cvector<VertexPosColor> vertices(NUM_VERTS_DBG_LINES_VB);
    cvector<uint16>         indices(NUM_INDEX_DBG_LINES_IB, 0);

    if (!debugLinesVB_.Init(vertices.data(), NUM_VERTS_DBG_LINES_VB, bDynamicBuf))
    {
        ShutdownDebugLinesBuffers();
        LogErr(LOG, "can't create a VB for debug lines");
        return false;
    }

    if (!debugLinesIB_.Init(indices.data(), NUM_INDEX_DBG_LINES_IB, bDynamicBuf))
    {
        ShutdownDebugLinesBuffers();
        LogErr(LOG, "can't create an IB for debug lines");
        return false;
    }

    return true;
}

//---------------------------------------------------------
// Desc:  release VRAM from debug lines VB and IB
//---------------------------------------------------------
void ModelMgr::ShutdownDebugLinesBuffers()
{
    debugLinesVB_.Shutdown();
    debugLinesIB_.Shutdown();
}

//---------------------------------------------------------
// Desc:  initialize buffers for decals
//---------------------------------------------------------
bool ModelMgr::InitDecalsBuffers()
{
    const bool    bDynamicVB   = true;
    const bool    bDynamicIB   = false;
    const int     numVerts     = MAX_NUM_DECALS * NUM_VERTS_PER_DECAL;
    const int     numIndices   = 6;
    ID3D11Device* pDevice      = Render::GetD3dDevice();    
    const uint16  indices[numIndices] = { 0,1,2,  0,2,3 };
    
    s_VertsDecals.resize(numVerts);
    VertexDecal3D* verts = s_VertsDecals.data();

    
    // reset with zeros CPU-side decals data
    memset(decalsRendList_, 0, sizeof(decalsRendList_));
    memset(verts,           0, sizeof(VertexDecal3D) * numVerts);

    // init vertex buffer
    if (!decalsVB_.Init(verts, numVerts, bDynamicVB))
    {
        LogErr(LOG, "can't init decals VB");
        return false;
    }

    // init index buffer
    if (!decalsIB_.Init(indices, numIndices, bDynamicIB))
    {
        LogErr(LOG, "can't init decals IB");
        return false;
    }

    // return great success
    return true;
}

//---------------------------------------------------------
// Desc:  push an input model into the storage
// Args:  - model:   a model which will be moved into the manager
// Ret:   identifier of added model
//---------------------------------------------------------
ModelID ModelMgr::AddModel(Model&& model)
{
    const ModelID id = model.GetId();

    // check if input model is unique
    bool isUnique = !ids_.binary_search(id);
    if (!isUnique)
    {
        PrintDump();

        LogErr(LOG, "there is already a model by ID: %" PRIu32, id);
        return INVALID_MODEL_ID;
    }

    const index idx = ids_.get_insert_idx(id);

    names_.insert_before(idx, ModelName());
    strncpy(names_[idx].name, model.GetName(), MAX_LEN_MODEL_NAME);

    ids_.insert_before(idx, model.GetId());
    models_.insert_before(idx, std::move(model));
    
    if (id >= lastModelID_)
        lastModelID_ = id + 1;

    return id;
}

//---------------------------------------------------------
// Desc:   push a new empty model into the storage
// Ret:    a ref to this new model
//---------------------------------------------------------
Model& ModelMgr::AddEmptyModel()
{
    const ModelID id = lastModelID_;
    ++lastModelID_;

    ids_.push_back(id);
    models_.push_back(Model(id));

    // add an "invalid" name
    names_.push_back(ModelName());

    // setup a name stored in the manager
    ModelName& name = names_.back();
    strcpy(name.name, "inv");

    return models_.back();
}

//---------------------------------------------------------
// Out:   array of pointers to models by input Ids
//---------------------------------------------------------
void ModelMgr::GetModelsByIds(
    const ModelID* ids,
    const size numModels,
    cvector<const Model*>& outModels)
{
    if (!ids)
    {
        LogErr(LOG, "ptr to IDs input arr == NULL");
        return;
    }
    if (numModels <= 0)
    {
        LogErr(LOG, "invalid num of models: %d", (int)numModels);
        return;
    }

    // get idxs by IDs
    ids_.get_idxs(ids, numModels, s_Idxs);

    // check idxs
#if _DEBUG | DEBUG
    for (const index idx : s_Idxs)
        assert(models_.is_valid_index(idx));
#endif

    // get pointers by idxs
    outModels.resize(numModels);

    for (int i = 0; const index idx : s_Idxs)
        outModels[i++] = &models_[idx];
}

//---------------------------------------------------------
// return a model by ID, or invalid model (by idx == 0) if there is no such ID
//---------------------------------------------------------
Model& ModelMgr::GetModelById(const ModelID id)
{
    const index idx = ids_.get_idx(id);

    if (models_.is_valid_index(idx))
        return models_[idx];

    return models_[INVALID_MODEL_ID];
}

//---------------------------------------------------------
// Desc:   get a model by input name
//---------------------------------------------------------
Model& ModelMgr::GetModelByName(const char* name)
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
    LogErr(LOG, "no model by name: %s", name);
    PrintDump();
    return models_[INVALID_MODEL_ID];
}
//---------------------------------------------------------
// Desc:   get a model ID by input name
//---------------------------------------------------------
ModelID ModelMgr::GetModelIdByName(const char* name)
{
    if (StrHelper::IsEmpty(name))
    {
        LogErr(LOG, "empty name");
        return ids_[0];                     // return empty model (actually cube)
    }

    for (index i = 0; i < names_.size(); ++i)
    {
        if (strcmp(names_[i].name, name) == 0)
            return ids_[i];
    }

    // return an empty model ID if we didn't find any
    LogErr(LOG, "no model by name: %s", name);
    return ids_[0];
}

//---------------------------------------------------------
// Desc:  check if we have any model by input name
//---------------------------------------------------------
bool ModelMgr::HasModelByName(const char* name)
{
    if (StrHelper::IsEmpty(name))
    {
        LogErr(LOG, "empty name");
        return false;
    }

    bool bHasModel = false;

    for (index i = 0; i < names_.size(); ++i)
    {
        bHasModel |= (strcmp(names_[i].name, name) == 0);
    }

    return bHasModel;
}

//---------------------------------------------------------
// Desc:   fill in the input array with names of the assets from the storage
//---------------------------------------------------------
void ModelMgr::GetModelsNamesList(cvector<ModelName>& names)
{
    names.resize(GetNumAssets());

    for (int i = 0; i < GetNumAssets(); ++i)
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
        LogErr(LOG, "empty name");
        return;
    }

    const index idx = ids_.get_idx(id);
    if (!ids_.is_valid_index(idx))
    {
        LogErr(LOG, "no model by id: %" PRIu32, id);
        return;
    }

    // clamp length of the name
    size_t len = strlen(name);

    if (len > MAX_LEN_MODEL_NAME)
        len = MAX_LEN_MODEL_NAME;

    // update model's name
    models_[idx].SetName(name);

    // update arr of names
    strncpy(names_[idx].name, name, len);
    names_[idx].name[len] = '\0';
}

//---------------------------------------------------------
// Desc:  generate and push a new decal into the decals rendering list
// 
// Args:  - center:        the collision point, or the center of the decal
//        - decalTangent:  direction for the decal
//        - normal:        normal vector of decal surface
//        - width:         width of the decal
//        - height:        height of the decal
//        - lifeTimeMs:    lifespace of this decal (if == 0, the decal won't dissapear)
//---------------------------------------------------------
void ModelMgr::AddDecal3D(
    const Vec3& center,
    const Vec3& decalTangent,
    const Vec3& normal,
    const float width,
    const float height,
    const float lifeTimeSec)
{
    // check input args
    if (numDecals_ >= MAX_NUM_DECALS)
    {
        LogErr(LOG, "decals buf overflow");
        return;
    }

    if (width <= 0 || height <= 0)
    {
        LogErr(LOG, "invalid decal dimensions: %.3f x %.3f", width, height);
        return;
    }


    Vec3 u = decalTangent;
    Vec3Normalize(u);

    Vec3 n = normal;
    Vec3Normalize(n);

    // v (no need to normalize the v vector because n and u is already normalized)
    Vec3 v = Vec3Cross(u, n);

    u *= (height * 0.5f);
    v *= (width  * 0.5f);

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
    Vec3 offset = n * 0.005f;

    // generate a decal
    Decal3D& d = decalsRendList_[numDecals_];
    numDecals_++;

    d.pos[0] = center + u + v + offset;  // UR
    d.pos[1] = center + u - v + offset;  // UL
    d.pos[2] = center - u - v + offset;  // BL
    d.pos[3] = center - u + v + offset;  // BR

    d.tex[0] = { 1, 0 };
    d.tex[1] = { 0, 0 };
    d.tex[2] = { 0, 1 };
    d.tex[3] = { 1, 1 };

    d.normal      = n;
    d.age         = lifeTimeSec;          // time until death of decal
    d.lifeTimeSec = lifeTimeSec;
}

//---------------------------------------------------------
// Desc:  update dynamic decals (its transparency)
//        for instance: bullet wallmarks, or decals after explosions
//
// Args:  - dt:  delta time - time passed since the previous frame
//---------------------------------------------------------
void ModelMgr::UpdateDynamicDecals(const float dt)
{
    if (numDecals_ == 0)
        return;

    constexpr uint numVerts = MAX_NUM_DECALS * NUM_VERTS_PER_DECAL;
    assert(numVerts <= decalsVB_.GetVertexCount());

    int v = 0;
    VertexDecal3D* verts = s_VertsDecals.data();
    Decal3D* data = (Decal3D*)decalsRendList_;
    

    // prepare data for each decal
    for (uint decalIdx = 0; decalIdx < numDecals_; decalIdx++)
    {
        const Decal3D& d = data[decalIdx];
        const Vec3* pos = d.pos;
        const Vec2* tex = d.tex;
        float translucency = 1.0f;

        if (d.lifeTimeSec > EPSILON_E3)
            translucency = d.age * (1.0f / d.lifeTimeSec);

        // vertex 0
        verts[v + 0].pos            = { pos[0].x, pos[0].y, pos[0].z };
        verts[v + 0].tex            = { tex[0].u, tex[0].v };
        verts[v + 0].normal         = { d.normal.x, d.normal.y, d.normal.z };
        verts[v + 0].translucency   = translucency;

        // vertex 1
        verts[v + 1].pos            = { pos[1].x, pos[1].y, pos[1].z };
        verts[v + 1].tex            = { tex[1].u, tex[1].v };
        verts[v + 1].normal         = { d.normal.x, d.normal.y, d.normal.z };
        verts[v + 1].translucency   = translucency;

        // vertex 2
        verts[v + 2].pos            = { pos[2].x, pos[2].y, pos[2].z };
        verts[v + 2].tex            = { tex[2].u, tex[2].v };
        verts[v + 2].normal         = { d.normal.x, d.normal.y, d.normal.z };
        verts[v + 2].translucency   = translucency;

        // vertex 3
        verts[v + 3].pos            = { pos[3].x, pos[3].y, pos[3].z };
        verts[v + 3].tex            = { tex[3].u, tex[3].v };
        verts[v + 3].normal         = { d.normal.x, d.normal.y, d.normal.z };
        verts[v + 3].translucency   = translucency;

        v += NUM_VERTS_PER_DECAL;
    }

    // update vertex buffer
    if (!decalsVB_.UpdateDynamic(verts, numVerts))
    {
        LogErr(LOG, "can't update decals VB");
        return;
    }

    // update decals lifetime
    for (uint32 i = 0; i < numDecals_; i++)
    {
        Decal3D& decal = decalsRendList_[i];
        decal.age -= dt;

        // remove dead decal
        if (decal.lifeTimeSec > 0.0f && decal.age < 0)
        {
            // swap n pop
            decal = decalsRendList_[numDecals_ - 1];
            --numDecals_;
        }
    }
}

//---------------------------------------------------------
// Desc:   print a dump of all the models (Model) in the model manager
//---------------------------------------------------------
void ModelMgr::PrintDump() const
{
    printf("\nALL MODELS DUMP: \n");

    // [idx]: (id: model_id) Name: model_name   lod1: model_id_lod_1;   lod2: model_id_lod_2
    const char* fmt = "[%u]: (id: %4" PRIu32 ") Name:%-32s  lod1: %" PRIu32 ";  lod2: %" PRIu32 "\n";

    for (index i = 0; i < models_.size(); ++i)
    {
        printf(fmt, i,
            models_[i].GetId(),
            models_[i].GetName(),
            models_[i].GetLod(LOD_1),
            models_[i].GetLod(LOD_2));
    }

    printf("\n");
}


} // namespace Core
