// =================================================================================
// Filename:    EntityCreatorWnd.cpp
//
// Created:     18.03.2025  by DimaSkup
// =================================================================================
#include "EntityCreatorWnd.h"
#include <CoreCommon/log.h>

#include <imgui.h>

namespace UI
{

EntityCreatorWnd::EntityCreatorWnd()
{

}

///////////////////////////////////////////////////////////

void EntityCreatorWnd::ShowAddComponentCheckboxes()
{
    ImGui::Text("Add components:");
    ImGui::Checkbox("Model", &(addedComponents_.isAddedModel));
    ImGui::Checkbox("Rendered", &(addedComponents_.isAddedRendered));
}

///////////////////////////////////////////////////////////

void EntityCreatorWnd::ShowTransformComponentFields()
{

    ImGui::DragFloat3("position",  transformData_.position.xyz);
    ImGui::DragFloat3("direction", transformData_.direction.xyz);
    ImGui::DragFloat("uni_scale", &transformData_.uniformScale);
}

///////////////////////////////////////////////////////////

void EntityCreatorWnd::ShowModelComponentFields(IFacadeEngineToUI* pFacade)
{
    if (ImGui::CollapsingHeader("Setup Model component:"))
    {
        ImGui::Text("Select Assets:");

        // load models names
        if (modelData_.modelsNames.empty())
        {
            int numAssets = pFacade->GetNumAssets();

            modelData_.modelsNames.resize(numAssets);

            pFacade->GetAssetsNamesList(
                modelData_.modelsNames.data(),
                (int)modelData_.modelsNames.size());

            Core::Log::Print("models names list is loaded", Core::eConsoleColor::YELLOW);
        }

        std::string& selectedName = modelData_.selectedModelName;

        // print a selectable list of models names
        for (const std::string& name : modelData_.modelsNames)
        {
            if (ImGui::Selectable(name.c_str(), selectedName == name, ImGuiSelectableFlags_DontClosePopups))
                selectedName = name;
        }  
    }
}

void EntityCreatorWnd::ShowRenderedComponentFields()
{
    if (ImGui::CollapsingHeader("Setup Rendered component:"))
    {
        ImGui::Text("Setup");
    }
}

///////////////////////////////////////////////////////////

void EntityCreatorWnd::RenderCreationWindow(bool* pOpen, IFacadeEngineToUI* pFacade)
{
    if (*pOpen == false)
        return;

    ImGui::OpenPopup("Entity Creation");

    // Always center this window when appearing
    const ImGuiViewport* pViewport = ImGui::GetMainViewport();
    const ImVec2 wndCenter = pViewport->GetCenter();
    const ImVec2 wndSize = { 0.5f * pViewport->Size.x, 0.5f * pViewport->Size.y };

    ImGui::SetNextWindowPos(wndCenter, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(wndSize);

    if (ImGui::BeginPopupModal("Entity Creation", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        // header
        ImGui::Text("Here you can create a new entity");
        ImGui::Separator();

        // name + checkboxes
        ImGui::InputText("Name", enttName_, maxNameLength_);
        ShowAddComponentCheckboxes();
        ImGui::Separator();

        // transformation is required so we show its fields in any case
        ShowTransformComponentFields();

        if (addedComponents_.isAddedModel)
            ShowModelComponentFields(pFacade);

        if (addedComponents_.isAddedRendered)
            ShowRenderedComponentFields();


        // OK, Cancel, Reset buttons
        if (ImGui::Button("OK", ImVec2(0.1f * wndSize.x, 0)))
        {
            *pOpen = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(0.1f * wndSize.x, 0)))
        {
            *pOpen = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }


#if 1
    // TEMP
    if (*pOpen == false)
    {
        const Core::eConsoleColor yellow = Core::eConsoleColor::YELLOW;

        Core::Log::SetConsoleColor(yellow);

        Core::Log::Print();
        Core::Log::Print();
        Core::Log::Print("Created Entity");
        Core::Log::Printf("Name: %s", enttName_);

        // debug: print transformation component info
        const Vec3& pos = transformData_.position;
        const Vec3& dir = transformData_.direction;
        Core::Log::Print("Transform:");
        Core::Log::Printf("pos: %f %f %f", pos.x, pos.y, pos.z);
        Core::Log::Printf("pos: %f %f %f", dir.x, dir.y, dir.z);
        Core::Log::Errorf("uniform scale: %d", transformData_.uniformScale);

        // debug: print model component info
        if (addedComponents_.isAddedModel)
            Core::Log::Print("with model: " + modelData_.selectedModelName);

        // debug: print rendered component info
        if (addedComponents_.isAddedRendered)
            Core::Log::Print("with rendered");

        Core::Log::Print();
        Core::Log::Print();
    }
#endif
}


} // namespace UI
