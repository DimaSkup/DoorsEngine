/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: ui_entt_animation_view.h
    Desc:     editor's fields for control of entities animations
              (a part of MVC pattern)

    Created:  24.12.2025  by DimaSkup
\**********************************************************************************/
#pragma once

namespace UI
{
// forward declarations
class IEditorController;
class EnttAnimationData;


class EnttAnimationView
{
public:
    void Render(IEditorController* pController, const EnttAnimationData* pData);
};

} // namespace
