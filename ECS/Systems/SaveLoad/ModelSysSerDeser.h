// ********************************************************************************
// Filename:    ModelSysSerDeser.h
// Description: contains serialization/deserialization functional 
//              for the ECS ModelSystem component
// 
// Created:     11.10.24
// ********************************************************************************
#pragma once

#include "../../Common/Types.h"
#include <vector>
#include <map>
#include <fstream>

namespace ECS
{

class ModelSysSerDeser
{
public:
#if 0
	static void Serialize(
		std::ofstream& fout,
		u32& offset,
		const u32 dataBlockMarker,
		const std::map<EntityID, std::vector<MeshID>>& enttsToMeshes);

	static void Deserialize(
		std::ifstream& fin,
		const u32 offset,
		std::map<EntityID, std::vector<MeshID>>& enttToMeshes,
		std::map<MeshID, std::vector<EntityID>>& meshToEntts);
#endif
};


} // namespace ECS
