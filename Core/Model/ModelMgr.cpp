// ************************************************************************************
// Filename:      ModelMgr.cpp
// Created:       30.10.24
// ************************************************************************************
#include <CoreCommon/pch.h>
#include "ModelMgr.h"

#include "ModelExporter.h"
#include "ModelsCreator.h"
#include "ModelStorageSerializer.h"

namespace fs = std::filesystem;


namespace Core
{

// init a global instance of the model manager
ModelMgr g_ModelMgr;

// a static pointer to the instance of the model manager
ModelMgr* ModelMgr::pInstance_ = nullptr;

// when we add some new model into a storage its ID == lastModelID
// and then we increase the lastModelID_ by 1, so the next model will have as ID
// this increased value, and so on
ModelID ModelMgr::lastModelID_ = 0;


///////////////////////////////////////////////////////////

ModelMgr::ModelMgr()
{
    if (pInstance_ == nullptr)
    {
        pInstance_ = this;
    }
    else
    {
        LogErr("there is already an instance of the ModelMgr");
        return;
    }
}

///////////////////////////////////////////////////////////

void SerializeModels(
    ID3D11Device* pDevice,
    const BasicModel* models,
    const std::string* relativePathsToAssets,
    const size numModels,
    const index startIdx,
    const index endIdx)
{
    ModelExporter exporter;

    for (index i = startIdx; i < endIdx; ++i)
    {
        // generate full path to the model in .de3d format
        sprintf(g_String, "%s%s", g_RelPathAssetsDir, relativePathsToAssets[i].c_str());

        // if there is no such model in internal format we store it as asset
        if (fopen(g_String, "r+") == nullptr)
        {
            exporter.ExportIntoDE3D(pDevice, models[i], relativePathsToAssets[i].c_str());
        }
    }
}

///////////////////////////////////////////////////////////

bool ModelMgr::InitBillboardBuffer()
{
    // initialize a billboards buffer 
    constexpr int  maxNumBillboards = 30000;
    constexpr bool isDynamic = true;

    cvector<BillboardSprite> vertices(maxNumBillboards);

    if (!billboardsVB_.Initialize(
        Render::g_pDevice,
        vertices.data(),
        maxNumBillboards, isDynamic))
    {
        LogErr(LOG, "can't create a vertex buffer for billboard sprites");
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////

void ModelMgr::Serialize(ID3D11Device* pDevice)
{
    // 1. write model storage's data into the file
    // 2. export model into the internal model format if it hadn't been done before

    LogDbg(LOG, "serialization: start");

    auto start = std::chrono::steady_clock::now();


    const char* pathToDataFile = "data/model_storage_data.txt";

    std::ofstream fout(pathToDataFile, std::ios::out);
    CAssert::True(fout.is_open(), "can't open a file for serialization of models storage");


    ModelStorageSerializer serializer;
    const size numModels = GetNumAssets() - 2;
    std::vector<std::string> relativePathsToAssets(numModels);

    serializer.WriteHeader(fout, numModels, lastModelID_);

    // generate relative paths based on models names
    for (index i = 2; i < numModels; ++i)
    {
        const std::string& name = models_[i].name_;
        relativePathsToAssets[i] = std::string(name + "/" + name + ".de3d");
    }

    serializer.WriteModelsInfo(fout, ids_.data(), relativePathsToAssets.data(), numModels);


    // export assets from memory into the internal .de3d format
    ModelExporter exporter;

    size numModelsToExport = (numModels - 2);
    const index firstHalfRangeStart = 2;
    const index firstHalfRangeEnd = numModels / 2;
    const index secondHalfRangeStart = numModels / 2;
    const index secondHalfRangeEnd = numModelsToExport + 2;

    SerializeModels(
        pDevice,
        models_.data(),
        relativePathsToAssets.data(),
        models_.size(),
        firstHalfRangeStart,
        firstHalfRangeEnd);

    SerializeModels(
        pDevice,
        models_.data(),
        relativePathsToAssets.data(),
        models_.size(),
        secondHalfRangeStart,
        secondHalfRangeEnd);


    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;

    fout.close();
    LogMsg(LOG, "Model mgr serialization duration: %f sec", elapsed.count());
    LogDbg(LOG, "serialization: finished");
}

///////////////////////////////////////////////////////////

void ModelMgr::Deserialize(ID3D11Device* pDevice)
{
    LogDbg(LOG, "deserialization: start");

    std::string ignore;
    ModelsCreator creator;
    int numModelsToLoad = 0;
    const std::string pathToDataFile = "data/model_storage_data.txt";

    std::ifstream fin(pathToDataFile, std::ios::in);
    CAssert::True(fin.is_open(), "can't open a file for deserialization of models storage");

    // skip header
    fin >> ignore;

    fin >> ignore >> lastModelID_;
    fin >> ignore >> numModelsToLoad;
    fin >> ignore;

    // prepare memory
    size curSize = std::ssize(ids_);
    ids_.reserve(curSize + numModelsToLoad);
    models_.reserve(curSize + numModelsToLoad);

    std::vector<ModelID>     modelsIDs(numModelsToLoad, INVALID_MODEL_ID);
    std::vector<std::string> pathsToAssets(numModelsToLoad, "invalid");

    // read in all pairs [id => path] from the file
    for (int i = 0; i < numModelsToLoad; ++i)
    {
        fin >> modelsIDs[i] >> pathsToAssets[i];
    }

    for (int i = 0; i < numModelsToLoad; ++i)
    {
        // load a model from the internal format
        ModelID loadedModelID = creator.CreateFromDE3D(pDevice, pathsToAssets[i].c_str());

        if (modelsIDs[i] != loadedModelID)
        {
            sprintf(g_String, "ID (%ud) of loaded model is not equal to the expected (%ud) one", loadedModelID, modelsIDs[i]);
            LogErr(g_String);
        }
    }

    LogDbg(LOG, "deserialization: finished");
}

//---------------------------------------------------------
// Desc:  push an input model into the storage
// Args:  - model:   a model which will be moved into the manager
// Ret:   identifier of added model
//---------------------------------------------------------
ModelID ModelMgr::AddModel(BasicModel&& model)
{
    // check if there is no such model id yet
    if (ids_.binary_search(model.id_))
    {
        LogErr(LOG, "can't add model: there is already a model by ID: %ud", model.id_);
        return INVALID_MODEL_ID;
    }

    const ModelID id = model.id_;
    const index idx = ids_.get_insert_idx(model.id_);

    ids_.insert_before(idx, id);
    models_.insert_before(idx, std::move(model));

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

    BasicModel& model = models_.back();
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
    CAssert::True(ids != nullptr, "input ptr to models IDs arr == nullptr");
    CAssert::True(numModels > 0,  "input number of models must be > 0");

    // get idxs by IDs
    cvector<index> idxs;
    ids_.get_idxs(ids, numModels, idxs);

    // get pointers by idxs
    outModels.resize(numModels);

    for (int i = 0; const index idx : idxs)
        outModels[i++] = &models_[idx];
}

//---------------------------------------------------------
// return a model by ID, or invalid model (by idx == 0) if there is no such ID
//---------------------------------------------------------
BasicModel& ModelMgr::GetModelById(const ModelID id)
{
    const index idx = ids_.get_idx(id);
    return models_[idx * (ids_[idx] == id)];
}

//---------------------------------------------------------
// Desc:   get a model by input name
//---------------------------------------------------------
BasicModel& ModelMgr::GetModelByName(const char* name)
{
    if ((name == nullptr) || (name[0] == '\0'))
    {
        LogErr("input name is empty");
        return models_[0];                  // return empty model (actually cube)
    }

    for (index i = 0; i < std::ssize(models_); ++i)
    {
        if (strcmp(models_[i].name_, name) == 0)
            return models_[i];
    }

    // return an empty model if we didn't find any
    LogErr(LOG, "there is no model by name: %s", name);
    return models_[0];                  
}
//---------------------------------------------------------
// Desc:   get a model ID by input name
//---------------------------------------------------------
ModelID ModelMgr::GetModelIdByName(const char* name)
{
    if ((name == nullptr) || (name[0] == '\0'))
    {
        LogErr("input name is empty");
        return ids_[0];                     // return empty model (actually cube)
    }

    for (vsize i = 0; i < models_.size(); ++i)
    {
        if (strcmp(models_[i].name_, name) == 0)
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

} // namespace Core
