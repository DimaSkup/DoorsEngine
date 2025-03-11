// =================================================================================
// Filename:       ProjectSaver.h
// Description:    functional for storing the current project state into a file
// 
// Created:        10.12.24
// =================================================================================
#pragma once

#include <d3d11.h>

namespace Core
{

class ProjectSaver
{
public:
	ProjectSaver();
	~ProjectSaver();

	void StoreModels(ID3D11Device* pDevice);
	void LoadModels(ID3D11Device* pDevice);
};

}