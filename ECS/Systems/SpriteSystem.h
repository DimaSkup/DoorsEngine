/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: SpriteSystem.h
    Desc:     ECS system to handle 2D sprites of entities

    Created:  04.01.2026  by DimaSkup
\**********************************************************************************/
#pragma once

#include "../Components/Sprite.h"

namespace ECS
{

class SpriteSystem
{
public:
    SpriteSystem(Sprite* pSpriteComponent);

    bool AddRecord(
        const EntityID enttId,
        const TexID texId,
        const uint16 leftPos,
        const uint16 topPos,
        const uint16 width,
        const uint16 height);

    void GetData(
        const EntityID enttId,
        TexID& texId,
        uint16& left,
        uint16& top,
        uint16& width,
        uint16& height) const;


    inline const EntityID* GetAllSpritesIds() const { return pSpriteComponent_->ids.data()+1; }
    inline size            GetNumAllSprites() const { return pSpriteComponent_->ids.size()-1; }

private:
    index GetIdx(const EntityID id) const;

private:
    Sprite* pSpriteComponent_ = nullptr;
};

} // namespace
