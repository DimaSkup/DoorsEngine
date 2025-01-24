// ********************************************************************************
// Filename:    TransformSysSerDeser.h
// Description: contains serialization/deserialization functional 
//              for the ECS TransformSysSerDeser component
// 
// Created:     11.10.24
// ********************************************************************************
#pragma once

#include <fstream>
#include <vector>
#include "../../Common/Types.h"

namespace ECS
{

class TransformSysSerDeser
{
public:
	static void Serialize(std::ofstream& fout,
		u32& offset,
		const u32 dataBlockMarker,
		const std::vector<EntityID>& ids,
		const std::vector<XMFLOAT4>& posAndUniScales,
		const std::vector<XMVECTOR>& dirQuats);

	static void Deserialize(
		std::ifstream& fin,
		const u32 offset,
		std::vector<EntityID>& ids,
		std::vector<XMFLOAT4>& posAndUniScales,
		std::vector<XMVECTOR>& dirQuats);
};

} // namespace ECS