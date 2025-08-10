// **********************************************************************************
// Filename:      NameSystem.h
// Description:   Entity-Component-System (ECS) system for control entities names;
// 
// Created:       12.06.24
// **********************************************************************************
#pragma once

#include <Types.h>
#include "../Components/Name.h"

namespace ECS
{

class NameSystem
{
public:
	NameSystem(Name* pNameComponent);
	~NameSystem() {}

	void Serialize(std::ofstream& fout, u32& offset);
	void Deserialize(std::ifstream& fin, const u32 offset);

    void AddRecords(
        const EntityID* ids,
        const std::string* names,
        const size numEntts);

	EntityID GetIdByName(const std::string& name);
	const char* GetNameById(const EntityID& id) const;

private:
	Name* pNameComponent_ = nullptr;

};

}
