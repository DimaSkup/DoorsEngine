// *********************************************************************************
// Filename:      MeshStorage.cpp
// Description:   implementation of the MeshStorage functional;
//
// Created:       16.05.24
// *********************************************************************************

#if 0
void MeshStorage::GetMeshesDataForRendering(
	const std::vector<MeshID>& meshesIDs,
	ModelDataForRender& outData)
{
	// go thought each mesh from the input arr and get its data which will
	// be used for rendering

	const size meshesCount = std::ssize(meshesIDs);
	std::vector<index> idxs;

	outData.Resize(meshesCount);
	idxs.resize(meshesCount);


	try
	{
		// get data idx of each input mesh
		for (int i = 0; const MeshID& id : meshesIDs)
			idxs[i++] = meshIdToDataIdx_.at(id);

		for (int i = 0; const index idx : idxs)
			outData.names_[i++] = names_[idx];

		for (int i = 0; const index idx : idxs)
			outData.pVBs_[i++] = vertexBuffers_[idx].Get();

		for (int i = 0; const index idx : idxs)
			outData.pIBs_[i++] = indexBuffers_[idx].Get();
	
		for (int i = 0; const index idx : idxs)
			outData.indexCount_[i++] = indexBuffers_[idx].GetIndexCount();

		// get arr of textures shader resource views for each mesh
		for (int i = 0; const index idx : idxs)
			outData.texIDs_[i++] = textures_[idx];

		// get AABB of each mesh
		for (int i = 0; const index idx : idxs)
			outData.boundBoxes_[i++] = aabb_[idx];
		
		for (int i = 0; const index idx : idxs)
			outData.materials_[i++] = materials_[idx];
	}
	catch (const std::out_of_range& e)
	{
		Log::Error(e.what());
		throw EngineException("something went out of range");
	}
}
#endif