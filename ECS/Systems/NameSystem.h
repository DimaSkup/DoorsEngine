// **********************************************************************************
// Filename:      NameSystem.h
// Description:   Entity-Component-System (ECS) system for control entities names;
// 
// Created:       12.06.24
// **********************************************************************************
#pragma once

#include "../Common/Types.h"
#include "../Components/Name.h"

namespace ECS
{

class NameSystem final
{
public:
	NameSystem(Name* pNameComponent);
	~NameSystem() {}

	void Serialize(std::ofstream& fout, u32& offset);
	void Deserialize(std::ifstream& fin, const u32 offset);

	void AddRecords(
		const std::vector<EntityID>& enttsIDs,
		const std::vector<EntityName>& enttsNames);
	
#if 0
	// TODO
	void RenameRecords(
		const std::vector<EntityID>& enttsIDs,
		const std::vector<EntityName>& newEnttsNames);

	void RemoveRecords(const std::vector<EntityID>& enttsIDs);
#endif

	//
	// getters
	//
	EntityID GetIdByName(const EntityName& name);
	const EntityName& GetNameById(const EntityID& id) const;

	// for different debug purposes
	void PrintAllNames();
	
private:
	void CheckInputData(
		const std::vector<EntityID>& ids,
		const std::vector<EntityName>& names);

	index GetIdxByID(const EntityID id) const;

private:
	Name* pNameComponent_ = nullptr;

};

}