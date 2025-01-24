// ********************************************************************************
// Filename:     EntityMgrSerializer.h
// Description:  contains functional for serialization of the EntityMgr data
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

class EntityMgrSerializer
{
public:
	EntityMgrSerializer() {}

	void WriteDataHeader(
		std::ofstream& fout,
		DataHeader& header);

	void SerializeEnttMgrData(
		std::ofstream& fout,
		const EntityID* ids,
		const ComponentsHash* hashes,
		const u32 count,
		const u32 enttMgrDataBlockMarker);
};

};