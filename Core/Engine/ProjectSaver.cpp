// =================================================================================
// Filename:       ProjectSaver.cpp
// Description:    functional for storing the current project state into a file
// 
// Created:        10.12.24
// =================================================================================
#include "ProjectSaver.h"

#include "../Model/ModelStorage.h"


namespace Core
{

ProjectSaver::ProjectSaver() {}
ProjectSaver::~ProjectSaver() {}

///////////////////////////////////////////////////////////

void ProjectSaver::StoreModels(ID3D11Device* pDevice)
{
	ModelStorage::Get()->Serialize(pDevice);
}

///////////////////////////////////////////////////////////

void ProjectSaver::LoadModels(ID3D11Device* pDevice)
{
	ModelStorage::Get()->Deserialize(pDevice);
}

} // namespace Core