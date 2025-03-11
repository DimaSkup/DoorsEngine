#include "NameSystem.h"

#include "../Common/Assert.h"
#include "../Common/Utils.h"
#include "../Common/log.h"

#include "SaveLoad/NameSysSerDeser.h"
#include <format>


namespace ECS
{


NameSystem::NameSystem(Name* pNameComponent)
{
	Assert::NotNullptr(pNameComponent, "ptr to the Name component == nullptr");
	pNameComponent_ = pNameComponent;

	// add invalid data; this data is returned when we ask for wrong entity
	pNameComponent_->ids_.push_back(INVALID_ENTITY_ID);
	pNameComponent_->names_.push_back(INVALID_ENTITY_NAME);
}

///////////////////////////////////////////////////////////

void NameSystem::Serialize(std::ofstream& fout, u32& offset)
{
	const Name& component = *pNameComponent_;

	NameSysSerDeser::Serialize(
		fout, 
		offset,
		static_cast<u32>(component.type_),
		component.ids_,
		component.names_);
}

///////////////////////////////////////////////////////////

void NameSystem::Deserialize(std::ifstream& fin, const u32 offset)
{
	Name& component = *pNameComponent_;

	NameSysSerDeser::Deserialize(
		fin,
		offset,
		component.ids_,
		component.names_);
}

///////////////////////////////////////////////////////////

void NameSystem::AddRecords(
	const std::vector<EntityID>& ids,
	const std::vector<EntityName>& names)
{
	// add name for each entity from the input arr

	Name& component = *pNameComponent_;
	CheckInputData(ids, names);

	for (index idx = 0; idx < std::ssize(ids); ++idx)
	{
		const index insertAt = Utils::GetPosForID(component.ids_, ids[idx]);
		Utils::InsertAtPos(component.ids_, insertAt, ids[idx]);
		Utils::InsertAtPos(component.names_, insertAt, names[idx]);
	}
}

///////////////////////////////////////////////////////////

void NameSystem::PrintAllNames()
{
	// print out all the names into the console
	const std::vector<EntityID>& ids     = pNameComponent_->ids_;
	const std::vector<EntityName>& names = pNameComponent_->names_;

	for (index idx = 0; idx < std::ssize(ids); ++idx)
		ECS::Log::Print(std::format("id:name = {} : {}", ids[idx], names[idx]));
}

///////////////////////////////////////////////////////////

EntityID NameSystem::GetIdByName(const EntityName& name)
{
	const Name& comp = *pNameComponent_;

	// check if such name exists
	if (!Utils::ArrHasVal(comp.names_, name))
		return INVALID_ENTITY_ID;
	
	// if there is such a name in the arr we return a responsible entity ID;
	return comp.ids_[Utils::FindIdxOfVal(comp.names_, name)];
}

///////////////////////////////////////////////////////////

const EntityName& NameSystem::GetNameById(const EntityID& id) const
{
	// if there is such an ID in the arr we return a responsible entity name;
	// or in another case we return invalid value
	return pNameComponent_->names_[GetIdxByID(id)];
}

///////////////////////////////////////////////////////////

void NameSystem::CheckInputData(
	const std::vector<EntityID>& ids,
	const std::vector<EntityName>& names)
{
	// here we check if input data is correct to store it into the Name component

	const Name& component = *pNameComponent_;
	bool idsValid = true;
	bool namesValid = true;
	bool namesUnique = true;

	// check ids are valid (entts doesn't have the Name component yet)
	idsValid = !Utils::CheckValuesExistInSortedArr(component.ids_, ids);

	// check names are valid
	for (const EntityName& name : names)
		namesValid &= (!name.empty());

	// check names are unique
	namesUnique = !Utils::CheckValuesExistInArr(component.names_, names);

	Assert::True(idsValid, "there is already an entt with the Name component");
	Assert::True(namesValid, "some input name is empty");
	Assert::True(namesUnique, "some input name isn't unique");
}

///////////////////////////////////////////////////////////

index NameSystem::GetIdxByID(const EntityID id) const
{
	// return valid idx if there is an entity by such ID;
	// or return 0 if there is no such entity;
	const std::vector<EntityID>& ids = pNameComponent_->ids_;
	//return (Utils::BinarySearch(ids, id)) ? Utils::GetIdxInSortedArr(ids, id) : 0;

	const index idx = Utils::GetIdxInSortedArr(ids, id);

	if (ids[idx] != id)
	{
		Log::Error("there is no name for entity by ID: " + std::to_string(id));
		return 0;
	}

	return idx;
}


}