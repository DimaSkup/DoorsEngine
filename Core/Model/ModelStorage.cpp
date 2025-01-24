// ************************************************************************************
// Filename:      ModelStorage.cpp
// Created:       30.10.24
// ************************************************************************************
#include "ModelStorage.h"

#include "../Common/FileSystemPaths.h"
#include "../Common/log.h"
#include "../Common/Utils.h"
#include "../Model/ModelExporter.h"
#include "../Model/ModelsCreator.h"


#include <algorithm>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;


// when we add some new model into a storage its ID == lastModelID
// and then we increase the lastModelID_ by 1, so the next model will have as ID
// this increased value, and so on
int ModelStorage::lastModelID_ = 1;

int ModelStorage::INVALID_MODEL_ID = 0;
ModelStorage* ModelStorage::pInstance_ = nullptr;



ModelStorage::ModelStorage()
{
	if (pInstance_ == nullptr)
	{
		pInstance_ = this;

		// invalid model
		ids_.push_back(0);
		models_.push_back(BasicModel());
	}
	else
	{
		Log::Error("there is already an instance of the ModelStorage");
		return;
	}
}

///////////////////////////////////////////////////////////

void ModelStorage::Serialize(ID3D11Device* pDevice)
{
	// 1. write model storage's data into the file
	// 2. export model into the internal model format if it hadn't been done before
	return;
	Log::Debug("serialization: start");

	ModelExporter exporter;
	const std::string pathToDataFile = g_DataDir + "model_storage_data.txt";

	std::ofstream fout(pathToDataFile, std::ios::out);
	Assert::True(fout.is_open(), "can't open a file for serialization of models storage");


	// -1 because model_ID == 0 is for the invalid model so we skip it
	const size numModels = std::ssize(ids_) - 1;   

	fout << "*****************Model_Storage_Data*****************\n";
	fout << "#LastID: " << lastModelID_ << '\n';
	fout << "#ModelsCount: " << numModels << "\n\n";
	fout << "#ModelsList(model_id,imported_model_path):\n";
	
	for (index i = 1; i < std::ssize(ids_); ++i)
	{
		// check if such a model is already stored in the internal model format 
		const std::string relativePathToInternal = models_[i].name_ + "/" + models_[i].name_ + ".de3d";
		const fs::path pathToInternal = g_ImportedModelsDirPath + relativePathToInternal;
		
		// if we need to store as internal
		if (!fs::exists(pathToInternal))
			exporter.ExportIntoDE3D(pDevice, models_[i], relativePathToInternal);

		// write info about this model into the data file
		fout << ids_[i] << ' ' << relativePathToInternal << '\n';
	}

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
	const std::string pathToDataFile = g_DataDir + "model_storage_data.txt";

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

	
	for (int i = 0; i < numModelsToLoad; ++i)
	{
		ModelID id;
		std::string path;

		fin >> id >> path;

		// load a model from the internal format
		ModelID loadedModelID = creator.CreateFromDE3D(pDevice, g_ImportedModelsDirPath + path);

		// check if we loaded the proper model
		Assert::True(id == loadedModelID, std::format("loaded wrong model (expected_id: {}, loaded_id: {}", id, loadedModelID));
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

	bool exist = std::binary_search(beg, end, id);

	// find an index of this ID
	index idx = std::distance(beg, std::upper_bound(beg, end, id)) - 1;
	idx *= exist;

	if (idx == 0)
	{
		// return an empty model
		Log::Error("attempt to access to an empty model");
		return models_[INVALID_MODEL_ID];
	}

	// if such model exist we return it
	return models_[idx];
}

///////////////////////////////////////////////////////////

BasicModel& ModelStorage::GetModelByName(const std::string& name)
{
	// get a model by its input name

	for (index i = 0; i < std::ssize(models_); ++i)
	{
		if (models_[i].name_ == name)
			return models_[i];
	}

	return models_[0];   // return empty model
}


// *****************************************************************************
// 
//                           PUBLIC UPDATING API
// 
// *****************************************************************************

void ModelStorage::SetTextureForModelSubset(
	const ModelID modelID,
	const int subsetID,
	const TexType type,
	const TexID texID)
{
	// set a texture for particular subset (mesh) of the model
	BasicModel& model = GetModelByID(modelID);
	model.SetTexture(subsetID, type, texID);
}

///////////////////////////////////////////////////////////

void ModelStorage::SetTexturesForModelSubset(
	const ModelID modelID,
	const int subsetID,
	std::vector<TexType>& types,
	std::vector<TexID>& texIDs)
{
	
	Assert::True(types.size() == texIDs.size(), "input data arrs must be equal");

	// setup multiple textures for particular subset (mesh) of the model
	BasicModel& model = GetModelByID(modelID);
	const int elemsCount = static_cast<int>(texIDs.size());

	model.SetTextures(subsetID, types.data(), texIDs.data(), elemsCount);
}

///////////////////////////////////////////////////////////

void ModelStorage::SetMaterialForModelSubset(
	const ModelID modelID,
	const int subsetID,
	const MeshMaterial& material)
{
	// set a material for particular subset (mesh) of the model
	BasicModel& model = GetModelByID(modelID);
	model.SetMaterialForSubset(subsetID, material);
}

///////////////////////////////////////////////////////////

void ModelStorage::SetMaterialsForModelSubsets(
	const ModelID modelID,
	const std::vector<int>& subsetsIDs,
	const std::vector<MeshMaterial>& materials)
{
	Assert::True(subsetsIDs.size() == materials.size(), "input data arrs must be equal");

	// setup material for each input subset (mesh) of the model
	BasicModel& model = GetModelByID(modelID);
	const int elemsCount = static_cast<int>(materials.size());

	model.SetMaterialsForSubsets(subsetsIDs.data(), materials.data(), elemsCount);
}

///////////////////////////////////////////////////////////

void ModelStorage::SetAABBForModelSubset(
	const ModelID modelID,
	const int subsetID,
	const DirectX::BoundingBox& aabb)
{
	// set a AABB for particular subset (mesh) of the model
	BasicModel& model = GetModelByID(modelID);
	model.SetSubsetAABB(subsetID, aabb);
}

///////////////////////////////////////////////////////////

void ModelStorage::SetAABBsForModelSubsets(
	const ModelID modelID,
	const std::vector<int>& subsetsIDs,
	const std::vector<DirectX::BoundingBox>& AABBs)
{
	Assert::True(subsetsIDs.size() == AABBs.size(), "input data arrs must be equal");

	// setup bounding box for each input subset (mesh) of the model
	BasicModel& model = GetModelByID(modelID);
	const int elemsCount = static_cast<int>(AABBs.size());

	model.SetSubsetAABBs(subsetsIDs.data(), AABBs.data(), elemsCount);
}

///////////////////////////////////////////////////////////