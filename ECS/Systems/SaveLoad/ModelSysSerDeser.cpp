// ********************************************************************************
// Filename:    ModelSysSerDeser.cpp
// 
// Created:     11.10.24
// ********************************************************************************
#include "ModelSysSerDeser.h"
#include "../../Common/UtilsFilesystem.h"
#include "../../Common/log.h"


namespace ECS
{

#if 0

void ModelSysSerDeser::Serialize(
	std::ofstream& fout,
	u32& offset,
	const u32 dataBlockMarker,
	const std::map<EntityID, std::vector<MeshID>>& enttsToMeshes)
{
	// serialize all the data from the Mesh component into the data file

	// store offset of this data block so we will use it later for deserialization
	offset = static_cast<u32>(fout.tellp());

	const u32 dataCount = static_cast<u32>(std::ssize(enttsToMeshes));

	FileUtils::FileWrite(fout, dataBlockMarker);
	FileUtils::FileWrite(fout, dataCount);

	// if we have any entt=>meshes data we serialize it
	for (const auto& it : enttsToMeshes)
	{
		FileUtils::FileWrite(fout, it.first);                    // write entt id
		FileUtils::FileWrite(fout, (u32)std::ssize(it.second));  // write how many meshes are related to this entt
		FileUtils::FileWrite(fout, it.second);                   // write related meshes ids
	}
}

///////////////////////////////////////////////////////////

void ModelSysSerDeser::Deserialize(
	std::ifstream& fin,
	const u32 offset,
	std::map<EntityID, std::vector<MeshID>>& enttToMeshes,
	std::map<MeshID, std::vector<EntityID>>& meshToEntts)
{
	// deserialize the data from the data file into the Mesh component

	// read data starting from this offset
	fin.seekg(offset, std::ios_base::beg);

	// check if we read the proper data block
	u32 dataBlockMarker = 0;
	FileUtils::FileRead(fin, &dataBlockMarker);

	const bool isProperDataBlock = (dataBlockMarker == static_cast<u32>(ComponentType::ModelComponent));
	if (!isProperDataBlock)
	{
		Log::Error("read wrong data during deserialization of the Mesh component data");
		return;
	}

	// ------------------------------------------

	// clear the component of previous data
	enttToMeshes.clear();
	meshToEntts.clear();

	// read in how much data will we have
	u32 dataCount = 0;
	FileUtils::FileRead(fin, &dataCount);

	// read in each entity ID and its related meshes IDs
	for (u32 idx = 0; idx < dataCount; ++idx)
	{
		EntityID enttID = 0;
		u32 relatedMeshesCount = 0;

		FileUtils::FileRead(fin, &enttID);
		FileUtils::FileRead(fin, &relatedMeshesCount);

		enttToMeshes[enttID].resize(relatedMeshesCount);
		//std::vector<MeshID> meshesIDs(relatedMeshesCount);

		FileUtils::FileRead(fin, enttToMeshes[enttID]);

		// store data into component: make pair ['entity_id' => 'set_of_related_meshes_ids']
		//enttToMeshes.insert({ enttID, meshesIDs });

		// store data into component: make pair ['mesh_id' => 'set_of_related_entities_ids']
		for (const MeshID meshID : enttToMeshes[enttID])
			meshToEntts[meshID].push_back(enttID);
	}
}

#endif

}