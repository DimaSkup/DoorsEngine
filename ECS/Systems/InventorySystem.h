// =================================================================================
// Filename:       Intentory.h
// Description     an ECS system for handling inventory data
// 
// Created:        06.09.25   by DimaSkup
// =================================================================================
#pragma once

#include "../Components/Inventory.h"

namespace ECS
{

class InventorySystem
{
public:
    InventorySystem(Inventory* pInventory);
    ~InventorySystem();

    void AddInventory(const EntityID id);

    void AddItem(const EntityID ownerId, const EntityID itemId);
    void DropItem(const EntityID ownerId, const EntityID itemId);
    void MoveItem(const EntityID ownerIdSrc, const EntityID ownerIdDst, const EntityID itemId);

    EntityID GetItemByIdx(const EntityID ownerId, const index idx);

private:
    Inventory* pInventory_ = nullptr;   // a ptr to the inventory component
};

} // namespace

