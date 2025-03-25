// ********************************************************************************
// Filename:    MoveSysSerDeser.h
// Description: contains serialization/deserialization functional 
//              for the ECS MoveSystem component
// 
// Created:     11.10.24
// ********************************************************************************
#pragma once

#include "../../Common/Types.h"
#include "../../Common/cvector.h"
#include <fstream>

namespace ECS
{

class MoveSysSerDeser
{
public:
	static void Serialize(
		std::ofstream& fout,
		u32& offset,
		const u32 dataBlockMarker,
		const cvector<EntityID>& ids,
		const cvector<XMFLOAT4>& translationAndUniScales,
		const cvector<XMVECTOR>& rotationQuats);

	static void Deserialize(
		std::ifstream& fin,
		const u32 offset,
		cvector<EntityID>& ids,
		cvector<XMFLOAT4>& translationAndUniScales,
		cvector<XMVECTOR>& rotationQuats);
};


} // namespace ECS
