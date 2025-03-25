#include "NameSystem.h"

#include "../Common/Assert.h"
#include "../Common/log.h"

#include "SaveLoad/NameSysSerDeser.h"


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
}

///////////////////////////////////////////////////////////

void NameSystem::Deserialize(std::ifstream& fin, const u32 offset)
{
}

///////////////////////////////////////////////////////////

void NameSystem::AddRecords(
    const EntityID* ids,
    const EntityName* names,
    const size numEntts)
{
    // add name for each entity from the input arr (for instance: ids[2] => names[2])

    Name& comp = *pNameComponent_;

    cvector<index> idxs;
    comp.ids_.get_insert_idxs(ids, numEntts, idxs);

    // allocate additional memory ahead if we need
    const size newCapacity = comp.ids_.size() + numEntts;
    comp.ids_.reserve(newCapacity);
    comp.names_.reserve(newCapacity);

    // execute sorted insertion of IDs
    for (index i = 0; i < numEntts; ++i)
        comp.ids_.insert_before(idxs[i] + i, ids[i]);

    for (index i = 0; i < numEntts; ++i)
        comp.names_.insert_before(idxs[i] + i, names[i]);
}

///////////////////////////////////////////////////////////

EntityID NameSystem::GetIdByName(const EntityName& name)
{
    // if there is such a name in the arr we return a responsible entity ID;
    const Name& comp = *pNameComponent_;
    const index idx  = comp.names_.find(name);

    return (idx != -1) ? comp.ids_[idx] : INVALID_ENTITY_ID;
}

///////////////////////////////////////////////////////////

const EntityName& NameSystem::GetNameById(const EntityID& id) const
{
    // if there is such an ID in the arr we return a responsible entity name;
    const Name& comp = *pNameComponent_;
    const bool exist = comp.ids_.binary_search(id);
    const index idx  = comp.ids_.get_idx(id);

    return (exist) ? comp.names_[idx] : INVALID_ENTITY_NAME;
}


}
