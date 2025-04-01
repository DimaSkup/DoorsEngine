// ********************************************************************************
// Filename:     EntityMgrSerializer.cpp
// Description:  contains implementation of functional 
//               for the EntityManagerSerializer
// 
// Created:      26.06.24
// ********************************************************************************
#include "EntityMgrSerializer.h"

#include "../Common/UtilsFilesystem.h"
#include "../Common/Assert.h"
#include "SerializationHelperTypes.h"


namespace ECS
{


///////////////////////////////////////////////////////////

void EntityMgrSerializer::WriteDataHeader(
	std::ofstream& fout,
	DataHeader& header)
{
	// write into a file by dataFilepath the data header for all the data
	// related to the Entity-Component-System; so later this header will be used
	// to get a position of the necessary 
	// block of data (component data or entity manager);

	fout.seekp(0, std::ios_base::beg);

	// set data block marker for each record
	for (u32 idx = 0; idx < header.GetRecordsCount(); ++idx)
		header.records_[idx].dataBlockMarker = idx;

	// write the header into the file
	FileWrite(fout, header.recordsCount_);
	FileWrite(fout, header.records_, header.recordsCount_);
}

///////////////////////////////////////////////////////////

void EntityMgrSerializer::SerializeEnttMgrData(
	std::ofstream& fout,
	const EntityID* ids,
	const ComponentHash* hashes,
	const u32 count,
	const u32 enttMgrDataBlockMarker)
{
	// serialize own data of the entity manager:
	// all the entities IDs and related components flags (not components by itself)

	Assert::NotNullptr(ids, "a ptr to ids == nullptr");
	Assert::NotNullptr(hashes, "a ptr to hashes == nullptr");
	Assert::NotZero(count, "wrong count value");

	// write data into the data file
	FileWrite(fout, enttMgrDataBlockMarker);
	FileWrite(fout, count);

	FileWrite(fout, ids, count);
	FileWrite(fout, hashes, count);
}

}
