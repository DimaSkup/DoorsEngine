// ********************************************************************************
// Filename:     EntityMgrDeserializer.h
// Description:  contains functional for deserialization of the EntityMgr data
//               as well as the components
// 
// Created:      26.06.24
// ********************************************************************************
#pragma once

#include "../Common/Types.h"
#include "SerializationHelperTypes.h"
#include <fstream>


namespace ECS
{

class EntityMgrDeserializer
{
public:
	EntityMgrDeserializer() {}

	void ReadDataHeader(
		std::ifstream& fin,
		DataHeader& header);

	void DeserializeEnttMgrData(
		std::ifstream& fin,
		EntityID** ids,
		ComponentHash** hashes,
		u32& count,
		const u32 enttMgrDataBlockMarker);
};

}