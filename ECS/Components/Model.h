// *********************************************************************************
// Filename:     Model.h
// Description:  an ECS component which constains relations 
//               between entities and models
// 
// Created:      16.05.24
// *********************************************************************************
#pragma once

#include <Types.h>
#include <cvector.h>

namespace ECS
{

struct Model
{
    cvector<EntityID> enttsIDs_;   // primary keys (can have only unique values)
    cvector<ModelID>  modelIDs_;   // there can be multiple the same values
};

}
