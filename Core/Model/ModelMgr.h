// ************************************************************************************
// Filename:      ModelMgr.h
// Description:   a main single storage for the models (BasicModel)
// 
// Created:       30.10.24
// ************************************************************************************
#pragma once

#include <CoreCommon/Types.h>
#include <CoreCommon/cvector.h>
#include "SkyModel.h"
#include "BasicModel.h"

namespace Core
{

class ModelMgr
{
public:
    ModelMgr();

    void Serialize  (ID3D11Device* pDevice);
    void Deserialize(ID3D11Device* pDevice);

    ModelID AddModel(BasicModel&& model);
    BasicModel& AddEmptyModel();

    void GetModelsByIDs(const ModelID* ids, const size numModels, cvector<const BasicModel*>& outModels);
    BasicModel& GetModelByID    (const ModelID id);
    BasicModel& GetModelByName  (const std::string& name);
    ModelID     GetModelIdByName(const std::string& name);

    //inline BasicModel& GetLastModel()       { return models_.back(); }
    inline SkyModel&   GetSky()             { return sky_; }
    inline int         GetNumAssets() const { return (int)std::ssize(ids_); }

    void GetAssetsNamesList(cvector<std::string>& names);

private:
    cvector<ModelID>    ids_;
    cvector<BasicModel> models_;

    SkyModel            sky_;

    static ModelMgr*    pInstance_;
    static ModelID      lastModelID_;
};


// =================================================================================
// Declare a global instance of the model manager
// =================================================================================
extern ModelMgr g_ModelMgr;

}
