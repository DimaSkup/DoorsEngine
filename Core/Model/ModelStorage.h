// ************************************************************************************
// Filename:      ModelStorage.h
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

class ModelStorage
{
public:
    ModelStorage();

    inline static ModelStorage* Get() { return pInstance_; };

    void Serialize  (ID3D11Device* pDevice);
    void Deserialize(ID3D11Device* pDevice);

    ModelID AddModel(BasicModel&& model);
    BasicModel& AddEmptyModel();

    BasicModel& GetModelByID    (const ModelID id);
    BasicModel& GetModelByName  (const std::string& name);
    ModelID     GetModelIdByName(const std::string& name);

    //inline BasicModel& GetLastModel()       { return models_.back(); }
    inline SkyModel&   GetSky()             { return sky_; }
    inline int         GetNumAssets() const { return (int)std::ssize(ids_); }

    void GetAssetsNamesList(cvector<std::string>& names);

private:
    cvector<ModelID>        ids_;
    cvector<BasicModel>     models_;

    SkyModel                sky_;

    static ModelStorage*    pInstance_;
    static ModelID          lastModelID_;
};

}
