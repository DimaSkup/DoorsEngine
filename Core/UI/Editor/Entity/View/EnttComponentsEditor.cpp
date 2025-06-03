// =================================================================================
// Filename:    EnttComponentsEditor.h
// Description: implementation of the EnttComponentsEditor functional
// 
// Created:     03.04.2025 by DimaSkup
// =================================================================================
#include <CoreCommon/pch.h>
#include "EnttComponentsEditor.h"
#include <imgui.h>


namespace UI
{

EnttComponentsEditor::EnttComponentsEditor(
    const std::map<eEnttComponentType, std::string>& componentTypeName) :
    componentTypeName_(componentTypeName)
{
    CAssert::True(!componentTypeName.empty(), "input map of pairs [component_type => component_name] is EMPTY");
}

///////////////////////////////////////////////////////////

void RenderAddedComponentsFields()
{
    // render fields for editing components which are added to selected entity

    ImGui::Text("Added components:");

#if 0
    for (const uint32_t type : selectedEnttData_.componentTypes)
    {
        if (ImGui::CollapsingHeader(name.c_str(), ImGuiTreeNodeFlags_SpanFullWidth))
        {
            ImGui::Text("%s component fields", name.c_str());
        }
    }
#endif
}


} // namespace UI
