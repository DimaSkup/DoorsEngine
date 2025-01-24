// ********************************************************************************
// Filename:    MoveSysSerDeser.h
// Description: contains serialization/deserialization functional 
//              for the ECS MoveSystem component
// 
// Created:     11.10.24
// ********************************************************************************
#pragma once

#include "../../Common/Types.h"
#include <fstream>
#include <vector>

namespace ECS
{

class MoveSysSerDeser
{
public:
	static void Serialize(
		std::ofstream& fout,
		u32& offset,
		const u32 dataBlockMarker,
		const std::vector<EntityID>& ids,
		const std::vector<XMFLOAT4>& translationAndUniScales,
		const std::vector<XMVECTOR>& rotationQuats);

	static void Deserialize(
		std::ifstream& fin,
		const u32 offset,
		std::vector<EntityID>& ids,
		std::vector<XMFLOAT4>& translationAndUniScales,
		std::vector<XMVECTOR>& rotationQuats);
};


} // namespace ECS