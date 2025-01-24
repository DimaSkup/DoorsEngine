// ********************************************************************************
// Filename:     EntityMgrDeserializer.cpp
// Description:  contains implementation of functional 
//               for the EntityManagerDeserializer
// 
// Created:      26.06.24
// ********************************************************************************
#include "EntityMgrDeserializer.h"

#include "../Common/UtilsFilesystem.h"
#include "../Common/Assert.h"

namespace ECS
{


///////////////////////////////////////////////////////////

void EntityMgrDeserializer::ReadDataHeader(
	std::ifstream& fin,
	DataHeader& header)
{
	// read in the data header from the data file;
	// so later we'll use this header to navigate through the file to
	// deserialize EntityMgr and Components data;

	for (int i = 0; i < (int)header.recordsCount_; ++i)
	{
		DataHeaderRecord& record = header.records_[i];
		Utils::FileRead(fin, &(record.dataBlockMarker));
		Utils::FileRead(fin, &(record.dataBlockPos));
	}
}

///////////////////////////////////////////////////////////

void EntityMgrDeserializer::DeserializeEnttMgrData(
	std::ifstream& fin,
	EntityID** ids,
	ComponentsHash** hashes,
	u32& count,
	const u32 enttMgrDataBlockMarker)
{
	// deserialize data for the entity manager: all the entities IDs 
	// and related components flags (not components data itself or something else)

	u32 dataBlockMarker = 0;
	Utils::FileRead(fin, &dataBlockMarker);

	// if we read wrong data block
	if (dataBlockMarker != enttMgrDataBlockMarker)
	{
		std::string errMsg;
		errMsg += "ECS deserialization: read wrong block of data: ";
		errMsg += "read (" + std::to_string(dataBlockMarker) + ") but must be (";
		errMsg += std::to_string(enttMgrDataBlockMarker) + ")";
		
		throw LIB_Exception(errMsg);
	}

	// --------------------------------

	// read in how many data elems we have
	Utils::FileRead(fin, &count);

	// prepare enough amount of memory for data
	*ids = new EntityID[count]{ 0 };
	*hashes = new ComponentsHash[count]{ 0 };

	Utils::FileRead(fin, ids, count);
	Utils::FileRead(fin, hashes, count);
}

///////////////////////////////////////////////////////////

}