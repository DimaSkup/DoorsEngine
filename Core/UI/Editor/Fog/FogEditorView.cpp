#include <CoreCommon/pch.h>
#include "FogEditorView.h"
#include <UICommon/ieditor_controller.h>
#include <UICommon/color.h>
#include <UICommon/editor_cmd.h>
#include <imgui.h>


namespace UI
{

ViewFog::ViewFog(IEditorController* pController) : pController_(pController)
{
    if (!pController)
    {
        LogErr("ptr to the view listener == nullptr");
    }
}

///////////////////////////////////////////////////////

void ViewFog::Draw(const ModelFog* pData)
{
    // show the fog editor fields

    // make local copies of the current model data to use it in the fields
    FogData data;
    pData->GetData(data);

    // draw editor fields
    if (ImGui::Checkbox("Enable fog", &data.fogEnabled))
    {
        CmdChangeFloat cmd(CHANGE_FOG_ENABLED, (float)data.fogEnabled);
        pController_->ExecCmd(&cmd);
    }

    if (ImGui::ColorEdit3("Fog color", data.fogColor.rgb))
    {
        CmdChangeColor cmd(CHANGE_FOG_COLOR, data.fogColor);
        pController_->ExecCmd(&cmd);
    }

    if (ImGui::DragFloat("Fog start", &data.fogStart))
    {
        CmdChangeFloat cmd(CHANGE_FOG_START, data.fogStart);
        pController_->ExecCmd(&cmd);
    }

    if (ImGui::DragFloat("Fog range", &data.fogRange))
    {
        CmdChangeFloat cmd(CHANGE_FOG_RANGE, data.fogRange);
        pController_->ExecCmd(&cmd);
    }
}


} // namespace UI
