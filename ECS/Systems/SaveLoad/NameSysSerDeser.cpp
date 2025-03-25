// ********************************************************************************
// Filename:    NameSysSerDeser.cpp
//
// Created:     11.10.24
// ********************************************************************************
#include "NameSysSerDeser.h"

#include "../../Common/UtilsFilesystem.h"
#include "../../Common/log.h"

namespace ECS
{

void NameSysSerDeser::Serialize(
	std::ofstream& fout, 
	u32& offset,
	const u32 dataBlockMarker,
	const cvector<EntityID>& ids,
	const cvector<EntityName>& names)
{
	// serialize all the data from the Name component into the data file

	// store offset of this data block so we will use it later for deserialization
	offset = static_cast<u32>(fout.tellp());

	// write the data block marker, data count, and the IDs values
	FileUtils::FileWrite(fout, &dataBlockMarker);
	FileUtils::FileWrite(fout, (u32)std::ssize(ids)); 
	FileUtils::FileWrite(fout, ids);

	for (const EntityName& name : names)
	{
		// go through each name and:
		// 1. write how many name's characters will we write into the file so 
		//    later we will be able to read proper string;
		// 2. write name

		u32 strSize = (u32)name.size();
		FileUtils::FileWrite(fout, strSize);
		FileUtils::FileWrite(fout, name.data(), strSize);
	}
}

///////////////////////////////////////////////////////////

void NameSysSerDeser::Deserialize(
	std::ifstream& fin,
	const u32 offset,
	cvector<EntityID>& outIds,
	cvector<EntityName>& outNames)
{
	// deserialize all the data from the data file into the Name component

	// read Name data starting from this offset
	fin.seekg(offset, std::ios_base::beg);

	// check if we read the proper data block
	u32 dataBlockMarker = 0;
	FileUtils::FileRead(fin, &dataBlockMarker);
	
	const bool isProperDataBlock = (dataBlockMarker == static_cast<u32>(ComponentType::NameComponent));
	if (!isProperDataBlock)
	{
		Log::Error("read wrong data during deserialization of the Name component data");
		return;
	}
	
	// ------------------------------------------

	// get how many data elements we will have
	u32 dataCount = 0;
	FileUtils::FileRead(fin, &dataCount);

	// prepare enough amount of memory for data
	outIds.resize(dataCount);
	outNames.resize(dataCount);

	// read in entities ids
	FileUtils::FileRead(fin, outIds);

	// read in entities names
	for (u32 idx = 0, strSize = 0; idx < dataCount; ++idx)
	{
		FileUtils::FileRead(fin, &strSize);                        // read in chars count
		outNames[idx].resize(strSize);                         // prepare memory for a string
		FileUtils::FileRead(fin, outNames[idx].data(), strSize);   // read in a string
	}
}

///////////////////////////////////////////////////////////

}  // namespace ECS
