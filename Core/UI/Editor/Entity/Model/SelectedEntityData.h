// =================================================================================
// Filename:    SelectedEntityData.h
// Description: container for the entity common data we fill it with data
//              after we pick (select) any entity in the editor;
//              so we store this data between frames
//              and don't reload it each frame anew;
//
// Created:     02.04.2025  by DimaSkup
// =================================================================================
#pragma once

#include <UICommon/cvector.h>
#include <string>


namespace UI
{

using EntityID = uint32_t;

enum eSelectedEnttType
{
    NONE,
    SKY,
    DIRECTED_LIGHT,
    POINT_LIGHT,
    SPOT_LIGHT,
    CAMERA,
    MODEL,
};

///////////////////////////////////////////////////////////

struct SelectedEntityData
{
    inline bool IsSelectedAnyEntt()         const { return (id != 0); }
    inline bool IsSelectedEnttLightSource() const { return (type == DIRECTED_LIGHT) || (type == POINT_LIGHT) || (type == SPOT_LIGHT); }

    EntityID          id = 0;
    eSelectedEnttType type = NONE;
    std::string       name;

    cvector<std::string> componentsNames;   // components names which are added to the entity
};


} // namespace UI
