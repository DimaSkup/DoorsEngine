// ********************************************************************************
// Filename:    MoveSysSerDeser.cpp
// 
// Created:     11.10.24
// ********************************************************************************
#include "MoveSysSerDeser.h"
#include "../../Common/UtilsFilesystem.h"
#include "../../Common/log.h"


namespace ECS
{

void MoveSysSerDeser::Serialize(
	std::ofstream& fout,
	u32& offset,
	const u32 dataBlockMarker,
	const cvector<EntityID>& ids,
	const cvector<XMFLOAT4>& translationAndUniScales,
	const cvector<XMVECTOR>& rotationQuats)
{
	// serialize all the data from the Movement component into the data file

	// store offset of this data block so we will use it later for deserialization
	offset = static_cast<u32>(fout.tellp());

	const u32 dataCount = static_cast<u32>(std::ssize(ids));

	// write movement data into the file
	FileWrite(fout, &dataBlockMarker);
	FileWrite(fout, &dataCount);

	FileWrite(fout, ids);
	FileWrite(fout, translationAndUniScales);
	FileWrite(fout, rotationQuats);
}

///////////////////////////////////////////////////////////

void MoveSysSerDeser::Deserialize(
	std::ifstream& fin, 
	const u32 offset,
	cvector<EntityID>& ids,
	cvector<XMFLOAT4>& translationAndUniScales,
	cvector<XMVECTOR>& rotationQuats)
{
	// deserialize the data from the data file into the Movement component

	// read data starting from this offset
	fin.seekg(offset, std::ios_base::beg);

	// check if we read the proper data block
	u32 dataBlockMarker = 0;
	FileRead(fin, &dataBlockMarker);

	const bool isProperDataBlock = (dataBlockMarker == static_cast<u32>(eComponentType::MoveComponent));
	if (!isProperDataBlock)
	{
		LogErr("read wrong data during deserialization of the Move component data");
		return;
	}

	// ------------------------------------------

	// read in how much data will we have
	u32 dataCount = 0;
	FileRead(fin, &dataCount);

	// prepare enough amount of memory for data
	ids.resize(dataCount);
	translationAndUniScales.resize(dataCount);
	rotationQuats.resize(dataCount);

	// read data from a file right into the component
	FileRead(fin, ids);
	FileRead(fin, translationAndUniScales);
	FileRead(fin, rotationQuats);
}


} // namespace ECS
