// **********************************************************************************
// Filename:      TexturesSystem.h
// Description:   Entity-Component-System (ECS) system for control 
//                textures data of entities
// 
// Created:       28.06.24
// **********************************************************************************
#pragma once

#include "../Common/Types.h"
#include "../Components/Textured.h"
#include <fstream>

namespace ECS
{

class TexturesSystem
{
public:
	TexturesSystem(Textured* pTextures);
	~TexturesSystem() {};

	void Serialize(std::ofstream& fout, u32& offset);
	void Deserialize(std::ifstream& fin, const u32 offset);

	void AddRecord(
		const EntityID enttID,
		const TexID* texIDs,
		const size numTextures,
		const int submeshID);

	const TexturedData& GetDataByEnttID(const EntityID enttID);

	void GetDataByEnttsIDs(
		const std::vector<EntityID>& ids,
		std::vector<TexturedData>& outData);

	void FilterEnttsWhichHaveOwnTex(
		const std::vector<EntityID>& ids,
		std::vector<EntityID>& outEnttsWithOwnTex,
		std::vector<EntityID>& outEnttsWithMeshTex);

private:
	Textured* pTexturesComponent_ = nullptr;   // a ptr to the instance of Textures component 
};

}