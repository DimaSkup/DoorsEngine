// =================================================================================
// Filename:    ModelsAssetsList.h
// 
// Created:     22.03.2025 by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include "ModelsAssetsList.h"
#include <imgui.h>

namespace UI
{

void ModelsAssetsList::LoadModelsNamesList(IFacadeEngineToUI* pFacade)
{
    if (pFacade == nullptr)
    {
        LogErr("ptr to facade == nullptr, dude you have to initialize it first!");
        return;
    }

    // if our list of names was empty before we load data
    if (modelsNames_.empty())
    {
        pFacade->GetModelsNamesList(modelsNames_);
    }
}

///////////////////////////////////////////////////////////

void ModelsAssetsList::PrintModelsNamesList()
{
    // print a selectable list of models names

    const std::string selectedName = selectedModelName_;

    for (const std::string& name : modelsNames_)
    {
        if (ImGui::Selectable(name.c_str(), selectedName == name, ImGuiSelectableFlags_DontClosePopups))
            selectedModelName_ = name;
    }
}

} // namespace UI
