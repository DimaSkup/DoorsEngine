// *********************************************************************************
// Filename:     Model.h
// Description:  an ECS component which constains relations 
//               between entities and models
// 
// Created:      16.05.24
// *********************************************************************************
#pragma once

#include "../Common/Types.h"
#include "../Common/cvector.h"
//#include <map>
//#include <set>

namespace ECS
{

struct Model
{
#if 1
    cvector<EntityID> enttsIDs_;   // primary keys (can have only unique values)
    cvector<ModelID>  modelIDs_;   // there can be multiple the same values

#else
	// each entity can be related to only one model
	std::map<EntityID, ModelID> enttToModel_; 

	// each model can be related to multiple entities
	std::map<ModelID, std::set<EntityID>> modelToEntt_;
#endif

    ComponentType type_ = ComponentType::ModelComponent;
};

}
