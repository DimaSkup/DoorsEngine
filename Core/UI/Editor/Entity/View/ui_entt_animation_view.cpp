/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: ui_entt_animation_view.cpp
    Desc:     editor's fields for control of entities animations
              (a part of MVC pattern)

    Created:  24.12.2025  by DimaSkup
\**********************************************************************************/
#include <CoreCommon/pch.h>
#include "ui_entt_animation_view.h"
#include "../Model/EnttAnimationData.h"

#include <UICommon/ieditor_controller.h>
#include <UICommon/editor_cmd.h>
#include <imgui.h>

namespace UI
{

//---------------------------------------------------------
// Desc:  show editor fields to control animations properties
//        of the currently selected entity
//---------------------------------------------------------
void EnttAnimationView::Render(
    IEditorController* pController,
    const EnttAnimationData* pData)
{
    if (!pController || !pData)
    {
        LogErr(LOG, "some input ptr == nullptr");
        return;
    }

    ImGui::Text("Skeleton: %s (id: %u)",  pData->skeletonName.name, pData->skeletonId);
    ImGui::Text("Animation: %s (id: %u)", pData->currAnimName.name, pData->animationId);
    ImGui::Text("Anim time: %f",          pData->currAnimTime);
    ImGui::Text("End anim time: %f",      pData->endAnimTime);

    ImGui::Separator();

    if (ImGui::TreeNode("Select animation"))
    {
        if (pData->animNames == nullptr)
        {
            ImGui::Text("no animations yet");
        }
        else
        {
            const AnimationID             currAnimId    = pData->animationId;
            const cvector<AnimationName>& animNames     = *pData->animNames;
            const uint32                  numAnimations = (uint32)animNames.size();

            ImGui::Text("Number of animations: %d", numAnimations);

            // print selectable list of animations names
            for (uint32 i = 0; i < numAnimations; ++i)
            {
                const bool isSelected = (currAnimId == i);
                char label[64]{'\0'};
                snprintf(label, 64, "[%d]: %s", (int)i, animNames[i].name);

                if (ImGui::Selectable(label, isSelected))
                {
                    AnimationID newSelectedAnimId = (AnimationID)i;
                    CmdChangeFloat cmd(CHANGE_ENTITY_ANIMATION_TYPE, newSelectedAnimId);
                    pController->ExecCmd(&cmd);
                }
            }
        }

        ImGui::TreePop();
    }

    ImGui::Separator();
}

} // namespace
