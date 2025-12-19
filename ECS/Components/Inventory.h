// =================================================================================
// Filename:       Intentory.h
// Description     an ECS component for storing inventory data
// 
// Created:        06.09.25   by DimaSkup
// =================================================================================
#pragma once

#include <cvector.h>
#include <Types.h>

namespace ECS
{

struct InventoryData
{
    cvector<EntityID> items;
};

struct Inventory
{
    cvector<EntityID>      ownersIds;
    cvector<InventoryData> inventories;
};

} // namespace
