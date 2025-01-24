// **********************************************************************************
// Filename:      TexturesSystem.cpp
// Description:   implementation of the TexturesSystem's functional
// 
// Created:       28.06.24
// **********************************************************************************
#include "TexturesSystem.h"

#include "../Common/Assert.h"
#include "../Common/Utils.h"

#include "Helpers/TexturesSysAddHelpers.h"

namespace ECS
{


TexturesSystem::TexturesSystem(Textured* pTextures) : pTexturesComponent_(pTextures)
{
	Assert::NotNullptr(pTextures, "input ptr to the Textures component == nullptr");

	// setup default (invalid) textures set
	const u32 texTypesCount = 22;
	const TexID texIDs[texTypesCount]{ INVALID_TEXTURE_ID };
	const int submeshID = 0;

	Textured& comp = *pTexturesComponent_;
	comp.ids_.push_back(INVALID_ENTITY_ID);
	comp.data_.push_back(TexturedData(texIDs, submeshID));
}

///////////////////////////////////////////////////////////

void TexturesSystem::Serialize(std::ofstream& fout, u32& offset)
{
	Assert::True(false, "TODO: implement it!");
}

///////////////////////////////////////////////////////////

void TexturesSystem::Deserialize(std::ifstream& fin, const u32 offset)
{
	Assert::True(false, "TODO: implement it!");
}

///////////////////////////////////////////////////////////

void TexturesSystem::AddRecord(
	const EntityID enttID,
	const TexID* texIDs,
	const size numTextures,
	const int submeshID)            
{
	// add own textures set to each input entity;
	// in: submeshID: to which mesh of this particular entt we set new textures

	Assert::NotNullptr(texIDs, "a ptr to textures arr == nullptr");
	Assert::True(numTextures == 22, "wrong numTextures (must be == 22)");
	Assert::True(submeshID >= 0, "wrong submesh id (must be >= 0)");

	Textured& comp = *pTexturesComponent_;

	// if there is already a record with such entt ID
	if (Utils::BinarySearch(comp.ids_, enttID))
		return;

	// add a record (here we execute sorted insertion into the data arrays)
	const index insertAt = Utils::GetPosForID(comp.ids_, enttID);

	Utils::InsertAtPos(comp.ids_, insertAt, enttID);
	Utils::InsertAtPos(comp.data_, insertAt, TexturedData(texIDs, submeshID));
}

///////////////////////////////////////////////////////////

const TexturedData& TexturesSystem::GetDataByEnttID(
	const EntityID enttID)
{
	// get Textured component data for entity by enttID

	const Textured& comp = *pTexturesComponent_;
	const bool exist = Utils::BinarySearch(comp.ids_, enttID);

	if (exist)
	{
		const index idx = exist * Utils::GetIdxInSortedArr(comp.ids_, enttID);
		return comp.data_[idx];
	}
	else
	{
		// input entt doesn't have this component so just return default invalid data
		return comp.data_[0];
	}
}

///////////////////////////////////////////////////////////

void TexturesSystem::GetDataByEnttsIDs(
	const std::vector<EntityID>& ids,
	std::vector<TexturedData>& outData)
{
	// get Textured component data for all the input entts;
	// NOTE: this method expects that all the input entts have the Textured component;

	const Textured& comp = *pTexturesComponent_;
	std::vector<index> idxs;

	Utils::GetIdxsInSortedArr(comp.ids_, ids, idxs);
	outData.resize(std::ssize(ids));

	for (int i = 0; const index idx : idxs)
		outData[i++] = comp.data_[idx];
}

///////////////////////////////////////////////////////////

void TexturesSystem::FilterEnttsWhichHaveOwnTex(
	const std::vector<EntityID>& ids,
	std::vector<EntityID>& outEnttsWithOwnTex,
	std::vector<EntityID>& outEnttsWithMeshTex)     
{
	// out: 1. entts with Textured component (so they have some specific textures)
	//      2. entts without Textured component (will be painted with its mesh textures)

	std::vector<bool> flags;
	Utils::GetExistingFlags(pTexturesComponent_->ids_, ids, flags);

	outEnttsWithOwnTex.resize(std::ssize(ids));
	outEnttsWithMeshTex.resize(std::ssize(ids));

	int pos1 = 0;
	int pos2 = 0;

	for (int i = 0; i < (int)std::ssize(ids); ++i)
	{
		// if has the Textured component
		if (flags[i])
		{
			outEnttsWithOwnTex[pos1++] = ids[i];
		}
		else
		{
			outEnttsWithMeshTex[pos2++] = ids[i];
		}
	}

	outEnttsWithOwnTex.resize(pos1);
	outEnttsWithMeshTex.resize(pos2);
}


}