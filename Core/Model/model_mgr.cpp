// ************************************************************************************
// Filename:      ModelMgr.cpp
// Created:       30.10.24
// ************************************************************************************
#include <CoreCommon/pch.h>
#include "model_mgr.h"

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
    if (!InitBillboardBuffer())
    {
        LogErr(LOG, "can't init billboard VB");
        return false;
    }

    if (!InitLineVertexBuffer())
    {
        LogErr(LOG, "can't init debug lines VB");
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
        Render::g_pDevice,
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
        Render::g_pDevice,
        vertices.data(),
        maxNumVertices,
        isDynamic))
    {
        LogErr(LOG, "can't create a vertex buffer for debug lines");
        return false;
    }

    if (!debugLinesIB_.Initialize(
        Render::g_pDevice,
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


} // namespace Core
