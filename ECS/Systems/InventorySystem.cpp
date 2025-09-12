// =================================================================================
// Filename:       Intentory.cpp
// Created:        06.09.25   by DimaSkup
// =================================================================================
#include "../Common/pch.h"
#include "InventorySystem.h"

namespace ECS
{

InventorySystem::InventorySystem(Inventory* pInventory) :
    pInventory_(pInventory)
{
    CAssert::True(pInventory_, "input ptr to inventory component == nullptr");
}

InventorySystem::~InventorySystem()
{
}

//---------------------------------------------------------
// Desc:   an entity by input id will have its own inventory
//---------------------------------------------------------
void InventorySystem::AddInventory(const EntityID id)
{
    Inventory& comp = *pInventory_;

    // check if input entity already has an inventory
    if (comp.ownersIds.binary_search(id))
        return;


    // find a place for sorted insertion
    const index idx = comp.ownersIds.get_insert_idx(id);

    // add an empty inventory for entity
    comp.ownersIds.insert_before(idx, id);
    comp.inventories.insert_before(idx, InventoryData());
}

//---------------------------------------------------------
// Desc:   add a new item into inventory of entity
//---------------------------------------------------------
void InventorySystem::AddItem(const EntityID ownerId, const EntityID itemId)
{
    Inventory& comp = *pInventory_;
    const index idx = comp.ownersIds.get_idx(ownerId);

    // check if entity has inventory
    if (comp.ownersIds[idx] != ownerId)
    {
        LogErr(LOG, "there is no inventory related to entity: %" PRIu32, ownerId);
        return;
    }

    // prevent double adding
    InventoryData& inventory = comp.inventories[idx];

    if (inventory.items.binary_search(itemId))
    {
        LogErr(LOG, "there is already an item (%" PRIu32 ") in the inventory of entity: %" PRIu32, itemId, ownerId);
        return;
    }

    // add an item
    inventory.items.push_back(itemId);
}

//---------------------------------------------------------
// Desc:   drop an item from inventory of entity
//---------------------------------------------------------
void InventorySystem::DropItem(const EntityID ownerId, const EntityID itemId)
{

}

//---------------------------------------------------------
// Desc:   move an item from inventory of entity by ownerIdSrc id
//         into inventory of entity by ownerIdDst id
//---------------------------------------------------------
void InventorySystem::MoveItem(
    const EntityID ownerIdSrc,
    const EntityID ownerIdDst,
    const EntityID itemId)
{

}

//---------------------------------------------------------
// Desc:   return from inventory an id of item by idx
// Args:   - ownerId:  owner of the inventory
//         - idx:      item's idx in arr
//---------------------------------------------------------
EntityID InventorySystem::GetItemByIdx(const EntityID ownerId, const index itemIdx)
{
    Inventory& comp = *pInventory_;
    const index ownerIdx = comp.ownersIds.get_idx(ownerId);

    // check if entity has inventory
    if (comp.ownersIds[ownerIdx] != ownerId)
    {
        LogErr(LOG, "there is no inventory related to entity: %" PRIu32, ownerId);
        return INVALID_ENTITY_ID;
    }

    InventoryData& inventory = comp.inventories[ownerIdx];
    const size numItems = inventory.items.size();

    // check if there is such item
    if (itemIdx >= 0 && itemIdx < numItems)
    {
        return inventory.items[itemIdx];
    }

    LogErr(LOG, "there is no item by idx (%d) in inventory of entity: %" PRIu32, (int)itemIdx, ownerId);
    return INVALID_ENTITY_ID;
}

}
