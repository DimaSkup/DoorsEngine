// ====================================================================================
// Filename:      FogEditorView.h
// Description:   the View part of the FogEditor MVC;
//                visualises data of the Model::Fog;
//                contains fields to change the Fog params;
// 
// Created:       31.12.24
// ====================================================================================
#pragma once

#include <log.h>

#include <UICommon/IEditorController.h>
#include <UICommon/Color.h>
#include <UICommon/EditorCommands.h>

#include "FogEditorModel.h"
#include <imgui.h>


namespace UI
{

class ViewFog
{
private:
    IEditorController* pController_ = nullptr;

public:
    ViewFog(IEditorController* pController) : pController_(pController)
    {
        if (!pController)
        {
            LogErr("ptr to the view listener == nullptr");
        }
    }

    ///////////////////////////////////////////////////////

    void Draw(const ModelFog* pData)
    {
        // show the fog editor fields

        // make local copies of the current model data to use it in the fields
        FogData data;
        pData->GetData(data);

        // draw editor fields
        if (ImGui::Checkbox("Enable fog", &data.fogEnabled))
        {
            CmdChangeFloat cmd(CHANGE_FOG_ENABLED, (float)data.fogEnabled);
            pController_->Execute(&cmd);
        }
       
        if (ImGui::ColorEdit3("Fog color", data.fogColor.rgb))
        {
            CmdChangeColor cmd(CHANGE_FOG_COLOR, data.fogColor);
            pController_->Execute(&cmd);
        }

        if (ImGui::DragFloat("Fog start", &data.fogStart))
        {
            CmdChangeFloat cmd(CHANGE_FOG_START, data.fogStart);
            pController_->Execute(&cmd);
        }

        if (ImGui::DragFloat("Fog range", &data.fogRange))
        {
            CmdChangeFloat cmd(CHANGE_FOG_RANGE, data.fogRange);
            pController_->Execute(&cmd);
        }
    }
};

} // namespace UI
