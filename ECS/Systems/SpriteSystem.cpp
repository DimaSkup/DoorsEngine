/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: SpriteSystem.cpp
    Desc:     implementation ECS system to handle 2D sprites of entities

    Created:  04.01.2026  by DimaSkup
\**********************************************************************************/
#include "../Common/pch.h"
#include "SpriteSystem.h"

namespace ECS
{

//---------------------------------------------------------
// 
//---------------------------------------------------------
SpriteSystem::SpriteSystem(Sprite* pSpriteComponent) :
    pSpriteComponent_(pSpriteComponent)
{
    assert(pSpriteComponent);
}

//---------------------------------------------------------
//---------------------------------------------------------
bool SpriteSystem::AddRecord(
    const EntityID enttId,
    const TexID texId,
    const uint16 leftPos,
    const uint16 topPos,
    const uint16 width,
    const uint16 height)
{
    Sprite& comp = *pSpriteComponent_;

    if (comp.ids.binary_search(enttId))
    {
        LogErr(LOG, "there is already a sprite by id: %" PRIu32, enttId);
        return false;
    }

    const index idx = comp.ids.get_insert_idx(enttId);

    comp.ids.insert_before(idx, enttId);
    comp.data.insert_before(idx, SpriteData(texId, leftPos, topPos, width, height));

    return true;
}

//---------------------------------------------------------
//---------------------------------------------------------
void SpriteSystem::GetData(
    const EntityID enttId,
    TexID& texId,
    uint16& left,
    uint16& top,
    uint16& width,
    uint16& height) const
{
    Sprite& comp = *pSpriteComponent_;
    const index idx = GetIdx(enttId);

    texId  = comp.data[idx].textureId;
    left   = comp.data[idx].left;
    top    = comp.data[idx].top;
    width  = comp.data[idx].width;
    height = comp.data[idx].height;
}

//---------------------------------------------------------
// Desc:  get index to data of entity by id
//        or return 0 if there is no such a record
//---------------------------------------------------------
index SpriteSystem::GetIdx(const EntityID id) const
{
    const index idx = pSpriteComponent_->ids.get_idx(id);

    if (idx > 0 && idx < pSpriteComponent_->ids.size())
        return idx;

    LogErr(LOG, "there is no 2D sprites by entt id: %" PRIu32, id);
    return 0;
}

} // namespace 

