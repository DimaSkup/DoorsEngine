// ********************************************************************************
// Filename:    TransformSysSerDeser.cpp
// 
// Created:     11.10.24
// ********************************************************************************
#include "TransformSysSerDeser.h"
#include "../../Common/UtilsFilesystem.h"
#include "../../Common/log.h"

namespace ECS
{

void TransformSysSerDeser::Serialize(
	std::ofstream& fout, 
	u32& offset,
	const u32 dataBlockMarker,
	const cvector<EntityID>& ids,
	const cvector<XMFLOAT4>& posAndUniScales,
	const cvector<XMVECTOR>& dirQuats)
{
	// serialize all the data from the Transform component into the data file

	// store offset of this data block so we will use it later for deserialization
	offset = static_cast<u32>(fout.tellp());

	const size dataCount = ids.size();

	FileWrite(fout, &dataBlockMarker);
	FileWrite(fout, &dataCount);

	FileWrite(fout, ids);
	FileWrite(fout, posAndUniScales);
	FileWrite(fout, dirQuats);
}

///////////////////////////////////////////////////////////

void TransformSysSerDeser::Deserialize(
	std::ifstream& fin, 
	const u32 offset,
	cvector<EntityID>& ids,
	cvector<XMFLOAT4>& posAndUniScales,
	cvector<XMVECTOR>& dirQuats)
{
	// deserialize all the data from the data file into the Transform component

	// read data starting from this offset
	fin.seekg(offset, std::ios_base::beg);

	// check if we read the proper data block
	u32 dataBlockMarker = 0;
	FileRead(fin, &dataBlockMarker);

	const bool isProperDataBlock = (dataBlockMarker == static_cast<u32>(eComponentType::TransformComponent));
	if (!isProperDataBlock)
	{
		Log::Error("read wrong data during deserialization of the Transform component data");
		return;
	}

	// ------------------------------------------

	size dataCount = 0;
	FileRead(fin, &dataCount);

	// prepare enough amount of memory for data
	ids.resize(dataCount);
	posAndUniScales.resize(dataCount);
	dirQuats.resize(dataCount);

	// read data from a file right into the component
	FileRead(fin, ids);
	FileRead(fin, posAndUniScales);
	FileRead(fin, dirQuats);
}


} // namespace ECS
