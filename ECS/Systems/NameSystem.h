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
        const EntityID* ids,
        const EntityName* names,
        const size numEntts);
	
#if 0
	// TODO
	void RenameRecords(
		const cvector<EntityID>& enttsIDs,
		const cvector<EntityName>& newEnttsNames);

	void RemoveRecords(const cvector<EntityID>& enttsIDs);
#endif

	//
	// getters
	//
	EntityID GetIdByName(const EntityName& name);
	const EntityName& GetNameById(const EntityID& id) const;

	
private:
	void CheckInputData(
		const cvector<EntityID>& ids,
		const cvector<EntityName>& names);

	index GetIdxByID(const EntityID id) const;

private:
	Name* pNameComponent_ = nullptr;

};

}
