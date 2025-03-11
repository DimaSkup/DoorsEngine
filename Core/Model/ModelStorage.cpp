// ************************************************************************************
// Filename:      ModelStorage.cpp
// Created:       30.10.24
// ************************************************************************************
#include "ModelStorage.h"

#include <CoreCommon/FileSystemPaths.h>
#include <CoreCommon/log.h>
#include <CoreCommon/Assert.h>
#include <CoreCommon/Utils.h>
#include "ModelExporter.h"
#include "ModelsCreator.h"
#include "ModelStorageSerializer.h"

#include <algorithm>
#include <fstream>
#include <filesystem>
#include <thread>
//#include <format>

namespace fs = std::filesystem;


namespace Core
{

// when we add some new model into a storage its ID == lastModelID
// and then we increase the lastModelID_ by 1, so the next model will have as ID
// this increased value, and so on
int ModelStorage::lastModelID_ = 0;

int ModelStorage::INVALID_MODEL_ID = 0;
ModelStorage* ModelStorage::pInstance_ = nullptr;



ModelStorage::ModelStorage()
{
    if (pInstance_ == nullptr)
    {
        pInstance_ = this;

        // invalid model
        //ids_.push_back(0);
        //models_.push_back(BasicModel());
    }
    else
    {
        Log::Error("there is already an instance of the ModelStorage");
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
        // if there is no such model in internal format we store it as asset
        if (!fs::exists(g_RelPathAssetsDir + relativePathsToAssets[i]))
        {
            Log::Debug("\tExport of model (thread 2nd): " + relativePathsToAssets[i]);
            exporter.ExportIntoDE3D(pDevice, models[i], relativePathsToAssets[i]);
        }
    }
}

///////////////////////////////////////////////////////////

void ModelStorage::Serialize(ID3D11Device* pDevice)
{
    // 1. write model storage's data into the file
    // 2. export model into the internal model format if it hadn't been done before

    Log::Debug("serialization: start");

    auto start = std::chrono::steady_clock::now();



    const char* pathToDataFile = "data/model_storage_data.txt";

    std::ofstream fout(pathToDataFile, std::ios::out);
    Assert::True(fout.is_open(), "can't open a file for serialization of models storage");


    ModelStorageSerializer serializer;
    const size numModels = GetNumAssets();
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

#if 0
    for (index i = 2; i < numModels; ++i)
    {
        // if there is no such model in internal format we store it as asset
        if (!fs::exists(g_RelPathAssetsDir + relativePathsToAssets[i]))
        {
            exporter.ExportIntoDE3D(pDevice, models_[i], relativePathsToAssets[i]);
        }
    }
#endif

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

#if 0
    std::thread th(
        SerializeModels,
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

   th.join();
#endif

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    Log::Print("Export duration: " + std::to_string(elapsed.count()) + "ms");

    fout.close();

    Log::Debug("serialization: finished");
}

///////////////////////////////////////////////////////////

void ModelStorage::Deserialize(ID3D11Device* pDevice)
{
    Log::Debug("deserialization: start");

    std::string ignore;
    ModelsCreator creator;
    int numModelsToLoad = 0;
    const std::string pathToDataFile = "data/model_storage_data.txt";

    std::ifstream fin(pathToDataFile, std::ios::in);
    Assert::True(fin.is_open(), "can't open a file for deserialization of models storage");



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
        ModelID loadedModelID = creator.CreateFromDE3D(pDevice, pathsToAssets[i]);
    }

    Log::Debug("deserialization: finished");
}

///////////////////////////////////////////////////////////

ModelID ModelStorage::AddModel(BasicModel&& model)
{
    // check if there is no such model id yet
    if (CoreUtils::ArrHasVal(ids_, model.id_))
    {
        Log::Error("there is already a model id: " + std::to_string(model.id_));
        Log::Error("can't add this model");
        return INVALID_MODEL_ID;
    }

    const index insertAt = CoreUtils::GetPosForVal(ids_, model.id_);

    CoreUtils::InsertAtPos(ids_, insertAt, model.id_);
    CoreUtils::InsertAtPos(models_, insertAt, std::move(model));

    return model.id_;
}

///////////////////////////////////////////////////////////

BasicModel& ModelStorage::AddEmptyModel()
{
    // push new empty model into the storage;
    // return: a ref to this new model

    ModelID id = lastModelID_;
    ++lastModelID_;

    ids_.push_back(id);
    models_.push_back(BasicModel());

    BasicModel& model = models_.back();
    model.id_ = id;

    return model;
}

///////////////////////////////////////////////////////////

BasicModel& ModelStorage::GetModelByID(const ModelID id)
{
    // check if such a model exist
    const auto beg = ids_.begin();
    const auto end = ids_.end();
    const bool exist = std::binary_search(beg, end, id);

    index idx = std::distance(beg, std::lower_bound(beg, end, id));
    idx *= exist;

    // idx > 0  -- we return a valid model
    // idx == 0 -- we return an invalid model (cube)
    return models_[idx];
}

///////////////////////////////////////////////////////////

BasicModel& ModelStorage::GetModelByName(const std::string& name)
{
    // get a model by its input name

    if (name.empty())
    {
        Log::Error("input name is empty");
        return models_[0];                  // return empty model
    }

    for (index i = 0; i < std::ssize(models_); ++i)
    {
        if (models_[i].name_ == name)
            return models_[i];
    }

    return models_[0];                  // return empty model
}

///////////////////////////////////////////////////////////

void ModelStorage::GetAssetsNamesList(std::string* namesArr, const int numNames)
{
    // fill in the input namesArr with names of the assets from the storage
    // NOTE: namesArr must be already allocated to size of numNames

    try
    {
        Assert::NotNullptr(namesArr, "ptr to the names arr == nullptr");
        Assert::True(numNames == GetNumAssets(), "input number of names is invalid: " + std::to_string(numNames));

        for (int i = 0; i < numNames; ++i)
            namesArr[i] = models_[i].GetName();
    }
    catch (EngineException& e)
    {
        Log::Error(e);
        Log::Error("can't get a list of assets names");
    }
}

} // namespace Core
