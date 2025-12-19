#include <CoreCommon/pch.h>
#include "EnttParticlesView.h"
#include <UICommon/color.h>
#include <UICommon/editor_cmd.h>
#include <UICommon/ieditor_controller.h>
#include <imgui.h>


namespace UI
{

EnttParticlesView::EnttParticlesView(IEditorController* pController) : pController_(pController)
{
    CAssert::NotNullptr(pController, "input ptr to the particles editor controller == nullptr");
}

//-----------------------------------------------------

void EnttParticlesView::Render(const EnttParticlesModel& model)
{
    // make a local copy of data container to use it in the editor fields
    EnttParticlesModel data = model;

    if (ImGui::ColorEdit3("Color", data.color.rgb))
    {
        CmdChangeColor cmd(CHANGE_PARTICLES_COLOR, data.color);
        pController_->ExecCmd(&cmd);
    }

    if (ImGui::DragFloat3("External Force", data.externForce.xyz, 0.001f))
    {
        CmdChangeVec3 cmd(CHANGE_PARTICLES_EXTERNAL_FORCE, data.externForce);
        pController_->ExecCmd(&cmd);
    }

    if (ImGui::DragInt("Spawn number per sec", &data.spawnNumPerSec))
    {
        CmdChangeFloat cmd(CHANGE_PARTICLES_SPAWN_NUM_PER_SECOND, (float)data.spawnNumPerSec);
        pController_->ExecCmd(&cmd);
    }

    if (ImGui::DragInt("Lifespan (ms)", &data.lifespanMs, 10, 0))
    {
        CmdChangeFloat cmd(CHANGE_PARTICLES_LIFESPAN_MS, (float)data.lifespanMs);
        pController_->ExecCmd(&cmd);
    }

    if (ImGui::DragFloat("Mass", &data.mass, 0.001f, 0.0f))
    {
        CmdChangeFloat cmd(CHANGE_PARTICLES_MASS, data.mass);
        pController_->ExecCmd(&cmd);
    }

    if (ImGui::DragFloat("Size", &data.size, 0.001f, 0.0f))
    {
        CmdChangeFloat cmd(CHANGE_PARTICLES_SIZE, data.size);
        pController_->ExecCmd(&cmd);
    }

    if (ImGui::DragFloat("Friction (air resistance)", &data.friction, 0.001f, 0.0f))
    {
        CmdChangeFloat cmd(CHANGE_PARTICLES_FRICTION, data.friction);
        pController_->ExecCmd(&cmd);
    }

    if (ImGui::Button("Change material"))
    {
        LogMsg("%sCHANGE MATERIAL%s", CYAN, RESET);
    }
}


} // namespace
