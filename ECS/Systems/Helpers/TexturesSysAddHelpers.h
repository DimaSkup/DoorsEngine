#pragma once

#include "../../Common/Assert.h"
#include "../../Common/Utils.h"
#include <vector>

namespace ECS
{

///////////////////////////////////////////////////////////

void CheckInputData(
	const std::vector<EntityID>& enttsIDs,
	const std::vector<std::vector<TexID>>& texIDs)     // array of textures IDs arrays
{
	Assert::True(Utils::CheckArrSizesEqual(enttsIDs, texIDs), "entities count != textures IDs arrays count");

	const size expectTexCount = 22; // (size)Textured::TEXTURES_TYPES_COUNT;
	bool texIDsAreOk = true;

	// check if we have proper number of textures IDs
	for (const std::vector<TexID>& idsArr : texIDs)
		texIDsAreOk &= (std::ssize(idsArr) == expectTexCount);

	Assert::True(texIDsAreOk, "the textures IDs data is INVALID (wrong number of)");
}

///////////////////////////////////////////////////////////



} // namespace ECS