// =================================================================================
// Filename:    SelectedEnttData.h
// Description: container for the entity common data we fill it with data
//              after we pick (select) any entity in the editor;
//              so we store this data between frames
//              and don't reload it each frame anew;
//
// Created:     02.04.2025  by DimaSkup
// =================================================================================
#pragma once

#include <UICommon/IFacadeEngineToUI.h>
#include <cvector.h>
#include <string>


namespace UI
{

enum eEnttLightType
{
    ENTT_LIGHT_TYPE_DIRECTED,
    ENTT_LIGHT_TYPE_POINT,
    ENTT_LIGHT_TYPE_SPOT,
    NUM_LIGHT_TYPES
};

struct SelectedEnttData
{
    inline bool IsSelectedAnyEntt()         const { return (id != 0); }

    
    EntityID        id = 0;                         // selected entity ID
    std::string     name;                           // selected entity name
    eEnttLightType  lightType = NUM_LIGHT_TYPES;
    
    cvector<eEnttComponentType> componentsTypes;   // types of components which are added to the entity
};


} // namespace UI
