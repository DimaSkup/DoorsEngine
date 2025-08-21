#include "../Common/pch.h"
#include "NameSystem.h"
#pragma warning (disable : 4996)


namespace ECS
{

NameSystem::NameSystem(Name* pNameComponent)
{
    CAssert::NotNullptr(pNameComponent, "ptr to the Name component == nullptr");
    pNameComponent_ = pNameComponent;

    // add invalid data; this data is returned when we ask for wrong entity
    pNameComponent_->ids_.push_back(INVALID_ENTITY_ID);
    pNameComponent_->names_.push_back("invalid");
}

//---------------------------------------------------------
// Desc:   add a name of entity by id
//---------------------------------------------------------
bool NameSystem::AddRecord(const EntityID id, const char* name)
{
    if (StrHelper::IsEmpty(name))
    {
        LogErr(LOG, "input name is empty for entt_id: %" PRIu32, id);
        return false;
    }

    if (!IsUnique(name))
    {
        LogErr(LOG, "input name is not unique (entt_id: %" PRIu32 ", name: %s)", id, name);
        return false;
    } 


    Name& comp = *pNameComponent_;
    const index idx = comp.ids_.get_insert_idx(id);

    // execute sorted insertion of IDs
    comp.ids_.insert_before(idx, id);
    comp.names_.insert_before(idx, name);

    return true;
}

//---------------------------------------------------------
// Desc:    add name for each entity from the input arr
//          (for instance: ids[2] => names[2])
//---------------------------------------------------------
bool NameSystem::AddRecords(
    const EntityID* ids,
    const std::string* names,
    const size numEntts)
{
    // check input args
    if (!ids)
    {
        LogErr(LOG, "input arr of entities ids == nullptr");
        return false;
    }

    if (!names)
    {
        LogErr(LOG, "input arr of names == nullptr");
        return false;
    }

    if (numEntts <= 0)
    {
        LogErr(LOG, "input num of entities must be > 0");
        return false;
    }

    // check if each input name is not empty and unique
    for (index i = 0; i < numEntts; ++i)
    {
        if (names[i].empty())
        {
            LogErr(LOG, "name by idx[%td] is empty (entt_id: %" PRIu32 ")", i, ids[i]);
            return false;
        }

        if (!IsUnique(names[i].c_str()))
        {
            LogErr(LOG, "name by idx[%td] isn't unique (entt_id: %" PRIu32 ", name: %s)", i, ids[i], names[i].c_str());
            return false;
        }
    }

    //*******************************

    Name& comp = *pNameComponent_;

    cvector<index> idxs;
    comp.ids_.get_insert_idxs(ids, numEntts, idxs);

    // allocate additional memory ahead
    const size newCapacity = comp.ids_.size() + numEntts;
    comp.ids_.reserve(newCapacity);
    comp.names_.reserve(newCapacity);

    // execute sorted insertion of IDs
    for (index i = 0; i < numEntts; ++i)
        comp.ids_.insert_before(idxs[i] + i, ids[i]);

    for (index i = 0; i < numEntts; ++i)
        comp.names_.insert_before(idxs[i] + i, std::move(names[i]));

    return true;
}

///////////////////////////////////////////////////////////

EntityID NameSystem::GetIdByName(const char* name) const
{
    if (StrHelper::IsEmpty(name))
    {
        LogErr(LOG, "input name is empty");
        return INVALID_ENTITY_ID;
    }

    // if there is such a name in the arr we return a responsible entity ID;
    const Name& comp = *pNameComponent_;
    const index idx  = comp.names_.find(name);

    return (idx != -1) ? comp.ids_[idx] : INVALID_ENTITY_ID;
}

//---------------------------------------------------------
// Desc:   get a name of entity by input identifier
//---------------------------------------------------------
const char* NameSystem::GetNameById(const EntityID& id) const
{
    const Name& comp = *pNameComponent_;
    const index idx  = comp.ids_.get_idx(id);
    const bool exist = (comp.ids_[idx] == id);

    return comp.names_[idx * exist].c_str();
}

}
