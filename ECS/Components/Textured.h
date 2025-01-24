// *********************************************************************************
// Filename:     Textured.h
// Description:  an ECS component which contains textures data of entities;
// 
// Created:      28.06.24
// *********************************************************************************
#pragma once

#include "../Common/Types.h"
#include "../Common/Assert.h"
#include <vector>

namespace ECS
{

struct TexturedData
{
	TexturedData() {}

	TexturedData(const TexID* texIDs, int submeshID) :
		submeshID_(submeshID)
	{
		Assert::NotNullptr(texIDs, "a ptr to tex arr == nullptr");
		Assert::True(submeshID > -1, "wrong submesh ID value");

		// copy 22 textures IDs
		std::copy(texIDs, texIDs + 22, texIDs_);
	}


	TexID texIDs_[22]{ 0 };   // each submesh can have 22 texture types
	int submeshID_ = -1;      // ID of entt's model submesh
};

struct Textured
{
	ComponentType type_ = ComponentType::TexturedComponent;
	static const ptrdiff_t TEXTURES_TYPES_COUNT = 22;  // AI_TEXTURE_TYPE_MAX + 1

	std::vector<EntityID>     ids_;
	std::vector<TexturedData> data_;
};

}