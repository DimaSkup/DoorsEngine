// =================================================================================
// Filename:    EnttComponentsEditor.h
// Description: a container for editor elements which are responsible for
//              editing ECS (Entity-Component-System) components of the
//              selected entity
//
// Created:     03.04.2025 by DimaSkup
// =================================================================================
#pragma once

#include <UICommon/IFacadeEngineToUI.h>
#include <map>
#include <string>


namespace UI
{

class EnttComponentsEditor
{
public:
    EnttComponentsEditor(const std::map<eEnttComponentType, std::string>& componentTypeName);

private:
    std::map<eEnttComponentType, std::string> componentTypeName_;
};

} // namespace UI
