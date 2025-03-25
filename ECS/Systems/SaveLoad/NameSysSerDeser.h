// ********************************************************************************
// Filename:    NameSysSerDeser.h
// Description: contains serialization/deserialization functional 
//              for the ECS NameSystem component
// 
// Created:     11.10.24
// ********************************************************************************
#pragma once

#include "../../Common/Types.h"
#include "../../Common/cvector.h"
#include <fstream>


namespace ECS
{


class NameSysSerDeser
{
public:

#if 0
	// FOR FUTURE: to make serialization/deserialization without using of cvector
	struct DataToSerialize
	{
		u32 dataBlockMarker = 0;
		u32 dataCount = 0;      // the number of entts/names

		EntityID* pEnttsIds = nullptr;

		EntityName** ppNames = nullptr;
		size* namesLength = nullptr;
	};
#endif

	static void Serialize(
		std::ofstream& fout,
		u32& offset,
		const u32 dataBlockMarker,
		const cvector<EntityID>& ids,
		const cvector<EntityName>& names);

	static void Deserialize(
		std::ifstream& fin,
		const u32 offset,
		cvector<EntityID>& outIds,
		cvector<EntityName>& outNames);
};


} // namespace ECS
