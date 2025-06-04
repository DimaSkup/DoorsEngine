// *********************************************************************************
// Filename:     Name.h
// Description:  an ECS component which contains names of some entities;
// 
// Created:      12.06.24
// *********************************************************************************
#pragma once

#include <Types.h>
#include <cvector.h>
#include <string>

namespace ECS
{

struct Name
{
	// both vectors have the same length because 
	// there is one to one records ['entity_id' => 'entity_name']
	cvector<EntityID> ids_;
	cvector<std::string> names_;
};

}
