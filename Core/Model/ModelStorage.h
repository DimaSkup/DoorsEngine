// ************************************************************************************
// Filename:      ModelStorage.h
// Description:   a main single storage for the models (BasicModel)
// 
// Created:       30.10.24
// ************************************************************************************
#pragma once

#include "../Common/Types.h"
#include "SkyModel.h"
#include "BasicModel.h"


#include <vector>

class ModelStorage
{
public:
	ModelStorage();

	inline static ModelStorage* Get() { return pInstance_; };

	void Serialize(ID3D11Device* pDevice);
	void Deserialize(ID3D11Device* pDevice);

	ModelID AddModel(BasicModel&& model);
	BasicModel& AddEmptyModel();

	BasicModel& GetModelByID(const ModelID id);
	BasicModel& GetModelByName(const std::string& name);

	inline BasicModel& GetLastModel()       { return models_.back(); }
	inline SkyModel&   GetSky()             { return sky_; }
	inline int         GetNumAssets() const { return (int)std::ssize(ids_); }

	void GetAssetsNamesList(std::string* namesArr, const int numNames);

#if 0
	// updating API
	void SetTextureForModelSubset(
		const ModelID modelID,
		const int subsetID,
		const TexType type,
		const TexID texID);

	void SetTexturesForModelSubset(
		const ModelID modelID,
		const int subsetID,
		std::vector<TexType>& types,
		std::vector<TexID>& texIDs);

	void SetMaterialForModelSubset(
		const ModelID modelID,
		const int subsetID,
		const MeshMaterial& material);

	void SetMaterialsForModelSubsets(
		const ModelID modelID,
		const std::vector<int>& subsetsIDs,
		const std::vector<MeshMaterial>& materials);


	void SetAABBForModelSubset(
		const ModelID modelID,
		const int subsetID,
		const DirectX::BoundingBox& aabb); 

	void SetAABBsForModelSubsets(
		const ModelID modelID,
		const std::vector<int>& subsetsIDs,
		const std::vector<DirectX::BoundingBox>& AABBs);
#endif

public:
	static int INVALID_MODEL_ID;

private:
	std::vector<ModelID>    ids_;
	std::vector<BasicModel> models_;

	SkyModel sky_;

	static ModelStorage*    pInstance_;
	static int              lastModelID_;
};