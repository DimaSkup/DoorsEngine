// =================================================================================
// Filename:     RenderSystem.cpp
// Description:  implementation of the ECS RenderSystem's functional
// 
// Created:      21.05.24
// =================================================================================
#include "../Common/pch.h"
#include "RenderSystem.h"


namespace ECS
{

RenderSystem::RenderSystem(Rendered* pRenderComponent) :
    pRenderComponent_(pRenderComponent)
{
    CAssert::NotNullptr(pRenderComponent, "ptr to the Rendered component == nullptr");

    // reserve some memory ahead
    pRenderComponent->ids.reserve(128);
}

//---------------------------------------------------------
// Desc:   set that input entities by IDs are able to be rendered onto the screen;
//---------------------------------------------------------
void RenderSystem::AddRecords(const EntityID* ids, const size numEntts)
{
    CAssert::True(ids != nullptr, "input ptr to entities IDs arr == nullptr");
    CAssert::True(numEntts > 0, "input number of entts must be > 0");

    Rendered& comp = *pRenderComponent_;
    cvector<index> idxs;

    comp.ids.get_insert_idxs(ids, numEntts, idxs);

    // execute sorted insertion of input values
    for (index i = 0; i < numEntts; ++i)
        comp.ids.insert_before(idxs[i] + i, ids[i]);
}

/////////////////////////////////////////////////

void RenderSystem::RemoveRecords(const EntityID* ids, const size numEntts)
{
    if (!ids)
    {
        LogErr(LOG, "can't remove records: input ptr to entts ids == nullptr");
        return;
    }

    if (numEntts == 0)
    {
        LogErr(LOG, "can't remove records: input number of entts must be > 0 (curr: %d)", (int)numEntts);
        return;
    }


    

    // remove one record
    if (numEntts == 1)
        RemoveRecord(ids[0]);

    // remove multiple records at once
    else
    {
        Rendered& comp = *pRenderComponent_;
        assert(0 && "TODO");
    }
}

void RenderSystem::RemoveRecord(const EntityID id)
{
    Rendered& comp = *pRenderComponent_;
    const index idx = comp.ids.get_idx(id);
    comp.ids.erase(idx);
}

} // namespace ECS
