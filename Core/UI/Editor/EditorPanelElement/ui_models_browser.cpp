// =================================================================================
// Filename:    ui_models_browser.cpp
// 
// Created:     22.03.2025 by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include "ui_models_browser.h"
#include <UICommon/IFacadeEngineToUI.h>
#include <imgui.h>

namespace UI
{

void UIModelsBrowser::LoadModelsNamesList(IFacadeEngineToUI* pFacade)
{
    if (pFacade == nullptr)
    {
        LogErr(LOG, "ptr to facade == nullptr, dude you have to initialize it first!");
        return;
    }

    // if our list of names was empty before we load data
    if (modelsNames_.empty())
    {
        pFacade->GetModelsNamesList(modelsNames_);
    }
}

//---------------------------------------------------------
// print a selectable list of models names
//---------------------------------------------------------
void UIModelsBrowser::PrintModelsNamesList()
{
    const std::string& selectedName = selectedModelName_;

    for (const std::string& name : modelsNames_)
    {
        if (ImGui::Selectable(name.c_str(), selectedName == name, ImGuiSelectableFlags_DontClosePopups))
            selectedModelName_ = name;
    }
}

} // namespace UI
