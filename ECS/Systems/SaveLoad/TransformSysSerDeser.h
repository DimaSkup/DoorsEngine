// ********************************************************************************
// Filename:    TransformSysSerDeser.h
// Description: contains serialization/deserialization functional 
//              for the ECS TransformSysSerDeser component
// 
// Created:     11.10.24
// ********************************************************************************
#pragma once

#include "../../Common/Types.h"
#include "../../Common/cvector.h"
#include <fstream>


namespace ECS
{

class TransformSysSerDeser
{
public:
	static void Serialize(
        std::ofstream& fout,
		u32& offset,
		const u32 dataBlockMarker,
		const cvector<EntityID>& ids,
		const cvector<XMFLOAT4>& posAndUniScales,
		const cvector<XMVECTOR>& dirQuats);

	static void Deserialize(
		std::ifstream& fin,
		const u32 offset,
		cvector<EntityID>& ids,
		cvector<XMFLOAT4>& posAndUniScales,
		cvector<XMVECTOR>& dirQuats);
};

} // namespace ECS
