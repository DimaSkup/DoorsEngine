// **********************************************************************************
// Filename:      NameSystem.cpp
// Description:   Entity-Component-System (ECS) system for control entities names;
// **********************************************************************************
#include "../Common/pch.h"
#include "NameSystem.h"
#pragma warning (disable : 4996)


namespace ECS
{

// static arrays for internal purposes
static cvector<index> s_Idxs;


//---------------------------------------------------------
// Desc:  constructor
//---------------------------------------------------------
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
// Desc:    add a name for each entity from the input arr
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

    // check if each input name is not empty and is unique
    for (index i = 0; i < numEntts; ++i)
    {
        const char* name = names[i].c_str();

        if (StrHelper::IsEmpty(name))
        {
            LogErr(LOG, "name by idx[%td] is empty (entt_id: %" PRIu32 ")", i, ids[i]);
            return false;
        }

        if (!IsUnique(name))
        {
            LogErr(LOG, "name by idx[%td] isn't unique (entt_id: %" PRIu32 ", name: %s)", i, ids[i], name);
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
        comp.names_.insert_before(idxs[i] + i, names[i]);

    return true;
}

//---------------------------------------------------------
// Desc:  get entity ID by input name
//        (if there is no record with such name we return 0)
//---------------------------------------------------------
EntityID NameSystem::GetIdByName(const char* name) const
{
    if (StrHelper::IsEmpty(name))
    {
        LogErr(LOG, "input name is empty");
        return INVALID_ENTITY_ID;
    }

    const Name& comp = *pNameComponent_;
    const index idx  = comp.names_.find(name);

    if (IsIdxValid(idx))
        return comp.ids_[idx];

    LogErr(LOG, "there is no entity by name: %s", name);
    return INVALID_ENTITY_ID;
}

//---------------------------------------------------------
// Desc:  check if input arr of names is completely valid
//---------------------------------------------------------
bool CheckNamesArr(const char** names, const size numNames)
{
    if (!names)
    {
        LogErr(LOG, "input ptr to arr of names == nullptr");
        return false;
    }
    if (numNames <= 0)
    {
        LogErr(LOG, "input number of names <= 0");
        return false;
    }

    // check if each input name has any data
    bool namesValid = true;
    for (uint i = 0; i < numNames; ++i)
        namesValid &= (!StrHelper::IsEmpty(names[i]));

    // if we have some invalid name -- print out a dump of input names arr
    if (namesValid == false)
    {
        LogErr(LOG, "some input name is empty:");
        printf("\tprint dump of input arr:\n");

        for (int i = 0; i < (int)numNames; ++i)
        {
            printf("\tname[%d]:  ", i);
            if (names[i])  printf("%s", names[i]);
            printf("\n");
        }

        return false;
    }

    // all the names are valid
    return true;
}

//---------------------------------------------------------
// Desc:  get arr of entities IDs by input arr of names
//---------------------------------------------------------
void NameSystem::GetIdsByNames(
    const char** names,
    const size numNames,
    cvector<EntityID>& outIds) const
{
    if (!CheckNamesArr(names, numNames))
    {
        LogErr(LOG, "can't get IDs by names: input args are invalid");
        outIds.resize(0);
        return;
    }

    const Name& comp = *pNameComponent_;
    s_Idxs.resize(numNames, 0);
    outIds.resize(numNames);

    // find idxs by names
    for (uint i = 0; i < numNames; ++i)
    {
        const index idx = comp.names_.find(names[i]);

        if (IsIdxValid(idx))
            s_Idxs[i] = idx;
    }

    // gather IDs by idxs
    for (uint i = 0; i < numNames; ++i)
        outIds[i] = comp.ids_[s_Idxs[i]];
}


//---------------------------------------------------------
// Desc:  get arr of entities IDs by input arr of names
//
// NOTE:  hardcore version since it's supposed that outIdsArr has size >= numNames
//---------------------------------------------------------
void NameSystem::GetIdsByNames(
    const char** names,
    const size numNames,
    EntityID* outIdsArr) const
{
    if (!CheckNamesArr(names, numNames) || !outIdsArr)
    {
        LogErr(LOG, "can't get IDs by names: input args are invalid");
        return;
    }

    const Name& comp = *pNameComponent_;
    s_Idxs.resize(numNames, 0);

    // find idxs by names
    for (uint i = 0; i < numNames; ++i)
    {
        const index idx = comp.names_.find(names[i]);

        if (IsIdxValid(idx))
            s_Idxs[i] = idx;
    }

    // gather IDs by idxs
    for (uint i = 0; const index idx : s_Idxs)
        outIdsArr[i++] = comp.ids_[idx];
}

//---------------------------------------------------------
// Desc:   get a name of entity by input identifier
//---------------------------------------------------------
const char* NameSystem::GetNameById(const EntityID id) const
{
    const Name& comp = *pNameComponent_;
    const index idx  = comp.ids_.get_idx(id);

    if (IsIdxValid(idx))
        return comp.names_[idx].c_str();

    else
        return comp.names_[0].c_str();
}

//-----------------------------------------------------
// Desc:   check if input name is unique
//-----------------------------------------------------
bool NameSystem::IsUnique(const char* name) const
{
    if (StrHelper::IsEmpty(name))
    {
        LogErr(LOG, "input name is empty");
        return false;
    }

    // if there is no such a name its idx == -1 (so it is a unique name)
    return pNameComponent_->names_.find(name) == -1;
}

}
