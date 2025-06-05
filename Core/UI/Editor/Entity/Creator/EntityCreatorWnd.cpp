// =================================================================================
// Filename:    EntityCreatorWnd.cpp
//
// Created:     18.03.2025  by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include "EntityCreatorWnd.h"
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
    ImGui::Text("Transform component:");
    ImGui::DragFloat3("position",  transformData_.position.xyz);
    ImGui::DragFloat3("direction", transformData_.direction.xyz);
    ImGui::DragFloat("uniform scale", &transformData_.uniformScale);
    ImGui::Separator();
}

///////////////////////////////////////////////////////////

void EntityCreatorWnd::ShowModelComponentFields(IFacadeEngineToUI* pFacade)
{
    if (ImGui::CollapsingHeader("Setup Model component:"))
    {
        ImGui::Text("Select Assets:");

        modelsList_.LoadModelsNamesList(pFacade);
        modelsList_.PrintModelsNamesList();
    }
}

///////////////////////////////////////////////////////////

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
        ImGui::InputText("Name", nameData_.enttName, nameData_.maxNameLength);
        ShowAddComponentCheckboxes();
        ImGui::Separator();

        // transformation is required so we show its fields in any case
        ShowTransformComponentFields();

        if (addedComponents_.isAddedModel)
            ShowModelComponentFields(pFacade);

        if (addedComponents_.isAddedRendered)
            ShowRenderedComponentFields();

        ImGui::Separator();

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
        LogMsgf(" ");
        LogMsgf("%sCreated Entity", YELLOW);
        LogMsgf("%sName: %s", YELLOW, nameData_.enttName);

        // debug: print transformation component info
        const Vec3& pos = transformData_.position;
        const Vec3& dir = transformData_.direction;
        const float uniScale = transformData_.uniformScale;
        LogMsgf("%sTransform:", YELLOW);
        LogMsgf("%spos: %f %f %f", YELLOW, pos.x, pos.y, pos.z);
        LogMsgf("%spos: %f %f %f", YELLOW, dir.x, dir.y, dir.z);
        LogMsgf("%suniform scale: %d", YELLOW, uniScale);

        // debug: print model component info
        if (addedComponents_.isAddedModel)
            LogMsgf("%swith model: %s", YELLOW, modelData_.selectedModelName.c_str());

        // debug: print rendered component info
        if (addedComponents_.isAddedRendered)
            LogMsgf("%swith rendered component", YELLOW);

        LogMsgf("");

        EntityID enttID = pFacade->CreateEntity();
        ModelID modelID = pFacade->GetModelIdByName(modelData_.selectedModelName);

        pFacade->AddTransformComponent(enttID, pos, dir, uniScale);
        pFacade->AddNameComponent(enttID, nameData_.enttName);
        pFacade->AddModelComponent(enttID, modelID);
        pFacade->AddRenderedComponent(enttID);
        pFacade->AddBoundingComponent(enttID, 1, DirectX::BoundingBox({ 0,0,0 }, { 1,1,1 }));

    }
#endif
}


} // namespace UI
