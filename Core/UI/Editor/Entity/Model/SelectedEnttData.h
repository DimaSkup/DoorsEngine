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
#include <UICommon/ui_common_types.h>


namespace UI
{

struct SelectedEnttData
{
    inline bool IsSelectedAnyEntt() const { return (id != 0); }

    
    EntityID           id = 0;                        // selected entity ID
    char               name[MAX_LEN_ENTT_NAME];       // selected entity name
    eEnttLightType     lightType = NUM_LIGHT_TYPES;

    eEnttComponentType addedComponents[eEnttComponentType::NUM_COMPONENTS];
    size               numAddedComponents = 0;
};

} // namespace UI
