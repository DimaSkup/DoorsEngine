#include "Utils.h"

namespace ECS
{

	// ****************************************************************************
	// different help purposes API

	std::string Utils::GetEnttsIDsAsString(
		const EntityID* ids,
		const int count,
		const std::string& glue)
	{
		// join all the input ids into a single str and separate it with glue

		std::stringstream ss;

		for (int i = 0; i < count; ++i)
			ss << ids[i] << glue;

		return ss.str();
	}


	// ****************************************************************************
	// sorted insertion API

	index Utils::GetPosForID(
		const std::vector<EntityID>& enttsIDs,
		const EntityID& enttID)
	{
		// get position (index) into array for sorted INSERTION of ID
		// 
		// input:  1. a SORTED array of IDs
		//         2. an ID for which we want to find an insertion pos
		// return:    an insertion idx for ID

		return std::distance(enttsIDs.begin(), std::upper_bound(enttsIDs.begin(), enttsIDs.end(), enttID));
	}

} // namespace Utils

