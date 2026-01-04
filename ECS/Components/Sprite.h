/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: Sprite.h
    Desc:     ECS component to hold data about 2D sprites

    Created:  04.01.2026  by DimaSkup
\**********************************************************************************/
#pragma once

#include <types.h>

namespace ECS
{

struct SpriteData
{
    TexID  textureId = 0;
    uint16 left      = 0;       // left edge
    uint16 top       = 0;       // top edge
    uint16 width     = 100;
    uint16 height    = 100;
};

//---------------------------------------------------------

struct Sprite
{
    Sprite()
    {
        // push data which will serve us as "invalid"
        // (we will receive it in cases when try to get data by wrong ID or name)
        ids.push_back(INVALID_ENTITY_ID);
        data.push_back(SpriteData());
    }

    cvector<EntityID>   ids;
    cvector<SpriteData> data;
};

} // namespace

